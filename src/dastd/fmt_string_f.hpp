/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
*
* @brief   This modules implements a set of formatters that can be used to print
*          on regular std::ostream streams.
*          Temporary instances of these objects take a source object and print
*          it using a given format.
*
* Compact version:
*
*     fmt_c     --> C11_ESCAPED
*     fmt_cq    --> C11_ESCAPED_QUOTED
*     fmt_js    --> JS_ESCAPED
*     fmt_utf8  --> UTF8
*
* For example:
*   std::cout << fmt_cq("Hello world");
*
**/
#pragma once
#include "defs.hpp"
namespace dastd {

using fmt_string_format_t = uint32_t;

/// @brief Set of available encodings of a n-bits characters string onto a 8-bit string
namespace fmt_string_f {
	/// @brief Indicates that hex values are to be printed in lower case (default is uppercase)
	static constexpr fmt_string_format_t LOWERCASE_HEX = 0x00000100;

	/// @brief Add double quotes on the side of the string
	static constexpr fmt_string_format_t WITHIN_DOUBLE_QUOTES = 0x00000200;

	/// @brief In SIMPLE_ESCAPED, it does not print double quotes but it prints their hex representation
	static constexpr fmt_string_format_t SIMPLE_ESC_DOUBLE_QUOTES_AS_HEX = 0x00000400;

	/// @brief When printing C strings with autorequote, after a new line it will close the quote and reopen it on the following line
	static constexpr fmt_string_format_t NEWLINES_ON_NEW_LINES = 0x00000800;

	/// @brief Main settings
	static constexpr fmt_string_format_t UTF8                            = 1; ///< Writes the binary UTF-8 encoding
	static constexpr fmt_string_format_t C11_ESCAPED                     = 2; ///< Writes a C++11 string with escapes (\x, \u and \U, plus the usual \0, \r, \n, etc.)
	static constexpr fmt_string_format_t C11_ESCAPED_AUTOREQUOTE         = 3; ///< Same as C11_ESCAPED but it adds a close/reopen double quotes pair in case of ambiguous sequence
	static constexpr fmt_string_format_t JS_ESCAPED                      = 4; ///< Javascript escape (escapes quotes, apostrophes, etc.)
	static constexpr fmt_string_format_t SIMPLE_ESCAPED                  = 5; ///< Uses a single custom escape character set with `esc`

	static constexpr fmt_string_format_t C11_ESCAPED_QUOTED              = C11_ESCAPED|WITHIN_DOUBLE_QUOTES; ///< Writes a C++11 string with escapes (\x, \u and \U, plus the usual \0, \r, \n, etc.) surrounded by double quotes
	static constexpr fmt_string_format_t C11_ESCAPED_AUTOREQUOTE_QUOTED  = C11_ESCAPED_AUTOREQUOTE|WITHIN_DOUBLE_QUOTES; ///< Same as C11_ESCAPED but it adds a close/reopen double quotes pair in case of ambiguous sequence surrounded by double quotes
	static constexpr fmt_string_format_t C11_ESCAPED_AUTOREQUOTE_QUOTED_NL  = C11_ESCAPED_AUTOREQUOTE|WITHIN_DOUBLE_QUOTES|NEWLINES_ON_NEW_LINES; ///< Same as C11_ESCAPED but it adds a close/reopen double quotes pair in case of ambiguous sequence surrounded by double quotes
	static constexpr fmt_string_format_t SIMPLE_ESCAPED_QUOTED           = SIMPLE_ESCAPED|WITHIN_DOUBLE_QUOTES|SIMPLE_ESC_DOUBLE_QUOTES_AS_HEX; ///< Same as C11_ESCAPED but it adds a close/reopen double quotes pair in case of ambiguous sequence surrounded by double quotes

	static constexpr fmt_string_format_t FMT_DEFAULT = C11_ESCAPED;

	/// @brief Return the main mode
	inline consteval fmt_string_format_t main_mode(fmt_string_format_t fmt) {return (fmt & 0x0F);}

	/// @brief Return true if the given flag is set
	inline consteval uint32_t has_flag(fmt_string_format_t fmt, uint32_t flag) {return DASTD_ISSET(fmt, flag);}

	/// @brief Return the format to set the escape character in `SIMPLE_ESCAPED`
	///
	/// Example:
	///
	///   std::cout << fmt_string<char, fmt_string_f::SIMPLE_ESCAPED | fmt_string_f::esc('#')>(myString);
	inline consteval fmt_string_format_t esc(char esc_ch) {return ((fmt_string_format_t)((uint8_t)esc_ch)) << 24;}

	/// @brief Return the format to set the escape character in `SIMPLE_ESCAPED`
	///
	/// Example:
	///
	///   std::cout << fmt_string<char, fmt_string_f::SIMPLE_ESCAPED | fmt_string_f::esc('#')>(myString);
	inline consteval char get_esc(fmt_string_format_t format) {return ((format >> 24) == 0 ? '#' : ((char)(format >> 24)));}
};
} // namespace dastd
