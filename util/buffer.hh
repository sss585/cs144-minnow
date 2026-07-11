#pragma once

#include <memory>
#include <string>

// 基于 shared_ptr 的字符串封装，多个 Buffer 可以共享同一份数据
class Buffer
{
  std::shared_ptr<std::string> buffer_;

public:
  // NOLINTBEGIN(*-explicit-*)

  // 从 string 构造（默认空字符串）
  Buffer( std::string str = {} ) : buffer_( make_shared<std::string>( std::move( str ) ) ) {}
  // 隐式转换为 string_view / string&，方便像普通字符串一样使用
  operator std::string_view() const { return *buffer_; }
  operator std::string&() { return *buffer_; }

  // NOLINTEND(*-explicit-*)

  // 交出内部 string 的所有权
  std::string&& release() { return std::move( *buffer_ ); }
  size_t size() const { return buffer_->size(); }
  size_t length() const { return buffer_->length(); }
  bool empty() const { return buffer_->empty(); }
};