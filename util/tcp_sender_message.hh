#pragma once

#include "buffer.hh"
#include "wrapping_integers.hh"

#include <string>

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

struct TCPSenderMessage
{
  Wrap32 seqno { 0 };
  bool SYN { false };
  Buffer payload {};
  bool FIN { false };

  // 这个段占用了多少个序列号？
  size_t sequence_length() const { return SYN + payload.size() + FIN; }
};
