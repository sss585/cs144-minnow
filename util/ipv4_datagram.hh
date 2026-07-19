#pragma once

#include "ipv4_header.hh"
#include "parser.hh"

#include <memory>
#include <string>
#include <vector>

//!// brief [IPv4](\ref rfc::rfc791) 互联网数据报
struct IPv4Datagram
{
  IPv4Header header {};
  std::vector<Buffer> payload {};//上一级的二进制内容--TCP

  void parse( Parser& parser )//2进制还原结构体
  {
    header.parse( parser );
    parser.all_remaining( payload );
  }

  void serialize( Serializer& serializer ) const  //结构体变为2进制---网络中二进制
  {       //头部先压缩+加上一级二进制完的内容，最后只留二进制到物理层发送
    header.serialize( serializer );
    for ( const auto& x : payload ) {
      serializer.buffer( x );
    }
  }
};

using InternetDatagram = IPv4Datagram;