# 版本说明书

## 版本配套说明

### 产品版本信息

<a name="table62675726"></a>
<table><tbody><tr id="row41561572"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.1.1"><p id="p11044137"><a name="p11044137"></a><a name="p11044137"></a>产品名称</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.1.1 "><p id="p1597721693713"><a name="p1597721693713"></a><a name="p1597721693713"></a>Kunpeng BoostKit</p>
</td>
</tr>
<tr id="row24726251"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.2.1"><p id="p56669300"><a name="p56669300"></a><a name="p56669300"></a>产品版本</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.2.1 "><p id="p11923034"><a name="p11923034"></a><a name="p11923034"></a><span id="text14311218114"><a name="text14311218114"></a><a name="text14311218114"></a>26.1.RC1</span></p>
</td>
</tr>
<tr id="row1930811171892"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.3.1"><p id="p2030912172097"><a name="p2030912172097"></a><a name="p2030912172097"></a>软件名称</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.3.1 "><p id="p1730912179911"><a name="p1730912179911"></a><a name="p1730912179911"></a><span id="text17191017111119"><a name="text17191017111119"></a><a name="text17191017111119"></a>simdjson（Kunpeng simdjson优化版）</span></p>
</td>
</tr>
<tr id="row1930811171892"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.3.1"><p id="p2030912172097"><a name="p2030912172097"></a><a name="p2030912172097"></a>软件版本</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.3.1 "><p id="p1730912179911"><a name="p1730912179911"></a><a name="p1730912179911"></a><span id="text17191017111119"><a name="text17191017111119"></a><a name="text17191017111119"></a>V1.0.0</span></p>
</td>
</tr>
</tbody>
</table>

### 与操作系统、编译器和CPU配套说明

|操作系统|CPU类型|编译器|
|--|--|--|
|openEuler 24.03 LTS SP3|鲲鹏950处理器（支持SVE2）|Clang 16.0.6|

## 版本更新说明

### V1.0.0

**新增特性**

|特性描述|更新说明|
|--|--|
|Stage 1字符分类SVE2优化|在ARM64平台引入SVE2 `svmatch_u8` 指令优化 `classify` 函数，通过单指令集合匹配替代原NEON查表与位运算，指令数减少40-50%，Stage 1扫描性能提升15-25%。|
|Stage 1字符串扫描SVE优化|使用SVE指令优化 `json_string_scanner::next` 函数，通过 `svcmpeq_n_u8` 快速识别引号和反斜杠，配合SVE加速的转义位图处理，显著降低大段字符串扫描的分支预测压力。|
|序列化整数转换优化|引入双位查找表 `kDigits100` 优化 `fast_itoa` 函数，采用每次剥离两位数字的方式，循环次数减半，显著降低算术运算开销。|
|序列化字符串转义优化|采用8字节循环展开扫描配合布尔位图 `kNeedEscaped`，并使用 `kQuoteTab` 映射表替代分支判断，实现快速路径跳过与分段批量拷贝，最大化 `memcpy` 利用率。|

**兼容性说明**

- SVE2优化需要硬件支持SVE2指令集（鲲鹏950处理器）
- 当硬件不支持SVE2时，自动回退至高效的NEON实现，确保兼容性
- 所有优化保持与原生simdjson API完全兼容，无需修改应用代码

## 版本配套文档

### V1.0.0版本配套文档

<a name="table41916133"></a>
<table><thead align="left"><tr id="row28804032"><th class="cellrowborder" valign="top" width="45.019999999999996%" id="mcps1.1.4.1.1"><p id="p4697041"><a name="p4697041"></a><a name="p4697041"></a>文档名称</p>
</th>
<th class="cellrowborder" valign="top" width="38.019999999999996%" id="mcps1.1.4.1.2"><p id="p44916036"><a name="p44916036"></a><a name="p44916036"></a>内容简介</p>
</th>
<th class="cellrowborder" valign="top" width="16.96%" id="mcps1.1.4.1.3"><p id="p14320308"><a name="p14320308"></a><a name="p14320308"></a>交付形式</p>
</th>
</tr>
</thead>
<tbody><tr id="row19094280">
<td class="cellrowborder" valign="top" width="40.510000000000005%" headers="mcps1.1.4.1.1"><p id="p1341193722116">《版本说明书》</p></td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p2042183752117"><a name="p2042183752117"></a>提供基于鲲鹏优化的simdjson每个发布版本的基础信息和特性更新说明。</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p12419194564814"><a name="p12419194564814"></a><a name="p12419194564814"></a>开源仓</p>
</td>
</tr>
<tr id="row17314122818119"><td class="cellrowborder" valign="top" width="45.019999999999996%" headers="mcps1.1.4.1.1 "><p id="p9534164291118"><a name="p9534164291118"></a><a name="p9534164291118"></a>《安装指南》</p>
</td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p1353414214111"><a name="p1353414214111"></a><a name="p1353414214111"></a>提供基于鲲鹏优化代码simdjson的环境配置与编译安装的详细指导。</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p1164511541488"><a name="p1164511541488"></a><a name="p1164511541488"></a>开源仓</p>
</td>
</tr>
<tr id="row1021013311110"><td class="cellrowborder" valign="top" width="45.019999999999996%" headers="mcps1.1.4.1.1 "><p id="p18535154291116"><a name="p18535154291116"></a><a name="p18535154291116"></a>《快速入门》</p>
</td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p1353554215117"><a name="p1353554215117"></a><a name="p1353554215117"></a>提供基于鲲鹏优化的simdjson快速上手示例与编译运行说明。</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p2646175434816"><a name="p2646175434816"></a><a name="p2646175434816"></a>开源仓</p>
</td>
</tr>

<tr id="row1021013311110"><td class="cellrowborder" valign="top" width="45.019999999999996%" headers="mcps1.1.4.1.1 "><p id="p18535154291116"><a name="p18535154291116"></a><a name="p18535154291116"></a>《API参考》</p>
</td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p1353554215117"><a name="p1353554215117"></a><a name="p1353554215117"></a>提供simdjson核心API接口说明与使用方法。</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p2646175434816"><a name="p2646175434816"></a><a name="p2646175434816"></a>开源仓</p>
</td>
</tr>
</tbody>
</table>

### 获取文档的方法

您可以通过访问[开源仓](https://gitcode.com/boostkit/simdjson)浏览和获取相关文档。
