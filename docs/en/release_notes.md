# Release Notes

## Version Mapping

### Product Version Information

<a name="table62675726"></a>
<table><tbody><tr id="row41561572"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.1.1"><p id="p11044137"><a name="p11044137"></a><a name="p11044137"></a>Product Name</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.1.1 "><p id="p1597721693713"><a name="p1597721693713"></a><a name="p1597721693713"></a>Kunpeng BoostKit</p>
</td>
</tr>
<tr id="row24726251"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.2.1"><p id="p56669300"><a name="p56669300"></a><a name="p56669300"></a>Product Version</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.2.1 "><p id="p11923034"><a name="p11923034"></a><a name="p11923034"></a><span id="text14311218114"><a name="text14311218114"></a><a name="text14311218114"></a>26.1.RC1</span></p>
</td>
</tr>
<tr id="row1930811171892"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.3.1"><p id="p2030912172097"><a name="p2030912172097"></a><a name="p2030912172097"></a>Software Name</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.3.1 "><p id="p1730912179911"><a name="p1730912179911"></a><a name="p1730912179911"></a><span id="text17191017111119"><a name="text17191017111119"></a><a name="text17191017111119"></a>simdjson (Kunpeng-optimized)</span></p>
</td>
</tr>
<tr id="row1930811171892"><th class="firstcol" valign="top" width="42.17%" id="mcps1.1.3.3.1"><p id="p2030912172097"><a name="p2030912172097"></a><a name="p2030912172097"></a>Software Version</p>
</th>
<td class="cellrowborder" valign="top" width="57.830000000000005%" headers="mcps1.1.3.3.1 "><p id="p1730912179911"><a name="p1730912179911"></a><a name="p1730912179911"></a><span id="text17191017111119"><a name="text17191017111119"></a><a name="text17191017111119"></a>V1.0.0</span></p>
</td>
</tr>
</tbody>
</table>

### OS, Compiler, and CPU

|OS|CPU|Compiler|
|--|--|--|
|openEuler 24.03 LTS SP3|Kunpeng 950 processor (supporting SVE2)|LLVM Clang 16.0.6|

## Version Updates

### V1.0.0

**New Features**

|Feature|Description|
|--|--|
|Stage 1 character classification SVE2 optimization|Introduces the SVE2 `svmatch_u8` instruction on the AArch64 platform to optimize the `classify` function. By replacing the original NEON table lookup and bitwise operations with single-instruction set matching, the instruction count is reduced by 40% to 50%, and stage 1 scan performance is improved by 15% to 25%.|
|Stage 1 string scan SVE optimization|Optimizes the `json_string_scanner::next` function using SVE instructions. It leverages `svcmpeq_n_u8` to quickly identify quotation marks and backslashes, combining it with SVE-accelerated escape bitmap processing to significantly reduce branch prediction pressure during long string scan.|
|Serialized integer conversion optimization|Introduces a two-digit lookup table `kDigits100` to optimize the `fast_itoa` function. By stripping two digits at a time, the number of loop iterations is halved, substantially reducing arithmetic overhead.|
|Serialized string escaping optimization|Applies an 8-byte unrolled loop scan combined with a boolean bitmap `kNeedEscaped`, and utilizes the `kQuoteTab` mapping table instead of branch judgments. This enables fast-path skipping and segmented bulk copying, maximizing `memcpy` utilization.|

**Compatibility**

- SVE2 optimization requires hardware support for the SVE2 instruction set (Kunpeng 950 processor).
- If the hardware does not support SVE2, it automatically falls back to the efficient NEON implementation, ensuring compatibility.
- All optimizations maintain full compatibility with the open-source simdjson APIs, requiring no modifications to the application code.

## Related Documentation

### V1.0.0 Documentation

<a name="table41916133"></a>
<table><thead align="left"><tr id="row28804032"><th class="cellrowborder" valign="top" width="45.019999999999996%" id="mcps1.1.4.1.1"><p id="p4697041"><a name="p4697041"></a><a name="p4697041"></a>Document Name</p>
</th>
<th class="cellrowborder" valign="top" width="38.019999999999996%" id="mcps1.1.4.1.2"><p id="p44916036"><a name="p44916036"></a><a name="p44916036"></a>Description</p>
</th>
<th class="cellrowborder" valign="top" width="16.96%" id="mcps1.1.4.1.3"><p id="p14320308"><a name="p14320308"></a><a name="p14320308"></a>Delivery Method</p>
</th>
</tr>
</thead>
<tbody><tr id="row19094280">
<td class="cellrowborder" valign="top" width="40.510000000000005%" headers="mcps1.1.4.1.1"><p id="p1341193722116">Release Notes</p></td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p2042183752117"><a name="p2042183752117"></a>Provides basic information and feature updates for each release of Kunpeng-optimized simdjson.</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p12419194564814"><a name="p12419194564814"></a><a name="p12419194564814"></a>Open-source repository</p>
</td>
</tr>
<tr id="row17314122818119"><td class="cellrowborder" valign="top" width="45.019999999999996%" headers="mcps1.1.4.1.1 "><p id="p9534164291118"><a name="p9534164291118"></a><a name="p9534164291118"></a>Installation Guide</p>
</td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p1353414214111"><a name="p1353414214111"></a><a name="p1353414214111"></a>Provides detailed guidance on environment configuration, compilation, and installation for Kunpeng-optimized simdjson.</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p1164511541488"><a name="p1164511541488"></a><a name="p1164511541488"></a>Open-source repository</p>
</td>
</tr>
<tr id="row1021013311110"><td class="cellrowborder" valign="top" width="45.019999999999996%" headers="mcps1.1.4.1.1 "><p id="p18535154291116"><a name="p18535154291116"></a><a name="p18535154291116"></a>Quick Start</p>
</td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p1353554215117"><a name="p1353554215117"></a><a name="p1353554215117"></a>Provides a quick start example and compilation/running instructions for Kunpeng-optimized simdjson.</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p2646175434816"><a name="p2646175434816"></a><a name="p2646175434816"></a>Open-source repository</p>
</td>
</tr>

<tr id="row1021013311110"><td class="cellrowborder" valign="top" width="45.019999999999996%" headers="mcps1.1.4.1.1 "><p id="p18535154291116"><a name="p18535154291116"></a><a name="p18535154291116"></a>API Reference</p>
</td>
<td class="cellrowborder" valign="top" width="38.019999999999996%" headers="mcps1.1.4.1.2 "><p id="p1353554215117"><a name="p1353554215117"></a><a name="p1353554215117"></a>Provides the description and usage of the core simdjson APIs.</p>
</td>
<td class="cellrowborder" valign="top" width="16.96%" headers="mcps1.1.4.1.3 "><p id="p2646175434816"><a name="p2646175434816"></a><a name="p2646175434816"></a>Open-source repository</p>
</td>
</tr>
</tbody>
</table>

### Obtaining Documentation

Visit the [open-source repository](https://gitcode.com/boostkit/simdjson) to view or download related documents.
