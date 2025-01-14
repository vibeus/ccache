// Copyright (C) 2021-2023 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#pragma once

#include <util/Bytes.hpp>
#include <util/conversion.hpp>

#include <third_party/nonstd/expected.hpp>
#include <third_party/nonstd/span.hpp>

#include <sys/stat.h> // for mode_t

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace util {

// --- Interface ---

enum class SizeUnitPrefixType { binary, decimal };

// Return true if `suffix` is a suffix of `string`.
bool ends_with(std::string_view string, std::string_view suffix);

// Format a hexadecimal string representing `data`. The returned string will be
// `2 * data.size()` long.
std::string format_base16(nonstd::span<const uint8_t> data);

// Format a lowercase base32hex string representing `data`. No padding
// characters will be added.
std::string format_base32hex(nonstd::span<const uint8_t> data);

// Format a hash digest representing `data`.
//
// The first two bytes are encoded as four lowercase base16 digits to maintain
// compatibility with the cleanup algorithm in older ccache versions and to
// allow for up to four uniform cache levels. The rest are encoded as lowercase
// base32hex digits without padding characters.
std::string format_digest(nonstd::span<const uint8_t> data);

// Format `diff` as a human-readable string.
std::string format_human_readable_diff(int64_t diff,
                                       SizeUnitPrefixType prefix_type);

// Format `size` as a human-readable string.
std::string format_human_readable_size(uint64_t size,
                                       SizeUnitPrefixType prefix_type);

// Join stringified elements of `container` delimited by `delimiter` into a
// string. There must exist an `std::string to_string(T::value_type)` function.
template<typename T>
std::string join(const T& container, const std::string_view delimiter);

// Join stringified elements between input iterators `begin` and `end` delimited
// by `delimiter` into a string. There must exist an `std::string
// to_string(T::value_type)` function.
template<typename T>
std::string
join(const T& begin, const T& end, const std::string_view delimiter);

// Parse a string into a double.
//
// Returns an error string if `value` cannot be parsed as a double.
nonstd::expected<double, std::string> parse_double(const std::string& value);

// Parse `duration`, an unsigned integer with d (days) or s (seconds) suffix,
// into seconds.
nonstd::expected<uint64_t, std::string>
parse_duration(std::string_view duration);

// Parse a string into a signed integer.
//
// Returns an error string if `value` cannot be parsed as an int64_t or if the
// value falls out of the range [`min_value`, `max_value`]. `min_value` and
// `max_value` default to min and max values of int64_t. `description` is
// included in the error message for range violations.
nonstd::expected<int64_t, std::string>
parse_signed(std::string_view value,
             std::optional<int64_t> min_value = std::nullopt,
             std::optional<int64_t> max_value = std::nullopt,
             std::string_view description = "integer");

// Parse a "size value", i.e. a string that can end in k, M, G, T (10-based
// suffixes) or Ki, Mi, Gi, Ti (2-based suffixes). For backward compatibility, K
// is also recognized as a synonym of k.
nonstd::expected<std::pair<uint64_t, util::SizeUnitPrefixType>, std::string>
parse_size(const std::string& value);

// Parse `value` (an octal integer).
nonstd::expected<mode_t, std::string> parse_umask(std::string_view value);

// Parse a string into an unsigned integer.
//
// Returns an error string if `value` cannot be parsed as an uint64_t with base
// `base`, or if the value falls out of the range [`min_value`, `max_value`].
// `min_value` and `max_value` default to min and max values of uint64_t.
// `description` is included in the error message for range violations.
nonstd::expected<uint64_t, std::string>
parse_unsigned(std::string_view value,
               std::optional<uint64_t> min_value = std::nullopt,
               std::optional<uint64_t> max_value = std::nullopt,
               std::string_view description = "integer",
               int base = 10);

// Percent-decode[1] `string`.
//
// [1]: https://en.wikipedia.org/wiki/Percent-encoding
nonstd::expected<std::string, std::string>
percent_decode(std::string_view string);

// Replace the all occurrences of `from` to `to` in `string`.
std::string replace_all(std::string_view string,
                        std::string_view from,
                        std::string_view to);

// Replace the first occurrence of `from` to `to` in `string`.
std::string replace_first(std::string_view string,
                          std::string_view from,
                          std::string_view to);

// Split `string` into two parts using `split_char` as the delimiter. The second
// part will be `nullopt` if there is no `split_char` in `string.`
std::pair<std::string_view, std::optional<std::string_view>>
split_once(const char* string, char split_char);
std::pair<std::string, std::optional<std::string>>
split_once(std::string&& string, char split_char);
std::pair<std::string_view, std::optional<std::string_view>>
split_once(std::string_view string, char split_char);

// Return true if `prefix` is a prefix of `string`.
bool starts_with(const char* string, std::string_view prefix);

// Return true if `prefix` is a prefix of `string`.
bool starts_with(std::string_view string, std::string_view prefix);

// Strip whitespace from left and right side of a string.
[[nodiscard]] std::string strip_whitespace(std::string_view string);

// --- Inline implementations ---

inline bool
ends_with(const std::string_view string, const std::string_view suffix)
{
  return string.length() >= suffix.length()
         && string.substr(string.length() - suffix.length()) == suffix;
}

template<typename T>
inline std::string
join(const T& container, const std::string_view delimiter)
{
  return join(container.begin(), container.end(), delimiter);
}

template<typename T>
inline std::string
join(const T& begin, const T& end, const std::string_view delimiter)
{
  std::string result;
  for (auto it = begin; it != end; ++it) {
    if (it != begin) {
      result.append(delimiter.data(), delimiter.length());
    }
    result += to_string(*it);
  }
  return result;
}

inline bool
starts_with(const char* const string, const std::string_view prefix)
{
  // Optimized version of starts_with(string_view, string_view): avoid computing
  // the length of the string argument.
  return std::strncmp(string, prefix.data(), prefix.length()) == 0;
}

inline bool
starts_with(const std::string_view string, const std::string_view prefix)
{
  return string.substr(0, prefix.size()) == prefix;
}

} // namespace util
