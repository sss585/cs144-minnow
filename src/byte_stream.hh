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
  uint64_t capacity_;
  // ↑ 在这里添加成员变量！不要加到 Writer/Reader 里。

public:
  explicit ByteStream( uint64_t capacity );

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
