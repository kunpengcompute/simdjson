# 安装指南

本文档提供基于鲲鹏优化的 simdjson 编译环境配置与编译安装的详细指导。

## 环境要求

- **已验证的硬件配置**：鲲鹏950处理器，支持NEON/SVE/SVE2指令集
- **已验证的OS**：openEuler 24.03 LTS SP3 / Debian 12
- **已验证的编译器**：Clang 16.0.6 / GCC 10.0+
- **构建工具**：CMake 3.15+、Git、Python 3

## 获取源码

### 方式 1：从 GitCode 克隆优化分支

```bash
# 克隆 SVE2 优化分支
git clone -b dev_sve2 https://gitcode.com/boostkit/simdjson.git
cd simdjson
```

### 方式 2：下载 patch 并应用到社区版本

```bash
# 克隆官方社区版本
git clone https://github.com/simdjson/simdjson.git
cd simdjson

# 检出指定的 commit
git checkout c2f25ab4

# 下载鲲鹏优化 patch
wget https://raw.gitcode.com/boostkit/simdjson/raw/master/kunpeng_opti_sve2.patch

# 应用 patch
git apply kunpeng_opti_sve2.patch
```

## 编译安装

### 基础编译（基线版本）

默认编译会包含所有架构的实现，支持运行时自动检测 CPU 能力：

```bash
# 配置构建
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build -j
```

### SVE2 优化编译（鲲鹏 950）

启用 SVE2 编译选项以充分发挥鲲鹏处理器的优化性能：

```bash
# 配置 CMake 并启用 SVE2 支持
cmake -B build/ \
  -DSIMDJSON_DEVELOPER_MODE=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-march=armv8-a+sve+crc+sve2 -msve-vector-bits=256" \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local/simdjson

# 编译
cmake --build build/ -j

# 安装到指定目录
cmake --install build/
```

**编译选项说明**：

| 选项 | 说明 | 默认值 |
|--|--|--|
| `CMAKE_BUILD_TYPE` | 构建类型（Debug/Release/RelWithDebInfo） | Release |
| `SIMDJSON_DEVELOPER_MODE` | 启用开发者模式（包含测试和基准测试） | OFF |
| `CMAKE_CXX_FLAGS` | 编译器标志（-march 启用 SVE2） | - |
| `CMAKE_INSTALL_PREFIX` | 安装路径 | /usr/local |
| `BUILD_SHARED_LIBS` | 构建共享库而非静态库 | OFF |

## 单头文件使用

simdjson 提供单头文件分发形式，适合直接嵌入项目或验证优化效果。

### 生成单头文件

```bash
# 生成单头文件
python3 singleheader/amalgamate.py

# 生成的文件位于：
# - singleheader/simdjson.h    （头文件）
# - singleheader/simdjson.cpp  （实现文件）
```

### 使用单头文件

将 `simdjson.h` 和 `simdjson.cpp` 拷贝到项目中，然后编译：

```bash
# 基础编译
g++ -o your_app your_app.cpp ./simdjson.cpp -I./ -std=c++17

# 带 SVE2 优化编译
g++ -o your_app your_app.cpp ./simdjson.cpp -I./ -std=c++17 \
    -march=armv8-a+sve+crc+sve2 -msve-vector-bits=256
```

## 在项目中集成

### 方式 1：使用已安装的库

```cmake
# CMakeLists.txt
find_package(simdjson REQUIRED)
target_link_libraries(your_target PRIVATE simdjson::simdjson)
```

```bash
# 编译时指定安装路径
g++ -std=c++17 your_app.cpp \
  -I$HOME/.local/simdjson/include \
  -L$HOME/.local/simdjson/lib64 \
  -lsimdjson -o your_app
```

### 方式 2：作为子模块集成

```cmake
# CMakeLists.txt
add_subdirectory(external/simdjson)
target_link_libraries(your_target PRIVATE simdjson)
```

### 方式 3：使用 FetchContent

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

## 测试

### 单元测试

```bash
# 运行所有测试用例
ctest --test-dir build -j

# 运行特定测试
ctest --test-dir build -R dom_tests
ctest --test-dir build -R ondemand_tests
```

### 性能基准测试

```bash
# 1. DOM 解析基准测试
## 单文档 parse()
numactl -C 21 ./build/benchmark/dom/parse -n 1000 ./jsonexamples/twitter.json

## 多文档解析 parse_many()
numactl -C 21 ./build/benchmark/dom/parse_stream ./jsonexamples/amazon_cellphones.ndjson

# 2. Google Benchmark 测试
## 通用解析基准测试
numactl -C 21 ./build/benchmark/bench_parse_call --benchmark_min_time=3s 2>&1

## DOM API 性能测试
numactl -C 21 ./build/benchmark/bench_dom_api --benchmark_min_time=3s 2>&1

## On-Demand API 性能测试
numactl -C 21 ./build/benchmark/bench_ondemand --benchmark_min_time=3s 2>&1

## 多文档解析基准性能测试
numactl -C 21 ./build/benchmark/bench_stream_formats --benchmark_min_time=3s 2>&1
```

## 验证安装

创建测试文件 `example.cpp`：

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
    
    // 查看当前活动实现
    std::cout << "Implementation: " << simdjson::get_active_implementation()->name() << std::endl;
    
    return 0;
}
```

### 编译

使用已安装的静态库编译：

```bash
g++ -std=c++17 example.cpp \
  -I$HOME/.local/simdjson/include \
  -L$HOME/.local/simdjson/lib64 \
  -lsimdjson -o example
```

### 运行和验证

运行程序：

```bash
./example
```

预期输出：

```text
Library: simdjson, Version: 1.0.0
Active implementation: arm64
```

### 验证 SVE2 优化

检查生成的二进制文件中是否包含 SVE2 match 指令：

```bash
objdump -d ./example | grep z0 | grep match
```

预期输出（包含 SVE2 match 指令）：

```text
  413e9c:	45268c00 	match	p0.b, p3/z, z0.b, z6.b
  413ebc:	45278c01 	match	p1.b, p3/z, z0.b, z7.b
```
