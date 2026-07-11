#pragma once

#include "wrapping_integers.hh"

#include <optional>

/*
 * TCPReceiverMessage 是 TCP 接收方回复给发送方的消息。
 *
 * 包含两个字段：
 *
 * 1) ackno（确认号）：接收方期望的下一个序列号。
 *    这是一个 optional 字段——如果接收方还没收到 ISN（初始序列号），则 ackno 为空。
 *
 * 2) window_size（窗口大小）：接收方能够接收的序列号数量，从 ackno（如果有的话）开始计算。---用的时候这样-next——index
 *    最大值为 65535（即 <cstdint> 中的 UINT16_MAX）。---值而已,不是相对
 */

struct TCPReceiverMessage
{
  std::optional<Wrap32> ackno {};
  uint16_t window_size {};
};