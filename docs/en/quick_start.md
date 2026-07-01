# Quick Start

## Basic Concepts of simdjson

simdjson provides two main JSON access modes:

- **DOM Mode**: Parses JSON at once to build a complete document tree, supporting random access and multiple iterations.
- **On-Demand Mode**: Iterates through JSON on demand, parsing only the accessed components, resulting in lower memory usage.

For detailed API usage and examples, see [API Reference](./api_reference.md).

## Runtime Implementation Selection

simdjson automatically selects the optimal implementation for the current CPU.

```cpp
#include <simdjson.h>
#include <iostream>

int main() {
    // Check available implementations.
    std::cout << "Available implementations:" << std::endl;
    for (auto impl : simdjson::get_available_implementations()) {
        std::cout << "  - " << impl->name() 
                  << " (" << impl->description() << ")"
                  << " supported: " << impl->supported_by_runtime_system()
                  << std::endl;
    }
    
    // Check the active implementation.
    std::cout << "Implementation: " << simdjson::get_active_implementation()->name() << std::endl;
    
    return 0;
}
```

On the Kunpeng processor, you should see the `arm64` implementation active, with SVE2 optimization support.

## Usage Example

1. Create a `example.cpp` file.

   ```cpp
   #include <simdjson.h>
   #include <iostream>
   #include <fstream>

   int main() {
       // Create a test JSON file.
       std::ofstream out("test.json");
       out << R"({
           "library": {
               "name": "simdjson",
               "version": "1.0.0",
               "authors": ["Daniel Lemire", "Geoff Langdale"]
           },
           "performance": {
               "speed": "gigabytes per second",
               "optimizations": ["SIMD", "SVE2", "NEON"]
           },
           "downloads": 1000000
       })";
       out.close();
    
       // DOM mode parsing
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

2. Build and run.

   ```bash
   g++ -o example example.cpp ./simdjson.cpp -I./ -std=c++17
   ./example
   ```

   The execution result is as follows:

   ```text
   === DOM Mode ===
   simdjson v1.0.0
   Authors: Daniel Lemire Geoff Langdale 
   Optimizations: SIMD SVE2 NEON 
   Downloads: 1000000
   ```
