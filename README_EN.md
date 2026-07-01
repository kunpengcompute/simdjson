# simdjson Patch Repository

## Latest Updates

- [2026-06-30]: Released the patch repository v1.0.0 based on the master branch commit `c2f25ab`. A table lookup method is introduced to serialization. The AArch64 SVE/SVE2 instruction set is introduced to deserialization stage 1 to accelerate character string scan and character classification.

## Project Introduction

### Overview

simdjson is a high-performance JSON parsing library developed by Daniel Lemire's team. Its core objective is to achieve **high-throughput, low-branching, and low-overhead** JSON parsing and access on general-purpose CPUs. Through SIMD instruction set optimization, simdjson can parse JSON documents at a speed of several gigabytes per second, far outperforming traditional JSON parsers.

This project is a patch repository contributed by Kunpeng to the simdjson open-source community. It optimizes the serialization and deserialization functions in simdjson using SVE/SVE2 instruction sets on Kunpeng processors, especially for the Kunpeng 950 processor. The function interfaces remain consistent with the open-source version of simdjson.

### Core Modules

The overall structure of simdjson is divided into the **public API facade**, **DOM module**, **On-Demand module**, **implementation selection module**, **generic algorithm layer**, and **architecture-specific layer**.

- **Public API facade**: `include/simdjson/base.h`, `include/simdjson/dom.h`, `include/simdjson/ondemand.h`, `include/simdjson/builder.h`, and `include/simdjson/minify.h`
- **DOM module**: `dom::parser`, `dom::document`, `dom::element`, and `dom::document_stream`, providing a document representation for single-pass parsing and random access.
- **On-Demand module**: `ondemand::parser`, `ondemand::document`, `ondemand::document_stream`, and `ondemand::value/object/array`, providing on-demand iteration and value extraction to reduce the overhead of intermediate objects.
- **Implementation selection module**: `implementation`, `get_available_implementations()`, and `get_active_implementation()`, automatically selecting the optimal implementation based on CPU capabilities.
- **Generic algorithm layer**: `src/generic/stage1` and `src/generic/stage2`, implementing core algorithms such as character classification, structural indexing, and Tape construction.
- **Architecture-specific layer**: `arm64`, `haswell`, `icelake`, `westmere`, `ppc64`, `lsx`, `lasx`, `rvv-vls`, and `fallback`, containing SIMD-optimized implementations for different CPU architectures.
- **Serialization/Builder module**: `include/simdjson/dom/serialization.h` and `include/simdjson/generic/builder/*`, providing JSON serialization capabilities.
- **Testing and benchmarking**: `tests/` and `benchmark/`

### Core External API Functions

The following are the core external API functions of simdjson, covering core capabilities such as DOM parsing, on-demand access, runtime implementation selection, utility functions, and serialization.

**DOM Parsing**

| Function/Method| Description|
| --- | --- |
| `dom::parser::parse()` | Parses a single JSON document and returns a `simdjson_result<dom::element>`. |
| `dom::parser::load()` | Loads and parses a single JSON document from a file.|
| `dom::parser::parse_many()` | Parses multiple documents and returns a `dom::document_stream`.|
| `dom::parser::load_many()` | Loads and batch-parses multiple documents from a file.|

**On-Demand Parsing**

| Function/Method| Description|
| --- | --- |
| `ondemand::parser::iterate()` | Iterates through a single JSON document on demand, returning a `simdjson_result<ondemand::document>`.|
| `ondemand::parser::iterate_many()` | Iterates through multiple documents on demand.|

**Serialization**

| Function/Method| Description|
| --- | --- |
| `dom::to_string()` / `dom::minify()` / `dom::operator<<` | Converts a DOM node to a JSON string or writes it to an output stream.|
| `builder::string_builder::append()` | Appends an object, array, string, or scalar to the builder.|
| `builder::to_json_string()` | Serializes an object into a JSON string.|

**Runtime Implementation Selection**

| Function/Method| Description|
| --- | --- |
| `get_available_implementations()` | Retrieves the list of implementations compiled into the library.|
| `get_active_implementation()` | Retrieves or sets the current active implementation.|
| `implementation::supported_by_runtime_system()` | Detects whether the current implementation can be used on the runtime CPU.|

**Utility Functions**

| Function/Method| Description|
| --- | --- |
| `simdjson::minify()` | Quickly removes JSON whitespace characters.|
| `validate_utf8()` | Validates UTF-8 input.|

## Directory Structure

The project directory structure is as follows:

