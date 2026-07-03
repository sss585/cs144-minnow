#include "byte_stream.hh"

#include <cstdint>
#include <stdexcept>

/*
 * read: 从 Reader 中 peek + pop 最多 len 个字节，拼成 string
 */
void read( Reader& reader, uint64_t len, std::string& out )
{
  out.clear();

  while ( reader.bytes_buffered() and out.size() < len ) {
    auto view = reader.peek();

    if ( view.empty() ) {
      throw std::runtime_error( "Reader::peek() returned empty string_view" );
    }

    view = view.substr( 0, len - out.size() ); // 不返回超过需要数量的字节
    out += view;
    reader.pop( view.size() );
  }
}

// 以下四个函数把 ByteStream 转换成 Reader/Writer 视图
// 所有数据都存在 ByteStream 里，Reader 和 Writer 只是不同的"视口"

Reader& ByteStream::reader()
{
  static_assert( sizeof( Reader ) == sizeof( ByteStream ),
                 "请把成员变量加到 ByteStream 基类，不要加到 Reader 里" );

  return static_cast<Reader&>( *this ); // NOLINT(*-downcast)
}

const Reader& ByteStream::reader() const
{
  static_assert( sizeof( Reader ) == sizeof( ByteStream ),
                 "请把成员变量加到 ByteStream 基类，不要加到 Reader 里" );

  return static_cast<const Reader&>( *this ); // NOLINT(*-downcast)
}

Writer& ByteStream::writer()
{
  static_assert( sizeof( Writer ) == sizeof( ByteStream ),
                 "请把成员变量加到 ByteStream 基类，不要加到 Writer 里" );

  return static_cast<Writer&>( *this ); // NOLINT(*-downcast)
}

const Writer& ByteStream::writer() const
{
  static_assert( sizeof( Writer ) == sizeof( ByteStream ),
                 "请把成员变量加到 ByteStream 基类，不要加到 Writer 里" );

  return static_cast<const Writer&>( *this ); // NOLINT(*-downcast)
}
