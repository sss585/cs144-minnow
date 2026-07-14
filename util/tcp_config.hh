#pragma once

#include "wrapping_integers.hh"

#include <cstddef>
#include <cstdint>
#include <optional>

// TCP 发送/接收双方共用配置
class TCPConfig
{
public:
  static constexpr size_t DEFAULT_CAPACITY = 64000; // 默认容量
  static constexpr size_t MAX_PAYLOAD_SIZE = 1000;  // 每个段最大 payload（保守值，适配真实互联网）
  static constexpr uint16_t TIMEOUT_DFLT = 1000;    // 默认重传超时 = 1000 毫秒（1 秒）
  static constexpr unsigned MAX_RETX_ATTEMPTS = 8;  // 放弃前最多重传次数

  uint16_t rt_timeout = TIMEOUT_DFLT;      // 初始重传超时（毫秒）
  size_t recv_capacity = DEFAULT_CAPACITY; // 接收容量（字节）
  size_t send_capacity = DEFAULT_CAPACITY; // 发送容量（字节）
  std::optional<Wrap32> fixed_isn {};      // 固定 ISN（可选，测试用）
};
