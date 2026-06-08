# API参考

## 函数说明

simdjson 补丁仓已优化函数如[**表 1** simdjson 补丁仓已优化函数列表](#simdjson补丁仓已优化函数列表)所示。

**表 1** simdjson 补丁仓已优化函数列表<a id="simdjson补丁仓已优化函数列表"></a>

|名称|说明|
|--|--|
|parse|将 JSON 字符串解析为 DOM 文档对象，Stage 1 阶段通过 SVE2 指令优化字符分类与字符串扫描。|
|iterate|按需遍历 JSON 文档，Stage 1 阶段通过 SVE2 指令优化字符分类与字符串扫描。|
|to_string / minify|将 DOM 节点序列化为 JSON 字符串，通过查找表优化整数转换与字符串转义。|
|json_minifier::minify|快速去除原始 JSON 字符串的空白字符，Stage 1 阶段通过 SVE2 指令优化字符分类。|

## 函数定义

### parse

**函数功能**

将 JSON 字符串解析为 DOM 文档对象，支持完整的 JSON 语法解析，包括对象、数组、字符串、数字、布尔值和 null。

**函数定义**

```cpp
simdjson::dom::parser parser;
simdjson_result<element> parser.parse(const padded_string &s) noexcept;
```

**参数说明**

|参数名|描述|取值范围|输入/输出|
|--|--|--|--|
|s|JSON 字符串|padded_string 类型|输入|

**返回值**

返回 `simdjson_result<element>`，包含解析后的 DOM 根元素或错误码。

>![](public_sys-resources/icon-note.gif) **说明：** 
>解析过程中会进行语法检查和 UTF-8 校验。如果 JSON 格式不正确，可通过 `.error()` 方法检查错误码。
>
>**关于 padding 的使用：** simdjson 使用 SIMD 指令批量处理数据，会在输入末尾读取额外的 `SIMDJSON_PADDING`（默认 64 字节）用于性能优化和保证正确性。因此输入缓冲区必须包含这些额外字节。推荐使用：
>- `_padded` 字面量后缀（字符串字面量）
>- `padded_string::load("file.json")`（从文件加载）
>- `padded_string` 类型（从 std::string 构造）

**示例**

```cpp
#include <simdjson.h>
#include <iostream>

int main() {
    simdjson::dom::parser parser;
    auto json = R"({"name": "simdjson", "version": "1.0.0"})"_padded;
    
    simdjson::dom::element doc = parser.parse(json);

    std::string_view name = doc["name"];
    std::string_view version = doc["version"];

    std::cout << "Library: " << name << std::endl;
    std::cout << "Version: " << version << std::endl;

    // 查看当前使用的实现
    std::cout << "Implementation: " << simdjson::get_active_implementation()->name() << std::endl;

    return 0;
}
```

### iterate

**函数功能**

按需遍历 JSON 文档，只解析访问到的部分，适合处理大型 JSON 文档或仅需访问部分字段的场景。

**函数定义**

```cpp
simdjson::ondemand::parser parser;
simdjson_result<document> parser.iterate(const padded_string &s) noexcept;
```

**参数说明**

|参数名|描述|取值范围|输入/输出|
|--|--|--|--|
|s|JSON 字符串|padded_string 类型|输入|

**返回值**

返回 `simdjson_result<document>`，包含可按需遍历的文档或错误码。

>![](public_sys-resources/icon-note.gif) **说明：** 
>On-Demand 模式要求按文档顺序访问字段，不支持随机访问和回溯。访问字段的顺序应与 JSON 文档中的顺序一致，以获得最佳性能。

**示例**

```cpp
#include <simdjson.h>
#include <iostream>

int main() {
    simdjson::ondemand::parser parser;
    auto json = R"({
        "user": {
            "id": 12345,
            "name": "张三",
            "email": "zhangsan@example.com"
        },
        "total": 100
    })"_padded;
    
    simdjson::ondemand::document doc = parser.iterate(json);
    
    // 只解析访问的字段
    int64_t user_id = doc["user"]["id"];
    std::string_view user_name = doc["user"]["name"];
    
    std::cout << "User ID: " << user_id << std::endl;
    std::cout << "User Name: " << user_name << std::endl;
    
    return 0;
}
```

### to_string / minify

**函数功能**

将已解析的 DOM 元素序列化为 minified JSON 字符串。`minify(doc)` 和 `to_string(doc)` 是同一个接口。

**函数定义**

```cpp
std::string simdjson::to_string(element value);
std::string simdjson::minify(element value);  // 等价于 to_string
```

**参数说明**

|参数名|描述|取值范围|输入/输出|
|--|--|--|--|
|value|DOM 元素|有效的 DOM 对象|输入|

**返回值**

返回序列化后的 JSON 字符串（minified 格式，无多余空白）。

**示例**

```cpp
#include <simdjson.h>
#include <iostream>

int main() {
    simdjson::dom::parser parser;
    auto json = R"({
        "name": "simdjson",
        "version": "1.0.0",
        "count": 42
    })"_padded;
    
    simdjson::dom::element doc = parser.parse(json);
    
    // 方式 1：使用 to_string
    std::string json_str = simdjson::to_string(doc);
    std::cout << "to_string: " << json_str << std::endl;
    
    // 方式 2：使用 minify（等价）
    std::string minified = simdjson::minify(doc);
    std::cout << "minify: " << minified << std::endl;
    
    // 方式 3：直接输出到流
    std::cout << "Direct output: " << doc << std::endl;
    
    return 0;
}
```

### json_minifier::minify

**函数功能**

快速去除原始 JSON 字符串中的空白字符（空格、制表符、换行符等），生成紧凑的 JSON 字符串。此函数不进行解析或验证，直接使用 SIMD 指令进行字节操作，性能极高。

**函数定义**

```cpp
error_code simdjson::minify(
    const char *buf,
    size_t len,
    char *dst,
    size_t &dst_len
) noexcept;
```

**参数说明**

|参数名|描述|取值范围|输入/输出|
|--|--|--|--|
|buf|输入 JSON 字符串缓冲区|非空指针|输入|
|len|输入缓冲区长度|非负整数|输入|
|dst|输出缓冲区|非空指针，至少与输入同样大|输出|
|dst_len|输出长度|非负整数|输出|

**返回值**

- 成功：返回 `SUCCESS`（错误码为 0）
- 失败：返回相应的错误码

Minify 后的结果存储在 `dst` 缓冲区中，实际长度通过 `dst_len` 返回。

>![](public_sys-resources/icon-note.gif) **说明：** 
>此函数不解析或验证 JSON，仅移除空白字符。如果需要验证 JSON 格式，请使用 `parse()` 或 `iterate()` 函数。

**示例**

```cpp
#include <simdjson.h>
#include <iostream>
#include <string>

int main() {
    std::string json = R"(
        {
            "name"  :  "simdjson"  ,
            "version"  :  "1.0.0"
        }
    )";
    
    std::string output(json.size(), '\0');
    size_t output_len;
    
    auto error = simdjson::minify(
        json.data(), 
        json.size(),
        output.data(), 
        output_len
    );
    
    if (!error) {
        output.resize(output_len);
        std::cout << "Original size: " << json.size() << std::endl;
        std::cout << "Minified size: " << output_len << std::endl;
        std::cout << "Minified JSON: " << output << std::endl;
    } else {
        std::cout << "Minify failed: " << error << std::endl;
    }
    
    return 0;
}
```