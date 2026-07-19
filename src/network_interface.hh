#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <iostream>
#include <list>
#include <optional>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

// "网络接口"——连接 IP（网络层）和以太网（链路层）

// 这个模块是 TCP/IP 协议栈的最底层（连接 IP 与下层网络协议，如以太网）。
// 但它也可以被路由器反复使用：路由器通常有多个网络接口，
// 路由器的任务就是在这些接口之间转发数据报。
// NetworkInterface 把 IP 数据报翻译成以太网帧。
// 要填充以太网目标地址，它需要查找每个数据报的下一跳的 MAC 地址，
// 通过 [ARP 协议](\ref rfc::rfc826) 来请求。
// 反方向上，NetworkInterface 接收以太网帧，检查是否是发给自己的，
// 然后根据帧类型处理载荷：IPv4 则上传，ARP 则学习或回复。

class NetworkInterface
{
private:
  // 本接口的 MAC 地址（硬件地址）
  EthernetAddress ethernet_address_;

  // 本接口的 IP 地址
  Address ip_address_;

  
  struct ARPEntry {
    EthernetAddress mac;//mac是每一跳之间的，IP才是全程
    size_t live_time;  // 学习时刻（毫秒）
  };

  std::unordered_map<uint32_t, ARPEntry> arp_table_;
  //      ↑ IP地址 → {MAC, 啥时候记的}
  // 还要维护：---以太网帧待发
  std::queue<EthernetFrame> out_queue_;  // 待发帧

  std::unordered_map<uint32_t, std::pair<std::vector<InternetDatagram>, size_t>> waiting_;
  //      ↑ IP → {ARP 没收到回复时暂存的数据报, ARP请求发送时间}---IP数据报

  size_t current_time_=0;//size_t 和uint64等价
  static constexpr size_t ARP_CACHE_TTL = 30000;   // ARP 缓存 30 秒失效
  static constexpr size_t ARP_REQUEST_TTL = 5000;  // ARP 请求 5 秒没回复重发
  // constexpr，用const编译器也是常量处理，这里


public:
  // 构造，指定 MAC 地址和 IP 地址
  NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address );

  // 取出待发送的以太网帧（如果有的话）
  std::optional<EthernetFrame> maybe_send();

  // 发送一个 IPv4 数据报（封装成以太网帧）。
  // 如果不知道下一跳的 MAC 地址，需要用 ARP 先查。
  void send_datagram( const InternetDatagram& dgram, const Address& next_hop );

  // 收到一个以太网帧，按类型处理：
  // IPv4 → 返回内部数据报
  // ARP 请求 → 学习映射 + 发送 ARP 回复
  // ARP 回复 → 学习映射
  std::optional<InternetDatagram> recv_frame( const EthernetFrame& frame );


  // 时间流逝，处理 ARP 缓存过期等
  void tick( size_t ms_since_last_tick );
};