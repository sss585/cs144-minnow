Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`


  cmake -S . -B build
  
  只需要跑一次

  cmake -S .        ← 源码在当前目录（Source）
        -B build    ← 输出到 build 目录（Build）

  作用：读取 CMakeLists.txt，生成 Makefile。相当于"准备施工场地"。

  ---
  cmake --build build
  
  每次改代码后跑

  作用：编译。把 .cc 变成可执行文件。相当于"盖房子"。

  ---
  
  cmake --build build --target test
  编译完跑这个

  作用：运行所有测试，打印通过/失败汇总。和 cd build && ctest 等价。

  ---
  cmake --build build --target check0
  
  check0 专用

  作用：只跑 check0 相关的测试（basics + one_write + two_writes + capacity +
  ...），不管别的 checkpoint。

  ---
  cmake --build build --target speed

  性能测试

  作用：跑速度基准测试，要求 > 0.1 Gbit/s。功能正确了再管这个。

  ---
  cmake --build build --target tidy
  
  可选，代码风格检查

  作用：clang-tidy 扫描你的代码，提示"这里可以改进"（比如变量忘加 const）。不影响功能。

  ---
  cmake --build build --target format
  
  推荐提交前跑一次

  作用：自动格式化代码（缩进、换行风格统一）。省得手动对齐。