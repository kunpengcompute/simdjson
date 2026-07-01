# Installation Guide

This document provides detailed guidance on configuring the compilation environment and compiling/installing Kunpeng-optimized simdjson.

## Environment Requirements

|Item| Version Requirement|
| ------ | ---------- | 
| Hardware| Kunpeng 950 processor (supporting NEON/SVE/SVE2)|
| OS| openEuler 24.03 LTS SP3, Debian 12|
| Compiler| LLVM Clang 16.0.6/GCC 10.0+ |
| Build tool| CMake 3.15+, Git, Python 3|

## Obtaining the Source Code

* Approach 1: Clone the optimized branch from GitCode.

  Clone the SVE2 optimized branch.

  ```bash
  git clone -b dev_sve2 https://gitcode.com/boostkit/simdjson.git
  cd simdjson
  ```

* Approach 2: Download the patch and apply it to the community version.

  1. Clone the official community version of simdjson.

     ```bash
     git clone https://github.com/simdjson/simdjson.git
     cd simdjson
     ```

  2. Check out the specified commit.

     ```bash
     git checkout c2f25ab4
     ```

  3. Download the Kunpeng-optimized simdjson patch.

     ```bash
     wget https://raw.gitcode.com/boostkit/simdjson/raw/master/kunpeng_opti_sve2.patch
     ```

  4. Apply the patch.

     ```bash
     git apply kunpeng_opti_sve2.patch
     ```

## Compilation and Installation

### Baseline Compilation

The default compilation includes implementations for all architectures and supports runtime auto-detection of CPU capabilities.

1. Configure the build.

   ```bash
   cmake -B build \
     -DCMAKE_BUILD_TYPE=Release
   ```

2. Compile.

   ```bash
   cmake --build build -j
   ```

### SVE2 Optimized Compilation (Kunpeng 950 Processor)

Enable SVE2 compilation options to fully leverage the optimized performance of the Kunpeng processor.

1. Configure CMake and enable SVE2 support.

   ```bash
   cmake -B build/ \
     -DSIMDJSON_DEVELOPER_MODE=ON \
     -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_CXX_FLAGS="-march=armv8-a+sve+crc+sve2 -msve-vector-bits=256" \
     -DCMAKE_INSTALL_PREFIX=$HOME/.local/simdjson
   ```

2. Compile.

   ```bash
   cmake --build build/ -j
   ```

3. Install to a specified directory.

   ```bash
   cmake --install build/
   ```

### Compilation Option Description

| Option| Description| Default Value|
|--|--|--|
| `CMAKE_BUILD_TYPE` | Build type (Debug/Release/RelWithDebInfo).| Release |
| `SIMDJSON_DEVELOPER_MODE` | Enables the developer mode (including tests and benchmarks).| OFF |
| `CMAKE_CXX_FLAGS` | Compiler flags (e.g., `-march` to enable SVE2).| - |
| `CMAKE_INSTALL_PREFIX` | Installation path.| /usr/local |
| `BUILD_SHARED_LIBS` | Builds a shared library instead of a static library.| OFF |

## Single-Header Usage

simdjson provides a single-header distribution format, which is suitable for direct embedding into projects or verifying optimization effects.

**Generating Single-Header Files**

Use the following command to generate single-header files:

```bash 
python3 singleheader/amalgamate.py
```

The generated files are as follows:

- Header file: `singleheader/simdjson.h`
- Implementation file: `singleheader/simdjson.cpp`

**Using Single-Header Files**

Copy `simdjson.h` and `simdjson.cpp` into your project, and use the following command to compile.

* Basic compilation

  ```bash
  g++ -o your_app your_app.cpp ./simdjson.cpp -I./ -std=c++17
  ```

* Compilation with SVE2 optimization

  ```bash
  g++ -o your_app your_app.cpp ./simdjson.cpp -I./ -std=c++17 \
      -march=armv8-a+sve+crc+sve2 -msve-vector-bits=256
  ```

