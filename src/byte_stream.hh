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

  //写的时候设置流的状态，读的时候查看
  bool is_closed_ = false;//is_closed_（已关闭） 是 ByteStream 的流是否已关闭标志
  bool has_error_ = false;//has_error_（出错） 是 ByteStream 的流是否出过错标志

public:
  explicit ByteStream( uint64_t capacity );//禁止隐式类型转换

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;
};

class Writer : public ByteStream
{
public:
  void push( std::string data ); // Push data to stream, but only as much as available capacity allows.

  void close();     // Signal that the stream has reached its ending. Nothing more will be written.
  void set_error(); // Signal that the stream suffered an error.

  bool is_closed() const;              // Has the stream been closed?
  uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?
  uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream
};

class Reader : public ByteStream
{
public:
  std::string_view peek() const; // Peek at the next bytes in the buffer
  void pop( uint64_t len );      // Remove `len` bytes from the buffer

  bool is_finished() const; // Is the stream finished (closed and fully popped)?
  bool has_error() const;   // Has the stream had an error?

  uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)
  uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream
};

/*
 * read: A (provided) helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read( Reader& reader, uint64_t len, std::string& out );//实现可以自己去找