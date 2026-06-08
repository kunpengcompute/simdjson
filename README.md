# simdjson补丁仓介绍

## 最新消息

- [2026.06.30]：基于 master 分支 `c2f25ab` 发布补丁仓v1.0.0版本。序列化引入查表法优化，反序列化 Stage1 引入 ARM64 SVE/SVE2 指令集加速字符串扫描与字符分类。

## 项目简介

### 简介

simdjson 是由 Daniel Lemire 团队研发的一款高性能 JSON 解析库，核心目标是在通用 CPU 上实现**高吞吐、低分支、低开销**的 JSON 解析与访问。通过 SIMD 指令集优化，simdjson 能够以每秒数 GB 的速度解析 JSON 文档，性能远超传统 JSON 解析器。

本项目是鲲鹏参与 simdjson 开源社区的补丁仓，基于华为鲲鹏处理器利用 SVE/SVE2 向量化指令集对 simdjson 中的序列化、反序列化函数进行性能优化，适用于鲲鹏 950 处理器，函数接口与开源 simdjson 保持一致。



### 核心模块

simdjson 整体分为**公共 API 门面**、**DOM 模块**、**On-Demand 模块**、**实现选择模块**、**通用算法层**和**架构特化层**，其内部模块划分如下：

- **公共 API 门面**：`include/simdjson/base.h`、`include/simdjson/dom.h`、`include/simdjson/ondemand.h`、`include/simdjson/builder.h`、`include/simdjson/minify.h`
- **DOM 模块**：`dom::parser`、`dom::document`、`dom::element`、`dom::document_stream`，提供一次解析、随机访问的文档表示
- **On-Demand 模块**：`ondemand::parser`、`ondemand::document`、`ondemand::document_stream`、`ondemand::value/object/array`，提供按需遍历与取值，减少中间对象开销
- **实现选择模块**：`implementation`、`get_available_implementations()`、`get_active_implementation()`，根据 CPU 能力自动选择最佳实现
- **通用算法层**：`src/generic/stage1`、`src/generic/stage2`，实现字符分类、结构索引、Tape 构建等核心算法
- **架构特化层**：`arm64`、`haswell`、`icelake`、`westmere`、`ppc64`、`lsx`、`lasx`、`rvv-vls`、`fallback`，针对不同 CPU 架构的 SIMD 优化实现
- **序列化/Builder 模块**：`include/simdjson/dom/serialization.h`、`include/simdjson/generic/builder/*`，提供 JSON 序列化能力
- **测试与基准**：`tests/`、`benchmark/`

### 对外核心接口函数

以下为 simdjson 对外的核心接口函数，覆盖 DOM 解析、On-Demand 访问、运行时实现选择、工具函数和序列化等核心能力。

| 接口函数/方法 | 功能描述 |
| --- | --- |
| **DOM 解析** | |
| `dom::parser::parse()` | 解析单个 JSON 文档，返回 `simdjson_result<dom::element>` |
| `dom::parser::load()` | 从文件加载并解析单个 JSON 文档 |
| `dom::parser::parse_many()` | 解析多个文档，返回 `dom::document_stream` |
| `dom::parser::load_many()` | 从文件加载并批量解析多个文档 |
| **On-Demand 解析** | |
| `ondemand::parser::iterate()` | 按需遍历单个 JSON 文档，返回 `simdjson_result<ondemand::document>` |
| `ondemand::parser::iterate_many()` | 按需遍历多个文档 |
| **序列化** | |
| `dom::to_string()` / `dom::minify()` / `dom::operator<<` | 将 DOM 节点转换为 JSON 字符串或写入输出流 |
| `builder::string_builder::append()` | 追加对象、数组、字符串或标量到 Builder |
| `builder::to_json_string()` | 将对象序列化为 JSON 字符串 |
| **运行时实现选择** | |
| `get_available_implementations()` | 获取编译进库的实现列表 |
| `get_active_implementation()` | 获取/设置当前活动实现 |
| `implementation::supported_by_runtime_system()` | 检测当前实现是否可在运行时 CPU 上使用 |
| **工具函数** | |
| `simdjson::minify()` | 快速去除 JSON 空白字符 |
| `validate_utf8()` | 校验 UTF-8 输入 |

## 目录结构

项目目录层级介绍如下：

