#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

// 构造函数：使用指定 RTO；如果没有给定 ISN 则随机生成
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , message_uncheck_()
  , message_unsend_()
  , abs_next_seqno_( 0 )
  , allbytes_uncheck_( 0 )
  , initial_RTO_ms_( initial_RTO_ms )
  , rto_ms_( initial_RTO_ms )
  , timer_ms_( 0 )
  , retr_count_( 0 )
  , need_retr( false )
  , window_size_( 1 )
  , syn_acked_( false )
  , syn_needed_( true )
  , fin_acked_( false )
  , fin_sended_( false )
  , fin_needed_( false )
  , pending_data_()
{}

// 已发送但未确认的序列号数量
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // 你的代码写在这里
  return allbytes_uncheck_;
}

// 连续重传次数
uint64_t TCPSender::consecutive_retransmissions() const
{
  // 你的代码写在这里
  return retr_count_;
}

// 尝试发送一个消息（如果窗口允许、有数据要发的话）
optional<TCPSenderMessage> TCPSender::maybe_send()//只负责提取出下一个应该发送的段
{//重传部分
  if(need_retr){
    need_retr=false;
    if(!message_uncheck_.empty())return message_uncheck_.front();}//优先重传--1.0窗口确认 2.syn和fin等待重传

  if(message_unsend_.empty()&&!make_one())return {};//空且新增失败
  
  //此时必有
  TCPSenderMessage tcp_send_msg=message_unsend_.front();//先提取开头最早的
  message_unsend_.pop_front();
  message_uncheck_.push_back( tcp_send_msg );


  return tcp_send_msg;

}

// push 数据到发送缓冲区
void TCPSender::push( Reader& outbound_stream )
{
  if(outbound_stream.bytes_buffered()){//读取条件是有数据可读，是否关闭无关
  //使用封装好的read函数
  string chunk;//要被修改
  read(outbound_stream,outbound_stream.bytes_buffered(),chunk);//同名恰好能区分
  pending_data_+=chunk;
  }
  if(outbound_stream.is_finished()&&!fin_sended_)fin_needed_=true;//没数据测试端也在一直push，刷新fin状态
      //只在未发送时开启---发送过就不需要了
 
  while(make_one());
}

bool TCPSender::make_one(){//构造数据段并更新状态


  TCPSenderMessage tcp_send_msg;

  tcp_send_msg.seqno=Wrap32::wrap(abs_next_seqno_,isn_);//序号
  if(syn_needed_){tcp_send_msg.SYN=true;syn_needed_=false;}//SYN

  
  uint64_t zero_win=window_size_>0?window_size_:1;//对0窗口时视为1窗口，传值试探---8次重传时间都不能处理一个字节，视为死机
  uint64_t win_left=(zero_win>allbytes_uncheck_+tcp_send_msg.SYN
    ?zero_win-allbytes_uncheck_-tcp_send_msg.SYN:0);
  uint64_t data_len=min(min(win_left,pending_data_.size()),TCPConfig::MAX_PAYLOAD_SIZE);//原来只有2个参数，{}版本任意多个

  

  tcp_send_msg.payload= pending_data_.substr(0,data_len);//文本
  //隐式转换成string使用   //substr第二个参数是len
  pending_data_.erase(0,data_len);

 //最后的最后判断，前面有没有发完。发完要带上fin，窗口满就下一趟
 if(win_left>data_len){
    if(fin_needed_//FIN
      &&pending_data_.empty())   {tcp_send_msg.FIN=true;fin_needed_=false;fin_sended_=true;}
    //发送端空且结束，本地缓存空，将fin作为未发消息之一，先前未发过
  }
  if ( data_len == 0 && !tcp_send_msg.SYN && !tcp_send_msg.FIN ) return false;//不拦截构造空段，拦截空段的返回

  message_unsend_.push_back(tcp_send_msg);
  allbytes_uncheck_+=tcp_send_msg.sequence_length();//全部类内保存的数据
  abs_next_seqno_+=tcp_send_msg.sequence_length();

 


  return true;

}

// 生成空消息（仅 ack，无数据）
TCPSenderMessage TCPSender::send_empty_message() const//该阶段不做
{
  return TCPSenderMessage{.seqno=Wrap32::wrap(abs_next_seqno_,isn_)};
}

// 收到接收方的 ack 和窗口更新
void TCPSender::receive( const TCPReceiverMessage& msg )
{

  window_size_=msg.window_size;

  if(!msg.ackno||message_uncheck_.empty()){//在正式发送消息前可能有代理提前询问状态，无isn算不了
    return;
  }

  uint64_t ack=msg.ackno.value().unwrap(isn_,abs_next_seqno_);//窗口最大就65535，不可能差出一轮

  if(ack>abs_next_seqno_)return;//不可能接受到的ack抛弃

  TCPSenderMessage uncheck_front=message_uncheck_.front();

  while(ack>=uncheck_front.seqno.unwrap(isn_,abs_next_seqno_)+uncheck_front.sequence_length()){
    //deque这种可双端弹出的分pop_front./back,queue这种pop就是front0---无返回值
    

    allbytes_uncheck_-=uncheck_front.sequence_length();
    if(uncheck_front.SYN)syn_acked_=true;
    if(uncheck_front.FIN){fin_acked_=true;}//fin控制只发一次
    message_uncheck_.pop_front();

   
    timer_ms_=0;
    rto_ms_=initial_RTO_ms_;//确实删了才排除
    retr_count_=0;

    if(message_uncheck_.empty()) break;
    uncheck_front=message_uncheck_.front();

  }

  


}

// 时间流逝，处理超时重传---其他人调用并传入时间
void TCPSender::tick( uint64_t ms_since_last_tick )
{//只用来确认超时，不直接负责重传
  if(message_uncheck_.empty())return;//有才重传

  //别人调用，传入间隔时间
  timer_ms_+=ms_since_last_tick;
  if(timer_ms_>=rto_ms_){//达到就发
   
    retr_count_++;
    timer_ms_=0;

    if(retr_count_<=TCPConfig::MAX_RETX_ATTEMPTS)need_retr=true;

    if(window_size_!=0){//空窗口确认时，不退避
    rto_ms_*=2;//指数退避
    }
  }

  

  
}