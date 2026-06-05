
#ifndef SIMDJSON_SERIALIZATION_INL_H
#define SIMDJSON_SERIALIZATION_INL_H

#include "simdjson/dom/base.h"
#include "simdjson/dom/parser.h"
#include "simdjson/dom/serialization.h"
#include "simdjson/internal/tape_type.h"

#include "simdjson/dom/array-inl.h"
#include "simdjson/dom/object-inl.h"
#include "simdjson/internal/tape_ref-inl.h"

#include <cmath>
#include <cstring>

namespace simdjson {
namespace dom {
inline bool parser::print_json(std::ostream &os) const noexcept {
  if (!valid) {
    return false;
  }
  simdjson::internal::string_builder<> sb;
  sb.append(doc.root());
  std::string_view answer = sb.str();
  os << answer;
  return true;
}

inline std::ostream &operator<<(std::ostream &out,
                                simdjson::dom::element value) {
  simdjson::internal::string_builder<> sb;
  sb.append(value);
  return (out << sb.str());
}
#if SIMDJSON_EXCEPTIONS
inline std::ostream &
operator<<(std::ostream &out,
           simdjson::simdjson_result<simdjson::dom::element> x) {
  if (x.error()) {
    throw simdjson::simdjson_error(x.error());
  }
  return (out << x.value());
}
#endif
inline std::ostream &operator<<(std::ostream &out, simdjson::dom::array value) {
  simdjson::internal::string_builder<> sb;
  sb.append(value);
  return (out << sb.str());
}
#if SIMDJSON_EXCEPTIONS
inline std::ostream &
operator<<(std::ostream &out,
           simdjson::simdjson_result<simdjson::dom::array> x) {
  if (x.error()) {
    throw simdjson::simdjson_error(x.error());
  }
  return (out << x.value());
}
#endif
inline std::ostream &operator<<(std::ostream &out,
                                simdjson::dom::object value) {
  simdjson::internal::string_builder<> sb;
  sb.append(value);
  return (out << sb.str());
}
#if SIMDJSON_EXCEPTIONS
inline std::ostream &
operator<<(std::ostream &out,
           simdjson::simdjson_result<simdjson::dom::object> x) {
  if (x.error()) {
    throw simdjson::simdjson_error(x.error());
  }
  return (out << x.value());
}
#endif

} // namespace dom

/***
 * Number utility functions
 **/
namespace {
/**@private
 * Escape sequence like \b or \u0001
 * We expect that most compilers will use 8 bytes for this data structure.
 **/
struct escape_sequence {
  uint8_t length;
  const char
      string[7]; // technically, we only ever need 6 characters, we pad to 8
};
/**@private
 * This converts a signed integer into a character sequence.
 * The caller is responsible for providing enough memory (at least
 * 20 characters.)
 * Though various runtime libraries provide itoa functions,
 * it is not part of the C++ standard. The C++17 standard
 * adds the to_chars functions which would do as well, but
 * we want to support C++11.
 */
static char *fast_itoa(char *output, int64_t value) noexcept {
  // This is a standard implementation of itoa.
  char buffer[20];
  uint64_t value_positive;
  // In general, negating a signed integer is unsafe.
  if (value < 0) {
    *output++ = '-';
    // Doing value_positive = -value; while avoiding
    // undefined behavior warnings.
    // It assumes two complement's which is universal at this
    // point in time.
    std::memcpy(&value_positive, &value, sizeof(value));
    value_positive = (~value_positive) + 1; // this is a negation
  } else {
    value_positive = value;
  }
  // We work solely with value_positive. It *might* be easier
  // for an optimizing compiler to deal with an unsigned variable
  // as far as performance goes.
  const char *const end_buffer = buffer + 20;
  char *write_pointer = buffer + 19;
  // A faster approach is possible if we expect large integers:
  // unroll the loop (work in 100s, 1000s) and use some kind of
  // memoization.
  while (value_positive >= 10) {
    *write_pointer-- = char('0' + (value_positive % 10));
    value_positive /= 10;
  }
  *write_pointer = char('0' + value_positive);
  size_t len = end_buffer - write_pointer;
  std::memcpy(output, write_pointer, len);
  return output + len;
}

/**@private
 * Lazy-initialized two-digit lookup table for faster itoa.
 * Only allocated when serialization is actually used.
 */
static inline const char* get_digits_table() noexcept {
  static const char kDigits100[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
  };
  return kDigits100;
}

/**@private
 * This converts an unsigned integer into a character sequence.
 * The caller is responsible for providing enough memory (at least
 * 19 characters.)
 * Though various runtime libraries provide itoa functions,
 * it is not part of the C++ standard. The C++17 standard
 * adds the to_chars functions which would do as well, but
 * we want to support C++11.
 */
static char *fast_itoa(char *output, uint64_t value) noexcept {
  char buffer[20];
  char *write_pointer = buffer + 19;

  const char* digits = get_digits_table();

  // Process two digits at a time using lookup table
  while (value >= 100) {
    const uint64_t q = value / 100;
    const uint64_t r = value % 100;
    value = q;
    const char* d = digits + r * 2;
    *write_pointer-- = d[1];
    *write_pointer-- = d[0];
  }

  // Handle remaining 1-2 digits
  if (value >= 10) {
    const char* d = digits + value * 2;
    *write_pointer-- = d[1];
    *write_pointer = d[0];
  } else {
    *write_pointer = char('0' + value);
  }

  const char *const end_buffer = buffer + 20;
  size_t len = end_buffer - write_pointer;
  std::memcpy(output, write_pointer, len);
  return output + len;
}

// Escape tables and helpers (ported from ksonic)
struct quoted_char {
  int n;
  const char s[8];
};

static const struct quoted_char kQuoteTab[256] = {
    {6, "\\u0000"}, {6, "\\u0001"}, {6, "\\u0002"}, {6, "\\u0003"},
    {6, "\\u0004"}, {6, "\\u0005"}, {6, "\\u0006"}, {6, "\\u0007"},
    {2, "\\b"},     {2, "\\t"},     {2, "\\n"},     {6, "\\u000b"},
    {2, "\\f"},     {2, "\\r"},     {6, "\\u000e"}, {6, "\\u000f"},
    {6, "\\u0010"}, {6, "\\u0011"}, {6, "\\u0012"}, {6, "\\u0013"},
    {6, "\\u0014"}, {6, "\\u0015"}, {6, "\\u0016"}, {6, "\\u0017"},
    {6, "\\u0018"}, {6, "\\u0019"}, {6, "\\u001a"}, {6, "\\u001b"},
    {6, "\\u001c"}, {6, "\\u001d"}, {6, "\\u001e"}, {6, "\\u001f"},
    {0, ""}, {0, ""}, {2, "\\\""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""}, {0, ""},
    {0, ""}, {0, ""}, {0, ""}, {0, ""}, {2, "\\\\"}, {0, ""}, {0, ""}, {0, ""}
};

static const bool kNeedEscaped[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


} // anonymous namespace
namespace internal {

/***
 * Minifier/formatter code.
 **/

template <class formatter>
simdjson_inline void base_formatter<formatter>::number(uint64_t x) {
  char number_buffer[24];
  char *newp = fast_itoa(number_buffer, x);
  chars(number_buffer, newp);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::number(int64_t x) {
  char number_buffer[24];
  char *newp = fast_itoa(number_buffer, x);
  chars(number_buffer, newp);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::number(double x) {
#if SIMDJSON_ENABLE_NAN_INF
  if (simdjson_unlikely(!std::isfinite(x))) {
    if (std::isnan(x)) {
      char const *s = "NaN";
      chars(s, s + 3);
    } else {
      if (x < 0) {
        one_char('-');
      }
      char const *s = "Infinity";
      chars(s, s + 8);
    }
    return;
  }
#endif

  char number_buffer[24];
  // Currently, passing the nullptr to the second argument is
  // safe because our implementation does not check the second
  // argument.
  char *newp = internal::to_chars(number_buffer, nullptr, x);
  chars(number_buffer, newp);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::start_array() {
  one_char('[');
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::end_array() {
  one_char(']');
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::start_object() {
  one_char('{');
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::end_object() {
  one_char('}');
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::comma() {
  one_char(',');
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::true_atom() {
  const char *s = "true";
  chars(s, s + 4);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::false_atom() {
  const char *s = "false";
  chars(s, s + 5);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::null_atom() {
  const char *s = "null";
  chars(s, s + 4);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::one_char(char c) {
  buffer.push_back(c);
}

template <class formatter>
simdjson_inline void base_formatter<formatter>::chars(const char *begin,
                                                      const char *end) {
  buffer.append(begin, end);
}

template <class formatter>
simdjson_inline void
base_formatter<formatter>::key(std::string_view unescaped) {
  string(unescaped);
  one_char(':');
}

template <class formatter>
simdjson_inline void
base_formatter<formatter>::string(std::string_view unescaped) {
  one_char('\"');

  const char *src = unescaped.data();
  size_t nb = unescaped.length();

  if (nb == 0) {
    one_char('\"');
    return;
  }

  // 8-byte unrolled scan for first escape character
  size_t i = 0;
  for (; i + 8 <= nb; i += 8) {
    if (kNeedEscaped[uint8_t(src[i])] |
        kNeedEscaped[uint8_t(src[i + 1])] |
        kNeedEscaped[uint8_t(src[i + 2])] |
        kNeedEscaped[uint8_t(src[i + 3])] |
        kNeedEscaped[uint8_t(src[i + 4])] |
        kNeedEscaped[uint8_t(src[i + 5])] |
        kNeedEscaped[uint8_t(src[i + 6])] |
        kNeedEscaped[uint8_t(src[i + 7])]) {
      break;
    }
  }
  for (; i < nb; i++) {
    if (kNeedEscaped[uint8_t(src[i])]) {
      break;
    }
  }

  // Batch copy clean prefix
  chars(src, src + i);

  // Handle remaining: table-driven escape + batch copy clean segments
  while (i < nb) {
    // Escape current character using lookup table
    int nc = kQuoteTab[uint8_t(src[i])].n;
    chars(kQuoteTab[uint8_t(src[i])].s, kQuoteTab[uint8_t(src[i])].s + nc);
    i++;

    // Find and batch copy next clean segment
    size_t start = i;
    while (i < nb && !kNeedEscaped[uint8_t(src[i])]) {
      i++;
    }
    if (i > start) {
      chars(src + start, src + i);
    }
  }

  one_char('\"');
}

template <class formatter> inline void base_formatter<formatter>::clear() {
  buffer.clear();
}

template <class formatter>
simdjson_inline std::string_view base_formatter<formatter>::str() const {
  return buffer.str();
}

simdjson_inline void mini_formatter::print_newline() { return; }

simdjson_inline void mini_formatter::print_indents(size_t depth) {
  (void)depth;
  return;
}

simdjson_inline void mini_formatter::print_space() { return; }

simdjson_inline void pretty_formatter::print_newline() { one_char('\n'); }

simdjson_inline void pretty_formatter::print_indents(size_t depth) {
  if (this->indent_step <= 0) {
    return;
  }
  for (size_t i = 0; i < this->indent_step * depth; i++) {
    one_char(' ');
  }
}

simdjson_inline void pretty_formatter::print_space() { one_char(' '); }

/***
 * String building code.
 **/

template <class serializer>
inline void string_builder<serializer>::append(simdjson::dom::element value) {
  // using tape_type = simdjson::internal::tape_type;
  size_t depth = 0;
  constexpr size_t MAX_DEPTH = 16;
  bool is_object[MAX_DEPTH];
  is_object[0] = false;
  bool after_value = false;

  internal::tape_ref iter(value.tape);
  do {
    // print commas after each value
    if (after_value) {
      format.comma();
      format.print_newline();
    }

    format.print_indents(depth);

    // If we are in an object, print the next key and :, and skip to the next
    // value.
    if (is_object[depth]) {
      format.key(iter.get_string_view());
      format.print_space();
      iter.json_index++;
    }
    switch (iter.tape_ref_type()) {

    // Arrays
    case tape_type::START_ARRAY: {
      // If we're too deep, we need to recurse to go deeper.
      depth++;
      if (simdjson_unlikely(depth >= MAX_DEPTH)) {
        append(simdjson::dom::array(iter));
        iter.json_index = iter.matching_brace_index() - 1; // Jump to the ]
        depth--;
        break;
      }

      // Output start [
      format.start_array();
      iter.json_index++;

      // Handle empty [] (we don't want to come back around and print commas)
      if (iter.tape_ref_type() == tape_type::END_ARRAY) {
        format.end_array();
        depth--;
        break;
      }

      is_object[depth] = false;
      after_value = false;
      format.print_newline();
      continue;
    }

    // Objects
    case tape_type::START_OBJECT: {
      // If we're too deep, we need to recurse to go deeper.
      depth++;
      if (simdjson_unlikely(depth >= MAX_DEPTH)) {
        append(simdjson::dom::object(iter));
        iter.json_index = iter.matching_brace_index() - 1; // Jump to the }
        depth--;
        break;
      }

      // Output start {
      format.start_object();
      iter.json_index++;

      // Handle empty {} (we don't want to come back around and print commas)
      if (iter.tape_ref_type() == tape_type::END_OBJECT) {
        format.end_object();
        depth--;
        break;
      }

      is_object[depth] = true;
      after_value = false;
      format.print_newline();
      continue;
    }

    // Scalars
    case tape_type::STRING:
      format.string(iter.get_string_view());
      break;
    case tape_type::BIGINT: {
      // Big integer stored as string — output raw digits (no quotes)
      auto sv = iter.get_string_view();
      format.chars(sv.data(), sv.data() + sv.size());
      break;
    }
    case tape_type::INT64:
      format.number(iter.next_tape_value<int64_t>());
      iter.json_index++; // numbers take up 2 spots, so we need to increment
                         // extra
      break;
    case tape_type::UINT64:
      format.number(iter.next_tape_value<uint64_t>());
      iter.json_index++; // numbers take up 2 spots, so we need to increment
                         // extra
      break;
    case tape_type::DOUBLE:
      format.number(iter.next_tape_value<double>());
      iter.json_index++; // numbers take up 2 spots, so we need to increment
                         // extra
      break;
    case tape_type::TRUE_VALUE:
      format.true_atom();
      break;
    case tape_type::FALSE_VALUE:
      format.false_atom();
      break;
    case tape_type::NULL_VALUE:
      format.null_atom();
      break;

    // These are impossible
    case tape_type::END_ARRAY:
    case tape_type::END_OBJECT:
    case tape_type::ROOT:
      SIMDJSON_UNREACHABLE();
    }
    iter.json_index++;
    after_value = true;

    // Handle multiple ends in a row
    while (depth != 0 && (iter.tape_ref_type() == tape_type::END_ARRAY ||
                          iter.tape_ref_type() == tape_type::END_OBJECT)) {
      format.print_newline();
      depth--;
      format.print_indents(depth);
      if (iter.tape_ref_type() == tape_type::END_ARRAY) {
        format.end_array();
      } else {
        format.end_object();
      }
      iter.json_index++;
    }

    // Stop when we're at depth 0
  } while (depth != 0);

  format.print_newline();
}

template <class serializer>
inline void string_builder<serializer>::append(simdjson::dom::object value) {
  format.start_object();
  auto pair = value.begin();
  auto end = value.end();
  if (pair != end) {
    append(*pair);
    for (++pair; pair != end; ++pair) {
      format.comma();
      append(*pair);
    }
  }
  format.end_object();
}

template <class serializer>
inline void string_builder<serializer>::append(simdjson::dom::array value) {
  format.start_array();
  auto iter = value.begin();
  auto end = value.end();
  if (iter != end) {
    append(*iter);
    for (++iter; iter != end; ++iter) {
      format.comma();
      append(*iter);
    }
  }
  format.end_array();
}

template <class serializer>
simdjson_inline void
string_builder<serializer>::append(simdjson::dom::key_value_pair kv) {
  format.key(kv.key);
  append(kv.value);
}

template <class serializer>
simdjson_inline void string_builder<serializer>::clear() {
  format.clear();
}

template <class serializer>
simdjson_inline std::string_view string_builder<serializer>::str() const {
  return format.str();
}

} // namespace internal
} // namespace simdjson

#endif