```text
# 文档目录
README.md                          # 项目说明
LICENSE                            # 代码许可证
docs/
├── LICENSE                        # 文档许可证
└── zh/
    ├── api_reference.md           # API参考
    ├── installation_guide.md      # 安装指南
    ├── quick_start.md             # 快速入门
    └── release_notes.md           # 版本说明书
    
# 代码目录
simdjson/
├── include/                       # 对外头文件
│   └── simdjson/
│       ├── base.h                 # 基础类型定义
│       ├── dom.h                  # DOM API
│       ├── ondemand.h             # On-Demand API
│       ├── builder.h              # Builder API
│       ├── minify.h               # Minify API
│       ├── generic/               # 通用实现
│       ├── internal/              # 内部数据结构
│       ├── arm64/                 # ARM64 架构特化
│       ├── haswell/               # Haswell 架构特化
│       ├── icelake/               # Ice Lake 架构特化
│       └── ...
├── src/                           # 运行时实现
│   ├── simdjson.cpp               # 统一编译入口
│   ├── implementation.cpp         # 实现选择与调度
│   ├── arm64.cpp                  # ARM64 实现
│   ├── generic/                   # 通用算法
│   │   ├── stage1/                # 字符分类、结构索引
│   │   └── stage2/                # Tape 构建、值解析
│   └── ...
├── tests/                         # 测试用例
├── benchmark/                     # 性能基准
├── singleheader/                  # 单头文件分发
├── CMakeLists.txt                 # CMake 构建配置
└── ...
```

## 版本说明

基于鲲鹏优化的 simdjson 每个发布版本特性变更详细信息，请参见《[版本说明书](docs/zh/release_notes.md)》 。

## 环境部署

基于鲲鹏优化的 simdjson 编译环境、依赖获取与安装步骤参见 《[安装指南](docs/zh/installation_guide.md)》。

## 快速入门

安装 simdjson 后如何快速上手使用 simdjson 请参见《[快速入门](docs/zh/quick_start.md)》。

## 文档

|资源名称|资源简介|
|--|--|
|[版本说明书](docs/zh/release_notes.md)|提供基于鲲鹏优化的 simdjson 发布版本的基础信息和特性更新说明。|
|[快速入门](docs/zh/quick_start.md)|提供基于鲲鹏优化的 simdjson 快速上手示例与编译运行说明。|
|[API参考](docs/zh/api_reference.md)|提供 simdjson 核心 API 接口说明与使用方法。|
|[安装指南](docs/zh/installation_guide.md)|提供基于鲲鹏优化代码 simdjson 的环境配置与编译安装的详细指导。|

## 免责声明

**致 simdjson 使用者**

- 本软件仅供调试和开发之用，使用者需自行承担使用风险，并理解以下内容：
    - 此代码仓计划参与 simdjson 软件开源，仅对 simdjson 部分函数在鲲鹏处理器上进行性能优化，编码风格遵照原生开源软件，继承原生开源软件安全设计，不破坏原生开源软件设计及编码风格和方式。软件的任何漏洞与安全问题，均由相应的上游社区根据其漏洞和安全响应机制解决。请密切关注上游社区发布的通知和版本更新。鲲鹏计算社区对软件的漏洞及安全问题不承担任何责任。
    - 数据处理及删除：用户在使用本软件过程中产生的数据属于用户责任范畴。建议用户在使用完毕后及时删除相关数据，以防信息泄露。
    - 数据保密与传播：使用者了解并同意不得将通过本软件产生的数据随意外发或传播。对于由此产生的信息泄露、数据泄露或其他不良后果，本软件及其开发者概不负责。
    - 用户输入安全性：用户需自行保证输入的命令行的安全性，并承担因输入不当而导致的任何安全风险或损失。对于输入命令行不当所导致的问题，本软件及其开发者概不负责。

- 免责声明范围：本免责声明适用于所有使用本软件的个人或实体。使用本软件即表示您同意并接受本声明的内容，并愿意承担因使用该功能而产生的风险和责任，如有异议请停止使用本软件。
- 在使用本软件之前，请**谨慎阅读并理解以上免责声明的内容**。对于使用本软件所产生的任何问题或疑问，请及时联系开发者。

**致数据所有者**

如果您不希望您的数据集等信息在 simdjson 补丁仓中被提及，或希望更新相关描述，请在 GitCode 提交 issue，我们将根据您的要求删除或更新相关描述。感谢您的理解与支持。

## License

- simdjson 补丁仓采用 Apache-2.0 许可证，具体请参见[LICENSE文件](LICENSE)。

- simdjson 项目的文档适用 CC-BY 4.0 许可证，具体请参见[LICENSE文件](docs/LICENSE)。

## 贡献声明

欢迎大家为社区做贡献，如果使用过程中有任何问题/建议，或者需要反馈特性需求和 bug 报告，可以提交 [Issues](https://gitcode.com/boostkit/community/blob/master/docs/contributor/issue-submit.md) 联系我们，具体贡献方法可参考[这里](https://gitcode.com/boostkit/community/blob/master/docs/contributor/contributing.md)。同时也欢迎大家在[讨论专区](https://gitcode.com/boostkit/community/discussions)展开讨论交流。感谢您的支持。

## 致谢

simdjson 补丁仓由华为公司的下列部门联合贡献：

- 鲲鹏计算 Boostkit 开发部

感谢来自社区的每一个 PR，欢迎贡献 simdjson！