#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPReceiver
{
  std::optional<Wrap32> ISN_{};//零点值是用wrap32存储的
  //不需要count，字节计算部分在output的push里计算
public:
  TCPReceiver():ISN_(){};
  /*
   * 接收 TCPSenderMessage，将其 payload 插入 Reassembler 的正确流位置。
   */
  void receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream );

  /* 向 TCPSender 回复一个 TCPReceiverMessage（包含 ackno 和 window_size）。 */
  TCPReceiverMessage send( const Writer& inbound_stream ) const;
};

/*
 * TCPReceiverMessage 是 TCP 接收方回复给发送方的消息。
 *
 * 包含两个字段：
 *
 * 1) ackno（确认号）：接收方期望的下一个序列号。
 *    这是一个 optional 字段——如果接收方还没收到 ISN（初始序列号），则 ackno 为空。---用于判断有值/为空
 *
 * 2) window_size（窗口大小）：接收方能够接收的序列号数量，从 ackno（如果有的话）开始计算。
 *    最大值为 65535（即 <cstdint> 中的 UINT16_MAX）。
 */

 
/*
 * TCPSenderMessage 是 TCP 发送方发给接收方的消息。
 *
 * 包含四个字段：
 *
 * 1) seqno：段起始的序列号。如果 SYN 标志被设置，这就是 SYN 的序列号；
 *    否则就是 payload 起始的序列号。
 *
 * 2) SYN 标志：如果设置，表示这是字节流的开头，seqno 字段存放的是 ISN（初始序列号）——即零点。
 *
 * 3) payload：字节流的一个子串（可能为空）。
 *
 * 4) FIN 标志：如果设置，表示 payload 代表字节流的末尾。
 */
