# 快速入门

## simdjson基础概念

simdjson提供两种主要的JSON访问模式。

- **DOM模式**：一次解析，构建完整的文档树，支持随机访问和多次遍历。
- **On-Demand模式**：按需遍历，只解析访问到的部分，内存占用更低。

详细的API使用方法和示例请参见《[API参考](./api_reference.md)》。

## 运行时实现选择

simdjson会自动选择适合当前CPU的最佳实现。

```cpp
#include <simdjson.h>
#include <iostream>

int main() {
    // 查看可用的实现
    std::cout << "Available implementations:" << std::endl;
    for (auto impl : simdjson::get_available_implementations()) {
        std::cout << "  - " << impl->name() 
                  << " (" << impl->description() << ")"
                  << " supported: " << impl->supported_by_runtime_system()
                  << std::endl;
    }
    
    // 查看当前活动实现
    std::cout << "Implementation: " << simdjson::get_active_implementation()->name() << std::endl;
    
    return 0;
}
```

在鲲鹏处理器上，您应该看到`arm64`实现处于活跃状态。

## 使用示例

1. 创建`example.cpp`。

   ```cpp
   #include <simdjson.h>
   #include <iostream>
   #include <fstream>

   int main() {
       // 创建测试 JSON 文件
       std::ofstream out("test.json");
       out << R"({
           "library": {
               "name": "simdjson",
               "version": "1.0.0",
               "authors": ["Daniel Lemire", "Geoff Langdale"]
           },
           "performance": {
               "speed": "gigabytes per second",
               "optimizations": ["SIMD", "AVX", "NEON"]
           },
           "downloads": 1000000
       })";
       out.close();
    
       // DOM 模式解析
       simdjson::dom::parser parser;
       simdjson::dom::element doc = parser.load("test.json");
    
       std::cout << "=== DOM Mode ===" << std::endl;
       std::string_view name = doc["library"]["name"];
       std::string_view version = doc["library"]["version"];
       std::cout << name << " v" << version << std::endl;
    
       std::cout << "Authors: ";
       for (auto author : doc["library"]["authors"]) {
           std::cout << std::string_view(author) << " ";
       }
       std::cout << std::endl;
    
       std::cout << "Optimizations: ";
       for (auto opt : doc["performance"]["optimizations"]) {
           std::cout << std::string_view(opt) << " ";
       }
       std::cout << std::endl;
    
       int64_t downloads = doc["downloads"];
       std::cout << "Downloads: " << downloads << std::endl;
    
       return 0;
   }
   ```

2. 编译并运行。

   ```bash
   g++ -o example example.cpp ./simdjson.cpp -I./ -std=c++17
   ./example
   ```

   运行结果如下。

   ```text
   === DOM Mode ===
   simdjson v1.0.0
   Authors: Daniel Lemire Geoff Langdale 
   Optimizations: SIMD AVX NEON 
   Downloads: 1000000
   ```
