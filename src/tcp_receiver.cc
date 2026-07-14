#include "tcp_receiver.hh"

using namespace std;

// 接收发送方发来的消息，把 payload 插入 Reassembler
void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if(!ISN_){
    if(!message.SYN)return;
    ISN_=message.seqno;
   //会有数据或FIN的
  }
  uint64_t checkpoint=inbound_stream.bytes_pushed()+1;//第一个是syn未计入，TCP=输入流+1---push值+1-参考值
  uint64_t abs = message.seqno.unwrap( *ISN_, checkpoint );//返回TCP的绝对字节序---要转为流的字节序---TCP绝对序列
  if(message.SYN)abs++;//如果有syn，流的索引应该从下一个开始
  uint64_t stream_idx=abs-1;//流的索引与TCP索引的对应关系---流比TCP少1---TCP绝对序列
  //FIN不会包括在流中，在流之后，对流序号无影响

  reassembler.insert(stream_idx,message.payload.release(),message.FIN,inbound_stream);
  

// size_t sequence_length() const { return SYN + payload.size() + FIN; }   FIN传在末尾
  //本身穿了个32位序号，以ISN为零点，以求接近index的值
}

// 构造回复给发送方的消息（ackno + window_size）
TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // TCP---确认码32位
  uint16_t window_max=(uint16_t)min(inbound_stream.available_capacity(),(uint64_t)65535);//剩下的空间为窗口
  //////注意，如果可用窗口大于65535，会被截成0，要先和最大值比较

  uint64_t next_abs=inbound_stream.bytes_pushed()+1;//只取出了流中序号，必有SYN+1---TCP预期的下一个序列号
  if(inbound_stream.is_closed())next_abs++;//可能有FIN+1
  if(ISN_){
    return TCPReceiverMessage{.ackno=Wrap32::wrap(next_abs,*ISN_),.window_size=window_max};//相当于命名空间使用
  }
  else{
    return TCPReceiverMessage{.window_size=window_max};//.member=value关键词赋值
  }
}