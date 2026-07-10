#pragma once

#include "byte_stream.hh"
#include <map>
#include <string>
//re  assembler重组
class Reassembler
{
public:
  Reassembler() : next_index_(0), buffer_(),total_buffer_bytes(0) ,is_last_substring_received_(false) {}

  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // Reassembler 内部暂存了多少字节？（还没能写入 output 的）
  uint64_t bytes_pending() const;



  
private:

  uint64_t next_index_ = 0; // 下一个要写入 output 的字节的索引
  std::map<uint64_t, std::string> buffer_;      // 暂存的字节
  uint64_t total_buffer_bytes=0;
  bool is_last_substring_received_ = false; // 是否已经收到最后一个子串
  void buffer_insert(const uint64_t index, const std::string& data);
  
};
/*
   * 插入一个新的子串，等待被重组后写入 ByteStream。
   *   `first_index`: 子串第一个字节在原始流中的索引位置
   *   `data`: 子串内容
   *   `is_last_substring`: 这个子串是否代表流的末尾
   *   `output`: ByteStream 的 Writer 端，用于写入已重组好的数据
   *
   * Reassembler 的职责：把带索引的子串（可能乱序到达、可能相互重叠）
   * 重组回原始的字节流。一旦 Reassembler 知道了流中的下一个连续字节，
   * 就应该立即把它写入 output。
   *
   * 如果 Reassembler 收到的字节在流的可用容量范围内，但因为前面还有
   * 未知字节而暂时无法写入，就应该把它们存起来，等前面的空洞被填上后再写。
   *
   * 超出可用容量的字节应该丢弃（即使前面的空洞以后被填上也装不下了）。 
   *
   * 写完最后一个字节后，Reassembler 应该关闭 Writer（调用 close()）。
   */