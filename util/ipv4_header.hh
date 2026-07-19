#pragma once

#include "parser.hh"

#include <cstddef>
#include <cstdint>
#include <string>

// IPv4 互联网数据报头部（不支持 IP 选项）
struct IPv4Header
{
  static constexpr size_t LENGTH = 20;        // IPv4 头部长度（不含选项）
  static constexpr uint8_t DEFAULT_TTL = 128; // 默认存活时间
  static constexpr uint8_t PROTO_TCP = 6;     // TCP 协议号

  static constexpr uint64_t serialized_length() { return LENGTH; }

  /*
   *   IPv4 头部格式（20 字节 + 可选选项）：
   *
   *   0                   1                   2                   3
   *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  | 版本号 |首部长度| 服务类型    |          总长度               |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |         标识符               |标志位|      分片偏移量         |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |  存活时间    |   协议号     |          头部校验和             |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                        源 IP 地址                             |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                       目的 IP 地址                            |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                    选项                      |    填充        |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  // IPv4 头部各个字段
  uint8_t ver = 4;           // IP 版本（4 = IPv4）
  uint8_t hlen = LENGTH / 4; // 首部长度（以 32 位为单位，=5 表示 20 字节）
  uint8_t tos = 0;           // 服务类型
  uint16_t len = 0;          // 总长度（含头部 + 数据）
  uint16_t id = 0;           // 标识符
  bool df = true;            // 不分片标志
  bool mf = false;           // 更多分片标志
  uint16_t offset = 0;       // 分片偏移
  uint8_t ttl = DEFAULT_TTL; // 存活时间（每跳减 1，为 0 时丢弃）
  uint8_t proto = PROTO_TCP; // 上层协议号（6 = TCP）
  uint16_t cksum = 0;        // 头部校验和
  uint32_t src = 0;          // 源 IP 地址
  uint32_t dst = 0;          // 目的 IP 地址

  // payload 长度 = 总长 - 头部长度
  uint16_t payload_length() const;

  // 伪首部校验和（TCP 校验和用）
  uint32_t pseudo_checksum() const;

  // 计算并填入正确的校验和
  void compute_checksum();

  // 人类可读字符串
  std::string to_string() const;

  void parse( Parser& parser );
  void serialize( Serializer& serializer ) const;
};