```text
# Documentation directory
README_EN.md                          # Project description
LICENSE                            # Code license
docs/
├── LICENSE                        # Document license
└── en/
    ├── api_reference.md           # API reference
    ├── installation_guide.md      # Installation guide
    ├── quick_start.md             # Quick start guide
    └── release_notes.md           # Release notes
    
# Code directory
simdjson/
├── include/                       # Public header files
│   └── simdjson/
│       ├── base.h                 # Basic type definitions
│       ├── dom.h                  # DOM API
│       ├── ondemand.h             # On-Demand API
│       ├── builder.h              # Builder API
│       ├── minify.h               # Minify API
│       ├── generic/               # Generic implementation
│       ├── internal/              # Internal data structures
│       ├── arm64/                 # ARM64 architecture specialization
│       ├── haswell/               # Haswell architecture specialization
│       ├── icelake/               # Ice Lake architecture specialization
│       └── ...
├── src/                           # Runtime implementation
│   ├── simdjson.cpp               # Unified compilation entry
│   ├── implementation.cpp         # Implementation selection and dispatch
│   ├── arm64.cpp                  # ARM64 implementation
│   ├── generic/                   # Generic algorithms
│   │   ├── stage1/                # Character classification and structural indexing
│   │   └── stage2/                # Tape construction and value parsing
│   └── ...
├── tests/                         # Test cases
├── benchmark/                     # Performance benchmark
├── singleheader/                  # Single-header distribution
├── CMakeLists.txt                 # CMake build configuration
└── ...
```

## Version Description

For details about the feature changes in each release of Kunpeng-optimized simdjson, see [Release Notes](docs/en/release_notes.md).

## Environment Deployment

For details about the compilation environment, dependency obtaining, and installation procedure of Kunpeng-optimized simdjson, see [Installation Guide](docs/en/installation_guide.md).

## Quick Start

For details about how to quickly get started with simdjson, see [Quick Start](docs/en/quick_start.md).

## Documentation

|Document Name|Description|
|--|--|
|[Release Notes](docs/en/release_notes.md)|Provides basic information and feature updates of each release of Kunpeng-optimized simdjson.|
|[Quick Start](docs/en/quick_start.md)|Provides quick start examples and compilation instructions for Kunpeng-optimized simdjson.|
|[API Reference](docs/en/api_reference.md)|Provides descriptions and usage of the core APIs of simdjson.|
|[Installation Guide](docs/en/installation_guide.md)|Provides detailed instructions on configuring, compiling, and installing Kunpeng-optimized simdjson.|

## Disclaimer

**To simdjson users**

- This software is intended solely for debugging and development. You are responsible for any risks and should carefully review the following information:
    - This code repository contributes to the simdjson open-source project solely for performance optimization of certain simdjson functions on Kunpeng processors. It strictly adheres to the coding style and methods, as well as security design of the original open-source software. Any vulnerability and security issues of the software shall be resolved by the corresponding upstream communities according to their response mechanisms. Please pay attention to the notifications and version updates released by the upstream communities. The Kunpeng computing community does not assume any responsibility for software vulnerabilities and security issues.
    - Data processing and deletion: Users are responsible for managing and deleting any data generated while using this software. You are advised to promptly delete any related data after use to prevent information leaks.
    - Data confidentiality and transmission: Users understand and agree not to share or transmit any data generated by this software. Neither the software nor its developers are responsible for any information leaks, data breaches, or other negative consequences.
    - User input security: Users are responsible for the security of any commands they enter and for any risks or losses resulting from improper input. The software and its developers are not liable for issues caused by incorrect command usage.

- Disclaimer scope: This disclaimer applies to all individuals and entities using this software. By using the software, you acknowledge and accept this statement and assume all risks and responsibilities arising from its use. If you do not agree, please stop using the software immediately.
- Before using this software, please **read and understand the preceding disclaimer**. If you have any questions, contact the developer.

**To Data Owners**

If you do not want your dataset to be mentioned in the simdjson patch repository, or if you wish to update its description, please submit an issue on GitCode. We will delete or update your description according to your request. Thank you for your understanding and support.

## License

- The simdjson patch repository is licensed under Apache-2.0. For details, see [LICENSE](LICENSE).

- The documents of this project are licensed under CC-BY 4.0. For details, see [LICENSE](docs/LICENSE).

## Contribution Statement

We welcome your contributions to the community. If you have any questions/suggestions or want to provide feedback on feature requirements and bug reports, you can submit issues. For details, see Contribution Guideline. You are also welcome to share insights in [Discussions](https://gitcode.com/boostkit/community/discussions). Thank you for your support.

## Acknowledgments

The simdjson patch repository is jointly developed by the following Huawei department:

- Kunpeng Computing BoostKit Development Dept

Thank you to everyone in the community for your PRs. We warmly welcome contributions to simdjson!
