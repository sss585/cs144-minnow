#include "wrapping_integers.hh"

using namespace std;

// 把 64 位绝对序列号 n 包装成 32 位 Wrap32（基于零点 zero_point）
//输入1.  TCP绝对字节序 2.包含零点的wrap
//返回  包含转换为相对字节序的wrap
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 {uint32_t( zero_point.raw_value_+n )};//取低位为相对索引
}

// 把当前 Wrap32 解包为 64 位绝对序列号，取最接近 checkpoint 的那一个
//输入1.包含零点的wrap  2.参考值（push的字节流）
//返回     TCP绝对字节序（从0开始）
uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t offest=raw_value_-zero_point.raw_value_;//先32位计算，再补0为64位---负数会绕回，从+2^32-zero开始计算
  const uint64_t mod=1ULL<<32;//unsigned ll--64
  uint64_t steps=offest>checkpoint?0:(checkpoint-offest+mod/2)/mod;//补半，四舍五入
  return offest+steps*mod;
}