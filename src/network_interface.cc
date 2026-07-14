#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: 本接口的 MAC 地址（ARP 里叫"硬件地址"）
// ip_address: 本接口的 IP 地址（ARP 里叫"协议地址"）
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ )
       << " and IP address " << ip_address_.ip() << "\n";
}

// dgram: 要发送的 IPv4 数据报
// next_hop: 下一跳的 IP 地址（通常是路由器或默认网关，如果目标在同一网络也可以是另一台主机）
// 提示：Address 类型可通过 Address::ipv4_numeric() 转换为 uint32_t（32 位 IP 地址）
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // 你的代码写在这里
  (void)dgram;
  (void)next_hop;
}

// frame: 收到的以太网帧
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // 你的代码写在这里
  (void)frame;
  return {};
}

// ms_since_last_tick: 距离上次调用本方法经过的毫秒数
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // 你的代码写在这里
  (void)ms_since_last_tick;
}

// 返回队列中下一个待发送的以太网帧（如果有的话）
optional<EthernetFrame> NetworkInterface::maybe_send()
{
  // 你的代码写在这里
  return {};
}