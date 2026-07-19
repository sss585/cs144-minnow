#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: 本接口的 MAC 地址（ARP 里叫"硬件地址"）
// ip_address: 本接口的 IP 地址（ARP 里叫"协议地址"）
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address ), arp_table_(), out_queue_(), waiting_(), current_time_( 0 )
{}



// dgram: 要发送的 IPv4 数据报--IP数据报
// EthernetFrame 以太网帧（包括IP数据包/ARP信息）
// next_hop: 下一跳的 IP 地址（通常是路由器或默认网关，如果目标在同一网络也可以是另一台主机）
// 提示：Address 类型可通过 Address::ipv4_numeric() 转换为 uint32_t（32 位 IP 地址）
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  EthernetFrame send_msg ;
  uint32_t IP_target=next_hop.ipv4_numeric();
  send_msg.header.src=ethernet_address_;

  if(arp_table_.count(IP_target)){
    send_msg.header.dst=arp_table_[IP_target].mac;
    send_msg.header.type= EthernetHeader::TYPE_IPv4;//类的静态成员
    send_msg.payload=serialize(dgram);//payload内容都是整个上一级的二进制---其中serialize都是把头部也二进制后加到一起
        //全局版本的serialize自动调用
  }else{
    send_msg.header.dst=ETHERNET_BROADCAST;
    send_msg.header.type=EthernetHeader::TYPE_ARP;
    
    if(waiting_.count(IP_target)){waiting_[IP_target].first.push_back(dgram);return;}//已经发过了，继续等
    else{
      waiting_[IP_target]={{},current_time_};
      waiting_[IP_target].first.push_back(dgram);

       //内容为ARPMessage
      ARPMessage arp_request;
      arp_request.opcode=ARPMessage::OPCODE_REQUEST;
      arp_request.sender_ethernet_address=ethernet_address_;
      arp_request.sender_ip_address=ip_address_.ipv4_numeric();//从本端口开始请求
      arp_request.target_ethernet_address={};//点对点的要求配对，不知为不知
      arp_request.target_ip_address= IP_target;
      
      send_msg.payload=serialize(arp_request);
    }
   
  }
  out_queue_.push(send_msg);

  
}

// frame: 收到的以太网帧---链路层协议---链路层，只查来的目标mac对不对
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{//frame帧
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST )
  return {};//检查mac是否对

  uint16_t frame_type=frame.header.type;
  if(frame_type==EthernetHeader::TYPE_IPv4){
    InternetDatagram in_datagram;
    parse(in_datagram,frame.payload);//是将二进制2参转换后存入1参
    return in_datagram;
  }
  else if(frame_type==EthernetHeader::TYPE_ARP){
    ARPMessage arp_msg;
    parse(arp_msg,frame.payload);
    EthernetAddress eth_adre=arp_msg.sender_ethernet_address;
    uint32_t ip_adre=arp_msg.sender_ip_address;

    if(arp_table_.count(ip_adre)){arp_table_[ip_adre].live_time=current_time_;}
    else {arp_table_[ip_adre]={eth_adre,current_time_};}

    if(arp_msg.opcode==ARPMessage::OPCODE_REQUEST){//只回复自己的
      if(arp_msg.target_ip_address==ip_address_.ipv4_numeric()){
        ARPMessage arp_reply;
        arp_reply.opcode = ARPMessage::OPCODE_REPLY;
        arp_reply.sender_ethernet_address = ethernet_address_;
        arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
        arp_reply.target_ethernet_address = eth_adre;
        arp_reply.target_ip_address = ip_adre;

        EthernetFrame send_msg;
        send_msg.header.src = ethernet_address_;
        send_msg.header.dst = eth_adre;
        send_msg.header.type = EthernetHeader::TYPE_ARP;
        send_msg.payload = serialize( arp_reply );
        out_queue_.push( send_msg );

      }

    }
    if(arp_msg.opcode==ARPMessage::OPCODE_REPLY){//再看自己有没有因为这个阻塞的
      if(waiting_.count(ip_adre)){
        vector<InternetDatagram> vec_ternet=move(waiting_[ip_adre].first);
        for(auto ip_msg:vec_ternet){
          EthernetFrame send_msg;
          send_msg.header.src = ethernet_address_;
          send_msg.header.dst = eth_adre;
          send_msg.header.type = EthernetHeader::TYPE_IPv4;
          send_msg.payload = serialize( ip_msg );
          out_queue_.push( send_msg );

        }
        waiting_.erase(ip_adre);
      }


    }
  }
  return{};
}

// ms_since_last_tick: 距离上次调用本方法经过的毫秒数
void NetworkInterface::tick( const size_t ms_since_last_tick )//按时高频调用
{
  current_time_+=ms_since_last_tick;//本地维护时钟，准度取决于调用频率
  for(auto it=arp_table_.begin();it!=arp_table_.end();){
    if( current_time_-it->second.live_time>=ARP_CACHE_TTL){it=arp_table_.erase(it);}//删掉后it会失效
    else{it++;}//方便但性能略低吗
  }

  for ( auto& [ip, val] : waiting_ ) {
    auto& [dgrams, arp_time] = val;
    if ( current_time_ - arp_time >= ARP_REQUEST_TTL ) {
      arp_time = current_time_;

      ARPMessage arp_msg;
      arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
      arp_msg.sender_ethernet_address = ethernet_address_;
      arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
      arp_msg.target_ethernet_address = {};
      arp_msg.target_ip_address = ip;

      EthernetFrame send_msg;
      send_msg.header.src = ethernet_address_;
      send_msg.header.dst = ETHERNET_BROADCAST;
      send_msg.header.type = EthernetHeader::TYPE_ARP;
      send_msg.payload = serialize( arp_msg );
      out_queue_.push( send_msg );
    }
  }
}
//测试框架调 maybe_send() 取出帧，塞给对方的recv_frame()，模拟网络传输---直接再程序里搬运
// 返回队列中下一个待发送的以太网帧（如果有的话）
optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(out_queue_.empty())return {};
  else{
    EthernetFrame out=out_queue_.front();
    out_queue_.pop();
    return out;
  }
}