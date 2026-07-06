#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  Address addr( host, "http" );     // "http" 自动解析为 80 端口
  TCPSocket sock;
  sock.connect( addr );             // 连接服务器

  string request = "GET " + path + " HTTP/1.1\r\n"
                   "Host: " + host + "\r\n"
                   "Connection: close\r\n"
                   "\r\n";          // 构建 HTTP GET 请求

  // 循环写完（write 可能只写了一部分）
  while ( !request.empty() ) {
    request.erase( 0, sock.write( request ) );
  }

  string response;
  while ( !sock.eof() ) {           // 没到 EOF 就继续读
    sock.read( response );          // read 内部会 clear，每次都是新数据
    cout << response;               // 打印到标准输出
  }
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
