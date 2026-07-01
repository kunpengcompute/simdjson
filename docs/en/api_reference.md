# API Reference

## Functions

[**Table 1**](#optimized-functions-in-the-simdjson-patch-repository) lists the optimized functions in the simdjson patch repository.

**Table 1** Optimized functions in the simdjson patch repository<a id="optimized-functions-in-the-simdjson-patch-repository"></a>

|Name|Description|
|--|--|
|parse|Parses a JSON string into a DOM document object. In stage 1, character classification and string scan are optimized using SVE2 instructions.|
|iterate|Iterates through a JSON document on demand. In stage 1, character classification and string scan are optimized using SVE2 instructions.|
|to_string/minify|Serializes DOM nodes into a JSON string, optimizing integer conversion and string escaping through lookup tables.|
|json_minifier::minify|Quickly removes whitespace from an original JSON string. In stage 1, character classification is optimized using SVE2 instructions.|

## Function Definitions

### parse

**Function Usage**

Parses a JSON string into a DOM object, supporting full JSON syntax parsing, including objects, arrays, strings, numbers, boolean values, and NULL.

**Function Syntax**

```cpp
simdjson::dom::parser parser;
simdjson_result<element> parser.parse(const padded_string &s) noexcept;
```

**Parameters**

|Parameter|Description|Value Range|Input/Output|
|--|--|--|--|
|s|JSON string.|`padded_string` type|Input|

**Return Value**

Returns `simdjson_result<element>`, which contains the parsed DOM root element or an error code.

> **Note:**
>Syntax check and UTF-8 validation are performed during parsing. If the JSON format is incorrect, you can check the error code via the `.error()` method.
>
**Regarding padding usage:** simdjson uses SIMD instructions to process data in batches and reads an additional `SIMDJSON_PADDING` (64 bytes by default) at the end of the input for performance optimization and to guarantee correctness. Therefore, the input buffer must contain these extra bytes. Recommended usage:

- `_padded` literal suffix (for string literals)
- `padded_string::load("file.json")` (for loading from a file)
- `padded_string` type (constructed from `std::string`)

**Example**

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

    // Check the current implementation.
    std::cout << "Implementation: " << simdjson::get_active_implementation()->name() << std::endl;

    return 0;
}
```

### iterate

**Function Usage**

Iterates through a JSON document on demand, parsing only the accessed parts. This is suitable for scenarios involving large JSON documents or when only specific fields need to be accessed.

**Function Syntax**

```cpp
simdjson::ondemand::parser parser;
simdjson_result<document> parser.iterate(const padded_string &s) noexcept;
```

**Parameters**

|Parameter|Description|Value Range|Input/Output|
|--|--|--|--|
|s|JSON string.|`padded_string` type|Input|

**Return Value**

Returns `simdjson_result<document>`, which contains the document that can be iterated on demand or an error code.

>**Note:**
>The on-demand mode requires fields to be accessed in the order they appear in the document; random access and backtracking are not supported. The field access sequence should match the sequence in the JSON document to achieve optimal performance.

**Example**

```cpp
#include <simdjson.h>
#include <iostream>

int main() {
    simdjson::ondemand::parser parser;
    auto json = R"({
        "user": {
            "id": 12345,
            "name": "zhangsan",
            "email": "zhangsan@example.com"
        },
        "total": 100
    })"_padded;
    
    simdjson::ondemand::document doc = parser.iterate(json);
    
    // Parse only the accessed fields.
    int64_t user_id = doc["user"]["id"];
    std::string_view user_name = doc["user"]["name"];
    
    std::cout << "User ID: " << user_id << std::endl;
    std::cout << "User Name: " << user_name << std::endl;
    
    return 0;
}
```

### to_string/minify

**Function Usage**

Serializes a parsed DOM element into a minified JSON string. `minify(doc)` and `to_string(doc)` are the same interface.

**Function Syntax**

```cpp
std::string simdjson::to_string(element value);
std::string simdjson::minify(element value);  // Equivalent to to_string
```

**Parameters**

|Parameter|Description|Value Range|Input/Output|
|--|--|--|--|
|value|DOM element.|Valid DOM object|Input|

**Return Value**

Returns the serialized JSON string (minified format, without redundant whitespace).

**Example**

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
    
    // Method 1: Using to_string
    std::string json_str = simdjson::to_string(doc);
    std::cout << "to_string: " << json_str << std::endl;
    
    // Method 2: Using minify (equivalent)
    std::string minified = simdjson::minify(doc);
    std::cout << "minify: " << minified << std::endl;
    
    // Method 3: Output directly to stream
    std::cout << "Direct output: " << doc << std::endl;
    
    return 0;
}
```

### json_minifier::minify

**Function Usage**

Quickly removes whitespace characters (spaces, tabs, newlines, etc.) from an original JSON string to generate a compact JSON string. This function does not perform parsing or validation; it performs byte operations directly using SIMD instructions, offering high performance.

**Function Syntax**

```cpp
error_code simdjson::minify(
    const char *buf,
    size_t len,
    char *dst,
    size_t &dst_len
) noexcept;
```

**Parameters**

|Parameter|Description|Value Range|Input/Output|
|--|--|--|--|
|buf|Input JSON string buffer.|Non-null pointer|Input|
|len|Input buffer length.|Non-negative integer|Input|
|dst|Output buffer.|Non-null pointer, at least as large as the input|Output|
|dst_len|Output length.|Non-negative integer|Output|

**Return Value**

- Success: Returns `SUCCESS` (error code 0).
- Failure: Returns the corresponding error code.

The minified result is stored in the `dst` buffer, and the actual length is returned via `dst_len`.

>**Note:**
>This function does not parse or validate JSON; it only removes whitespace characters. If you need to validate the JSON format, use the `parse()` or `iterate()` function.

**Example**

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
