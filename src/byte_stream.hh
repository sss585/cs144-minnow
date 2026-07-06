#pragma once

#include <queue>
#include <stdexcept>
#include <string>
#include <string_view>

class Reader;
class Writer;

class ByteStream
{
protected:
//uint64--8字节，TCP32位还会回绕
  uint64_t capacity_;//capacity_（能力） 是 ByteStream 的容量，单位是字节
  // ↑ 在这里添加成员变量！不要加到 Writer/Reader 里。保护内部使用
  std::string buffer_;//buffer_（缓冲区） 是 ByteStream 的缓冲区，存储已写入但还没被读走的数据

  uint64_t bytes_pushed_ = 0;//bytes_pushed_（已写入字节数） 是 ByteStream 的累计写入字节总数
  uint64_t bytes_popped_ = 0;//bytes_popped_（已读出字节数） 是 ByteStream 的累计已读出的字节总数

  bool is_closed_ = false;//is_closed_（已关闭） 是 ByteStream 的流是否已关闭标志
  bool has_error_ = false;//has_error_（出错） 是 ByteStream 的流是否出过错标志

public:
  explicit ByteStream( uint64_t capacity );//禁止隐式类型转换

  // 辅助函数（已提供）：把 ByteStream 转成 Reader/Writer 视图来用
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;
};

class Writer : public ByteStream
{
public:
  // 写入数据，但不能超过当前可用容量（多的部分丢弃）
  void push( std::string data );

  // 标记流结束，之后不能再写
  void close();

  // 标记流出错
  void set_error();

  // 流是否已关闭？
  bool is_closed() const;

  // 现在还能写入多少字节？
  uint64_t available_capacity() const;

  // 累计写入字节总数（只增不减）
  uint64_t bytes_pushed() const;
};

class Reader : public ByteStream
{
public:
  // 看一眼缓冲区里的数据（不移除）
  std::string_view peek() const;

  // 从缓冲区前端移除 len 个字节
  void pop( uint64_t len );

  // 流是否已结束？（已关闭 且 缓冲区已被读完）
  bool is_finished() const;

  // 流是否出过错？
  bool has_error() const;

  // 当前缓冲区里的字节数（已写但还没被读走）
  uint64_t bytes_buffered() const;

  // 累计已读出的字节总数（只增不减）
  uint64_t bytes_popped() const;
};
 
/*
 * read: 一个已提供的辅助函数，从 Reader 中 peek + pop 最多 len 个字节，拼成 string
 */
void read( Reader& reader, uint64_t len, std::string& out );
