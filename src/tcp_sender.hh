#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
//TCP本身的收发管理
/*内核收到 TCP 段时：
  if (header.flags & SYN)  → 消费 1 个 seqno，状态机推进
  if (header.flags & FIN)  → 消费 1 个 seqno，状态机推进
  if (payload_len > 0)     → 消费 payload_len 个 seqno
  三者在"需要确认、允许重传、排序必须正确"上完全一致。唯一区别就是 payload 有实际字节要交给应用层，SYN/FIN 只推进状态机不产生数据
  //SYN和FIN特别对待，同data待遇，必须传入，其他丢就丢了
*/




class TCPSender//TCP发送端
{
  Wrap32 isn_;//Initial Sequence Number — 初始序列号
  // 已发送未确认的段队列（用于重传）
  std::deque<TCPSenderMessage> message_uncheck_ {};//已发送未确认的消息序列  //queue FIFO队列，尾入头出无返回值
  std::deque<TCPSenderMessage> message_unsend_ {};
  //  deque  内存离散的随机数组---可用于SACK控制\\\\\\\ vector不能操作头部--头部无法弹出
    
  //序列号管理
  uint64_t abs_next_seqno_{0};   //下一个可用的的TCP绝对序列号
  uint64_t allbytes_uncheck_ {};    //未确认和还未发送在缓存里的的字节数---总缓存数

  //1 2 3 4
  

  //重传
  uint64_t initial_RTO_ms_{};   //超时时间初始值
  uint64_t rto_ms_{};           //当前退避超时时间---追踪的是俩条确认消息之间的间隔
  uint64_t timer_ms_ {};      //累计时间
  uint64_t retr_count_ {};      //重传次数retranmission
  bool need_retr{};

  //窗口
  uint16_t window_size_ {1}; //窗口大小---不知道时，默认保守1字节，也只需要发syn---syn/fin占窗口实际流中会小一点
  bool syn_acked_{};          //是否接收到SYN
  bool syn_needed_{};          //是否需要发SYN

  bool fin_acked_{};          //是否接收到FIN
  bool fin_sended_{};          //是否已经发FIN
  bool fin_needed_{};          //是否需要发FIN

  //SYN---开始默认需要发（需要和发送合一），发送后等接收到再继续
  //FIN---开始不需要发，需要了有条件发（需要和发送分开），接收后再继续



  std::string pending_data_ {};   //本地缓冲区
  //pending 待决的，待定的
public:
  // 构造，指定初始重传超时(RTO)和可选的固定 ISN
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  // 发送端字节流主动 push 数据进来，准备发送
  void push( Reader& outbound_stream );
  bool make_one();//测试要求构造send段逻辑与send分开
  // 需要发送的话返回一个 TCPSenderMessage，否则返回空 optional
  std::optional<TCPSenderMessage> maybe_send();

  // 生成一个空的 TCPSenderMessage（只有 ackno，没有 payload）---测试用
  //TCP中发送的信息，都以TCPSenderMessage形式包装起来
  TCPSenderMessage send_empty_message() const;

  // 收到对端接收方的 ack + window
  void receive( const TCPReceiverMessage& msg );

  // 时间流逝，单位毫秒
  void tick( uint64_t ms_since_last_tick );

  // 以下为测试用访问器
  uint64_t sequence_numbers_in_flight() const;  // 已发出但还没确认的序列号数
  uint64_t consecutive_retransmissions() const; // 连续重传次数
};