## Integration into Projects

**Approach 1: Using the Installed Library**

```cmake
# CMakeLists.txt
find_package(simdjson REQUIRED)
target_link_libraries(your_target PRIVATE simdjson::simdjson)
```

The installation path needs to be specified during compilation.

```bash
g++ -std=c++17 your_app.cpp \
  -I$HOME/.local/simdjson/include \
  -L$HOME/.local/simdjson/lib64 \
  -lsimdjson -o your_app
```

**Approach 2: Integrating as a Submodule**

```cmake
# CMakeLists.txt
add_subdirectory(external/simdjson)
target_link_libraries(your_target PRIVATE simdjson)
```

**Approach 3: Using FetchContent**

```cmake
# CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://gitcode.com/boostkit/simdjson.git
  GIT_TAG        dev_sve2
)
FetchContent_MakeAvailable(simdjson)
target_link_libraries(your_target PRIVATE simdjson)
```

## Tests

### Unit Tests

1. Run all test cases.

   ```bash
   ctest --test-dir build -j
   ```

2. Run a specific test.

   ```bash
   ctest --test-dir build -R dom_tests
   ctest --test-dir build -R ondemand_tests
   ```

### Performance Benchmarks

1. DOM parsing benchmarks

   * Parse a single document `parse()`.

     ```bash
     numactl -C 21 ./build/benchmark/dom/parse -n 1000 ./jsonexamples/twitter.json
     ```

   * Parse multiple documents `parse_many()`.

     ```bash
     numactl -C 21 ./build/benchmark/dom/parse_stream ./jsonexamples/amazon_cellphones.ndjson
     ```

2. Google Benchmark tests

   * General parsing benchmark

     ```bash
     numactl -C 21 ./build/benchmark/bench_parse_call --benchmark_min_time=3s 2>&1
     ```

   * DOM API performance test

     ```bash
     numactl -C 21 ./build/benchmark/bench_dom_api --benchmark_min_time=3s 2>&1
     ```

   * On-Demand API performance test

     ```bash
     numactl -C 21 ./build/benchmark/bench_ondemand --benchmark_min_time=3s 2>&1
     ```

   * Multi-document parsing benchmark

     ```bash
     numactl -C 21 ./build/benchmark/bench_stream_formats --benchmark_min_time=3s 2>&1
     ```

## Installation Verification

1. Create the test file `example.cpp`.

   ```cpp
   #include <simdjson.h>
   #include <iostream>

   int main() {
       simdjson::dom::parser parser;
       auto json = R"({"name": "simdjson", "version": "1.0.0"})"_padded;
    
       simdjson::dom::element doc = parser.parse(json);
       std::string_view name = doc["name"];
       std::string_view version = doc["version"];
    
       std::cout << "Library: " << name << ", Version: " << version << std::endl;
    
       // Check the active implementation.
       std::cout << "Implementation: " << simdjson::get_active_implementation()->name() << std::endl;
    
       return 0;
   }
   ```

2. Use the installed static library for compilation.

   ```bash
   g++ -std=c++17 example.cpp \
     -I$HOME/.local/simdjson/include \
     -L$HOME/.local/simdjson/lib64 \
     -lsimdjson -o example
   ```

3. Run the program.

   ```bash
    ./example
    ```

   The expected output is as follows:

   ```text
   Library: simdjson, Version: 1.0.0
   Active implementation: arm64
   ```

4. Verify SVE2 optimization.

   Check whether the generated binary file contains SVE2 match instructions.

   ```bash
   objdump -d ./example | grep z0 | grep match
   ```

   The expected output is as follows (including SVE2 match instructions):

   ```text
     413e9c: 45268c00  match p0.b, p3/z, z0.b, z6.b
     413ebc: 45278c01  match p1.b, p3/z, z0.b, z7.b
   ```
