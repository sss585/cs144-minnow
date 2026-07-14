#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

// 构造函数：使用指定 RTO；如果没有给定 ISN 则随机生成
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , message_uncheck_()
  , abs_next_seqno_( 0 )
  , bytes_uncheck_( 0 )
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
  return bytes_uncheck_;
}

// 连续重传次数
uint64_t TCPSender::consecutive_retransmissions() const
{
  // 你的代码写在这里
  return retr_count_;
}

// 尝试发送一个消息（如果窗口允许、有数据要发的话）
optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if(need_retr){
    need_retr=false;
    if(!message_uncheck_.empty())return message_uncheck_.front();}//优先重传--1.0窗口确认 2.syn和fin等待重传
  if(window_size_==0) return{};//该阶段等重传确认窗口--无符号数--下面最小有1
  if((!syn_needed_&&!syn_acked_)||(fin_sended_&&!fin_acked_)) return{};//已经发过等重传
  
  if ( !syn_needed_ && pending_data_.empty() && !fin_needed_ )return {};//测试要求无数据时返回空

  TCPSenderMessage tcp_send_msg;

  tcp_send_msg.seqno=Wrap32::wrap(abs_next_seqno_,isn_);
  if(syn_needed_){tcp_send_msg.SYN=true;syn_needed_=false;}

  uint64_t win_left=(window_size_>bytes_uncheck_+tcp_send_msg.SYN
    ?window_size_-bytes_uncheck_-tcp_send_msg.SYN:0);
  uint64_t data_len=min(min(win_left,pending_data_.size()),TCPConfig::MAX_PAYLOAD_SIZE);//原来只有2个参数，{}版本任意多个
  tcp_send_msg.payload= pending_data_.substr(0,data_len);
  //隐式转换成string使用   //substr第二个参数是len

  if(fin_needed_
    &&pending_data_.empty()
    &&message_uncheck_.empty())   {tcp_send_msg.FIN=true;fin_needed_=false;fin_sended_=true;}
  //发送端空且结束，本地缓存空，所有消息都已确认，先前未发过---不会顺带空间的情况

  message_uncheck_.push_back(tcp_send_msg);
  bytes_uncheck_+=tcp_send_msg.sequence_length();
  abs_next_seqno_+=tcp_send_msg.sequence_length();
  pending_data_.erase(0,data_len);

  return tcp_send_msg;

}

// push 数据到发送缓冲区
void TCPSender::push( Reader& outbound_stream )
{
  if(!outbound_stream.bytes_buffered())return;//读取条件是有数据可读，是否关闭无关
  //使用封装好的read函数
  string chunk;//要被修改
  read(outbound_stream,outbound_stream.bytes_buffered(),chunk);//同名恰好能区分
  pending_data_+=chunk;

  if(outbound_stream.is_finished())fin_needed_=true;


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
  TCPSenderMessage uncheck_front=message_uncheck_.front();

  while(ack>=uncheck_front.seqno.unwrap(isn_,abs_next_seqno_)+uncheck_front.sequence_length()){
    //deque这种可双端弹出的分pop_front./back,queue这种pop就是front0---无返回值
    

    bytes_uncheck_-=uncheck_front.sequence_length();
    if(uncheck_front.SYN)syn_acked_=true;
    if(uncheck_front.FIN){fin_acked_=true;fin_sended_=false;}
    message_uncheck_.pop_front();

   
    timer_ms_=0;
    rto_ms_=initial_RTO_ms_;//确实删了才排除

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
  if(timer_ms_>rto_ms_){
    need_retr=true;
    retr_count_++;
    timer_ms_=0;

    if(window_size_!=0){//空窗口确认时，不退避
    rto_ms_*=2;//指数退避
    }
  }

  

  
}