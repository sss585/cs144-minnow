#pragma once

#include <cstdint>

/*
 * Wrap32 类型表示一个 32 位无符号整数，它：
 *    - 从一个任意的"零点"（初始值）开始计数
 *    - 到达 2^32 - 1 后会绕回 0
 */

class Wrap32
{
protected://同类之间可直接访问
  uint32_t raw_value_ {};
  //64位绝对索引--32位相对索引（截断，取低位）

public:
  explicit Wrap32( uint32_t raw_value ) : raw_value_( raw_value ) {}

  // 给定绝对序列号 n 和零点，构造对应的 Wrap32---绝对索引-从0开始算的序列号，零点-开始序列号
  static Wrap32 wrap( uint64_t n, Wrap32 zero_point );//静态函数，为了直接用方法，不用创建一个新的
  //返回一个记录着数据的类对象---工厂？
  /*
   * unwrap 方法返回一个绝对序列号，该绝对序列号在给定零点下会包装为当前 Wrap32 值。
   * 参数中还需要一个"检查点"（checkpoint）：一个靠近期望答案的绝对序列号。
   *
   * 存在无数个绝对序列号都能包装为同一个 Wrap32。
   * unwrap 应该返回其中最靠近 checkpoint 的那一个。
   */
  uint64_t unwrap( Wrap32 zero_point, uint64_t checkpoint ) const;//函数承诺不改东西，const只用这个

  Wrap32 operator+( uint32_t n ) const { return Wrap32 { raw_value_ + n }; }
  bool operator==( const Wrap32& other ) const { return raw_value_ == other.raw_value_; }
};