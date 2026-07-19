#pragma once

#include "parser.hh"

#include <array>
#include <cstdint>
#include <string>

// MAC 地址 = 6 字节数组
using EthernetAddress = std::array<uint8_t, 6>;

// 以太网广播地址 (ff:ff:ff:ff:ff:ff)
constexpr EthernetAddress ETHERNET_BROADCAST = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// 打印 MAC 地址
std::string to_string( EthernetAddress address );

// 以太网帧头（14 字节）
struct EthernetHeader
{
  static constexpr size_t LENGTH = 14;         // 帧头长度（字节）
  static constexpr uint16_t TYPE_IPv4 = 0x800; // 类型码：IPv4
  static constexpr uint16_t TYPE_ARP = 0x806;  // 类型码：ARP

  EthernetAddress dst;   // 目标 MAC
  EthernetAddress src;   // 源 MAC
  uint16_t type;          // 帧类型（IPv4 or ARP）

  // 人类可读字符串
  std::string to_string() const;

  void parse( Parser& parser );
  void serialize( Serializer& serializer ) const;
};
