#pragma once

#include "ethernet_header.hh"
#include "ipv4_header.hh"
#include "parser.hh"

// [ARP](\ref rfc::rfc826) 消息
struct ARPMessage
{
  static constexpr size_t LENGTH = 28;         // ARP 消息总长度（字节）
  static constexpr uint16_t TYPE_ETHERNET = 1; // 链路层协议类型：以太网/Wi-Fi
  static constexpr uint16_t OPCODE_REQUEST = 1; // 操作码：ARP 请求
  static constexpr uint16_t OPCODE_REPLY = 2;   // 操作码：ARP 回复

  uint16_t hardware_type = TYPE_ETHERNET;             // 链路层协议类型（一般是以太网）
  uint16_t protocol_type = EthernetHeader::TYPE_IPv4; // 网络层协议类型（一般是 IPv4）
  uint8_t hardware_address_size = sizeof( EthernetHeader::src );   // 硬件地址长度
  uint8_t protocol_address_size = sizeof( IPv4Header::src );       // 协议地址长度
  uint16_t opcode {}; // 操作码：请求 or 回复

  EthernetAddress sender_ethernet_address {};   // 发送方 MAC 地址
  uint32_t sender_ip_address {};                // 发送方 IP 地址

  EthernetAddress target_ethernet_address {};   // 目标 MAC 地址
  uint32_t target_ip_address {};                // 目标 IP 地址

  // 返回人类可读的 ARP 消息字符串
  std::string to_string() const;

  // 此类型 ARP 消息是否被解析器支持？
  bool supported() const;

  void parse( Parser& parser );
  void serialize( Serializer& serializer ) const;
};