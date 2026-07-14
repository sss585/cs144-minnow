#include "reassembler.hh"

using namespace std;

// 插入一个子串，重组后写入 output
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{//为对应字节序号
  uint64_t index_max=next_index_+output.available_capacity();//窗口大小以output为准
  //是全部接受数据的上线，存在rea缓冲区是avail的一部分，不能重复计算
  
  if(first_index>=index_max||output.available_capacity()<=total_buffer_bytes)
      return ;
  else if(first_index+data.size()>index_max){
    data=data.substr(0,index_max-first_index);
  }

 is_last_substring_received_ |= is_last_substring;
 //防止传入空字符串带有结束符，判断一下 
//  if(!data.size()&&is_last_substring_received_ &&bytes_pending()==0){---非空且落后会出错
//      output.close();//关闭逻辑放最后
//      return;
//  }
 


  if ( first_index + data.size() <= next_index_ ) {      //index+size是end（）的位置，即下一个位置，不包含
    // 这个子串完全在已写入的范围内，直接丢弃-----会丢掉一些关闭的空数据
    //空之所以出错，是因为被落后抛弃了，在此判断
    //1.正常流入，后续判断最后结尾close  2.落后序列（空），在此拦截判断
    if(is_last_substring_received_ &&bytes_pending()==0){
        output.close();//关闭逻辑放最后
        return;
    }
    return;
  }
  else if ( first_index < next_index_ ) {
    // 这个子串有一部分已经写入，截取未写入的部分
    data = data.substr( next_index_-first_index );//截取相对长度  ---起始位置~~end
    first_index=next_index_;//截取之后长度变了
  }
  else if(first_index>next_index_){         //不连续，有空洞写入缓存

    buffer_insert(first_index,data);
    return;
  }
  output.push(data);
  next_index_ = first_index+data.size();
  while(buffer_.count(next_index_)){//可能可以连续填充空洞
    uint64_t tmp=next_index_;
    output.push(buffer_[next_index_]);
    
    total_buffer_bytes-=buffer_[next_index_].size();

    next_index_+=buffer_[next_index_].size();
    buffer_.erase(tmp);
  }

  auto erase_it = [&]( auto it ) {
    total_buffer_bytes -= it->second.size();
    return buffer_.erase( it );
  };

  
  //用it--  +while循环迭代器范围太繁琐了，改用for吧
  for(auto it=buffer_.begin();it!=buffer_.end();){//对整体遍历--放弃了（时刻注意迭代器）--erase天然会返回下一个的
    uint64_t old_end=it->first+it->second.size();
    if(it->first>next_index_)break;
    else if(old_end<=next_index_){
      it=erase_it(it);
    }
    else{
      data=it->second.substr(next_index_-it->first);//给起始位置
      output.push(data);
      it=erase_it(it);

      next_index_ = next_index_+data.size();
      while(buffer_.count(next_index_)){//可能可以连续填充空洞
        uint64_t tmp=next_index_;
        output.push(buffer_[next_index_]);

        total_buffer_bytes-=buffer_[next_index_].size();

        next_index_+=buffer_[next_index_].size();
        buffer_.erase(tmp);
      }//尾巴更新，继续往后遍历
      
    }


  }



  if(is_last_substring_received_ &&bytes_pending()==0){
    //继续等全传完.因为缓存满丢弃的部分会有TCP控制重传，不会永远堆积
    //最后再判断---是否已经传入最后一个且空洞全部补完了---最后一个可能先传，需要提前判断
    output.close();//关闭逻辑放最后
    return;
  }
}

// Reassembler 内部暂存了多少未写入的字节
uint64_t Reassembler::bytes_pending() const
{

 
  return total_buffer_bytes;
}


void Reassembler::buffer_insert(const uint64_t  index, const string& data){//对缓存插入及去重
  auto it=buffer_.lower_bound(index);//--找第一个>=index的--无则右侧min\\ upper~是第一个>的
  if(it!=buffer_.begin())it--;//找到对应值前一个

  uint64_t new_begin=index;
  uint64_t new_end=index+data.size();//新插入串开头结尾
  
  vector<uint64_t> to_erase;
  //如果使用插入后将重叠部分全部合并，在填补空洞时性能会高一点
  //但是实现略麻烦，目前之际替换重叠部分，留下一堆小碎片实现更简单XXXXX
  /////////会导致data本身的碎片化，太麻烦，回到最开始合并处理。化零为整


  while(it!=buffer_.end()&&it->first<new_end){//要确保it存在,//超出结束
    uint64_t old_begin = it->first;
    uint64_t old_end   = it->first + it->second.size();

    if (old_end >= new_begin) {  // 有重叠
        new_begin = min(new_begin, old_begin);
        new_end   = max(new_end, old_end);//最大范围内
        total_buffer_bytes -= it->second.size();
        to_erase.push_back(it->first);
    }
    it++;
  }

  string merged(new_end-new_begin,'\0');

  for(auto& key:to_erase){//插入旧数据
    auto& old_data=buffer_[key];//提取
    //string.data()返回其中数组的首地址
    copy(old_data.begin(),old_data.end(),merged.begin()+key-new_begin);
    buffer_.erase(key);//for中直接删迭代器---此处循环vec删map
  }
  copy(data.begin(),data.end(),merged.begin()+index-new_begin);//插入新数据

  total_buffer_bytes+=merged.size();//右值后为空了

  buffer_[new_begin]=move(merged);//move转为右值---表明不再使用，可用右值引用---移动指针，窃取句柄，原来为空了
 

}
