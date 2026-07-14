#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), buffer_(), bytes_pushed_( 0 ), bytes_popped_( 0 ), is_closed_( false ), has_error_( false )
{}

void Writer::push( string data )
{
  uint64_t n = min( available_capacity(), data.size() );
  buffer_ += data.substr( 0, n );//从0截取到n，截取n个字符
  bytes_pushed_ += n;
  
  // Your code here.
  //(void)data; 防止出现参数未使用报错

}

void Writer::close()
{
  is_closed_ = true;
  
}

void Writer::set_error()
{
  has_error_ = true;
}

bool Writer::is_closed() const
{
  
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

string_view Reader::peek() const//peek--偷看一眼
{//string_view---C++17, 本质是一个指向字符串的指针和长度，类似于一个轻量级的字符串引用
  // Your code here.
  return string_view( buffer_ );//返回缓冲区的字符串引用
}

bool Reader::is_finished() const
{
  // Your code here.
  return buffer_.empty() && is_closed_;//read结束就是缓冲区空
}

bool Reader::has_error() const
{
  // Your code here.
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  // 单纯移除
  uint64_t n = min( len, bytes_buffered() );//取len和缓冲区大小的最小值
  buffer_.erase(0,n); // 从前端移除 n 个字节
  bytes_popped_ +=n;
  
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}
