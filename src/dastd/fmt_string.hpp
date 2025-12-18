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
#include "fmt.hpp"
#include "flooder_ch32.hpp"
#include "char32string.hpp"
#include "fmt_string_f.hpp"
#include <filesystem>
namespace dastd {

/// @brief Print a string on a 8-bit std::ostream applying some encoding
/// @tparam CHARTYPE Type of the characters being formatted
template<class CHARTYPE=char32_t, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
class fmt_string {
	public:
		/// @brief Constructor
		/// @param ptrz    Zero-terminated string
		fmt_string(const CHARTYPE* ptrz):
			m_data_type(use_ptrz) {m_data.m_ptr = ptrz;}

		/// @brief Constructor
		/// @param ptr    Input string
		/// @param length  Length of the input string
		fmt_string(const CHARTYPE* ptr, size_t length):
			m_data_type(use_ptr_len), m_length(length) {m_data.m_ptr = ptr;}

		/// @brief Constructor that writes a string
		/// @param string    Input string
		fmt_string(const std::basic_string_view<CHARTYPE>& string):
			m_data_type(use_string), m_string(string) {}

		/// @brief Constructor that writes a string
		/// @param string    Input string
		fmt_string(const std::basic_string<CHARTYPE>& string):
			m_data_type(use_string), m_string(string) {}

		/// @brief Constructor that writes a string
		/// @param src     Char32source object
		fmt_string(const flooder_ch32& src):
			m_data_type(use_char32source) {m_data.m_char32source = &src;}

		/// @brief Constructor that writes a char32string
		/// @param string     Char32source object
		template<typename U = CHARTYPE, typename = typename std::enable_if< std::is_same<U, char32_t>::value>::type>
		fmt_string(const char32string& string):
			m_data_type(use_string), m_string(string) {}

		/// @brief Constructor that writes a single character
		/// @param ch      Single character
		fmt_string(char32_t ch):
			m_data_type(use_char32) {m_data.m_ch32 = ch;}

		/// @brief Print the contents
		/// @param o Output stream
		void print(std::ostream& o) const;

	private:
		/// @brief Indicates what data has been set internally
		enum data_type {
			use_none, use_string, use_ptrz, use_ptr_len, use_char32source, use_char32
		};

		/// @brief Type of data enabled
		data_type m_data_type;

		/// @brief Text string (m_data_type == use_string)
		const std::basic_string<CHARTYPE> m_string;

		/// @brief Length of static const data (m_data_type == use_ptr_len)
		size_t m_length = 0;

		union {
			/// @brief Pointer to static const data (m_data_type == use_ptrz or use_ptr_len)
			const CHARTYPE* m_ptr;

			/// @brief Pointer to a flooder_ch32 (m_data_type == use_char32source)
			const flooder_ch32* m_char32source;

			/// @brief Single char32 (m_data_type == use_char32)
			char32_t m_ch32;

		} m_data;
};

/// @brief Print a path in reasonable readable way
class fmt_path: public fmt_string<char, fmt_string_f::SIMPLE_ESCAPED_QUOTED> {
	public:
		fmt_path(const std::filesystem::path& path): fmt_string<char, fmt_string_f::SIMPLE_ESCAPED_QUOTED>(path.string()) {}
};

#if 0
// This automatic version causes the 11.3.1 and 11.4.0 g++ compiler to crash with "internal compiler error: Segmentation fault"
// Therefore we shall use fmt_xx or fmt32_xx according to the type.
template<class CHARTYPE> using fmt_c = fmt_string<CHARTYPE, fmt_string_f::C11_ESCAPED>;
template<class CHARTYPE> using fmt_cq = fmt_string<CHARTYPE, fmt_string_f::C11_ESCAPED_QUOTED>;
template<class CHARTYPE> using fmt_js = fmt_string<CHARTYPE, fmt_string_f::JS_ESCAPED>;
template<class CHARTYPE> using fmt_utf8 = fmt_string<CHARTYPE, fmt_string_f::UTF8>;
#else
using fmt_c = fmt_string<char, fmt_string_f::C11_ESCAPED>;
using fmt_cq = fmt_string<char, fmt_string_f::C11_ESCAPED_QUOTED>;
using fmt_js = fmt_string<char, fmt_string_f::JS_ESCAPED>;
using fmt_utf8 = fmt_string<char, fmt_string_f::UTF8>;
using fmt_ca = fmt_string<char, fmt_string_f::C11_ESCAPED_AUTOREQUOTE>;
using fmt_caq = fmt_string<char, fmt_string_f::C11_ESCAPED_AUTOREQUOTE_QUOTED>;
using fmt_s = fmt_string<char, fmt_string_f::SIMPLE_ESCAPED>;
using fmt_sq = fmt_string<char, fmt_string_f::SIMPLE_ESCAPED_QUOTED>;
#endif

using fmt16_c = fmt_string<char16_t, fmt_string_f::C11_ESCAPED>;
using fmt16_cq = fmt_string<char16_t, fmt_string_f::C11_ESCAPED_QUOTED>;
using fmt16_js = fmt_string<char16_t, fmt_string_f::JS_ESCAPED>;
using fmt16_utf8 = fmt_string<char16_t, fmt_string_f::UTF8>;
using fmt16_ca = fmt_string<char16_t, fmt_string_f::C11_ESCAPED_AUTOREQUOTE>;
using fmt16_caq = fmt_string<char16_t, fmt_string_f::C11_ESCAPED_AUTOREQUOTE_QUOTED>;
using fmt16_s = fmt_string<char16_t, fmt_string_f::SIMPLE_ESCAPED>;
using fmt16_sq = fmt_string<char16_t, fmt_string_f::SIMPLE_ESCAPED_QUOTED>;


using fmt32_c = fmt_string<char32_t, fmt_string_f::C11_ESCAPED>;
using fmt32_cq = fmt_string<char32_t, fmt_string_f::C11_ESCAPED_QUOTED>;
using fmt32_js = fmt_string<char32_t, fmt_string_f::JS_ESCAPED>;
using fmt32_utf8 = fmt_string<char32_t, fmt_string_f::UTF8>;
using fmt32_ca = fmt_string<char32_t, fmt_string_f::C11_ESCAPED_AUTOREQUOTE>;
using fmt32_caq = fmt_string<char32_t, fmt_string_f::C11_ESCAPED_AUTOREQUOTE_QUOTED>;
using fmt32_s = fmt_string<char32_t, fmt_string_f::SIMPLE_ESCAPED>;
using fmt32_sq = fmt_string<char32_t, fmt_string_f::SIMPLE_ESCAPED_QUOTED>;

template<class CHARTYPE,fmt_string_format_t FORMAT> std::ostream& operator<< (std::ostream& o, const fmt_string<CHARTYPE,FORMAT>& n) {n.print(o); return o;}
template<class CHARTYPE,fmt_string_format_t FORMAT> sink_ch32& operator<<(sink_ch32& sink, const fmt_string<CHARTYPE,FORMAT>& s) {fmt32utf8<const fmt_string<CHARTYPE,FORMAT>&> fm(s); fm.write_to_sink(sink); return sink;}

// Encode the char32_t in UTF-8 on the given stream
template<class CHARTYPE>
void print_UTF8(std::ostream& o, CHARTYPE code_point)
{
	char buf[UTF8_CHAR_MAX_LEN];
	size_t m_len = write_utf8_asciiz(buf, code_point);
	o.write(buf, m_len);
}

// Encode the char32string in UTF-8 on the given stream
template<class CHARTYPE>
void print_UTF8(std::ostream& o, const std::basic_string_view<CHARTYPE>& text)
{
	for (auto code_point: text) print_UTF8(o, code_point);
}

/// @brief Writes on the indicated ostream the `code_point1` closing and reopening the double
///        quotes if `code_point2` creates an ambiguity.
///
/// It will close and reopen the double quotes
/// if code_point1 followed by code_point2 might cause an ambiguity.
/// This can be used to avoid "\xABC" to be misinterpreted as 0xABC
/// and not "\xAB" followed by "C". In case of "\xAB" it will write
/// '" "' so it becomes ...\xAB" "C...
///
/// @tparam CHARTYPE    Character type (char, char32_t, etc)
/// @param o            Output string
/// @param code_point1   Input character
/// @param code_point2   If set to other than zero, it expects to contain the character
///                     following 'code_point1'.
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_C11_ESCAPED_AUTOREQUOTE(std::ostream& o, CHARTYPE code_point1, CHARTYPE code_point2)
{
	static_assert(std::is_integral<CHARTYPE>::value, "CHARTYPE must be integral, like 'char', 'char32_t', etc.)");
	std::make_unsigned_t<CHARTYPE> cp1 = (std::make_unsigned_t<CHARTYPE>)code_point1;
	std::make_unsigned_t<CHARTYPE> cp2 = (std::make_unsigned_t<CHARTYPE>)code_point2;

	bool lower_case_hex = fmt_string_f::has_flag(FORMAT, fmt_string_f::LOWERCASE_HEX);

	switch(cp1) {
		case '\\': o << "\\\\"; break;
		case '\0': o << "\\0"; break;
		case '\r': o << "\\r"; break;
		case '\n': o << "\\n"; break;
		case '\t': o << "\\t"; break;
		case '"' : o << "\\\""; break;
		default: {
			if ((cp1 >= ' ') && (cp1 < 127)) {
				o << (char)cp1;
			}
			else {
				if (cp1 <= 0xFF) {
					o << "\\x" << fmt(cp1, 16, 2, lower_case_hex);
				}
				else if (cp1 <= 0xFFFF) {
					o << "\\u" << fmt(cp1, 16, 4, lower_case_hex);
				}
				else {
					o << "\\U" << fmt(cp1, 16, 8, lower_case_hex);
				}
				if (cp2 && ((cp2 >= '0' && cp2 <= '9') || (cp2 >= 'A' && cp2 <= 'F') || (cp2 >= 'a' && cp2 <= 'f'))) {
					o << "\" \"";
				}
			}
		}
	}
}

/// @brief Writes on the indicated ostream the specified code_point
///
/// @tparam CHARTYPE    Character type (char, char32_t, etc)
/// @param o            Output string
/// @param code_point1   Input character
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_C11_ESCAPED(std::ostream& o, CHARTYPE code_point1) {print_C11_ESCAPED_AUTOREQUOTE<CHARTYPE,FORMAT>(o, code_point1, (CHARTYPE)0);}

/// @brief Writes on the indicated ostream the character according
///
/// @tparam CHARTYPE    Character type (char, char32_t, etc)
/// @param o            Output string
/// @param text         Input string
/// @param auto_requote  If true, it closes and reopens the double quotes
///                     after an hex value. This can be used to avoid
///                     "\xABC" to be misinterpreted as 0xABC and not "\xAB" followed
///                     by "C". In case of "\xAB" it will write '" "' so it becomes
///                     ...\xAB" "C...
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_C11_ESCAPED(std::ostream& o, const std::basic_string_view<CHARTYPE>& text, bool auto_requote=false)
{
	if (auto_requote) {
		bool first=true;
		CHARTYPE prev = 0;
		for (CHARTYPE code_point: text) {
			if (first) {first=false;}
			else print_C11_ESCAPED_AUTOREQUOTE<CHARTYPE,FORMAT>(o, prev, code_point);
			prev=code_point;
		}
		if (!first) print_C11_ESCAPED<CHARTYPE,FORMAT>(o, prev);
	}
	else {
		for (auto code_point: text) print_C11_ESCAPED<CHARTYPE,FORMAT>(o, code_point);
	}
}

/// @brief JavaScript aggressive escaping
///
/// @tparam CHARTYPE    Character type (char, char32_t, etc)
/// @param o            Output string
/// @param code_point1   Input character
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_JS_ESCAPED(std::ostream& o, CHARTYPE code_point1)
{
	static_assert(std::is_integral<CHARTYPE>::value, "CHARTYPE must be integral, like 'char', 'char32_t', etc.)");
	std::make_unsigned_t<CHARTYPE> cp1 = (std::make_unsigned_t<CHARTYPE>)code_point1;

	switch(cp1) {
		case '\\': o << "\\\\"; break;
		case '\0': o << "\\0"; break;
		case '\r': o << "\\r"; break;
		case '\n': o << "\\n"; break;
		case '\t': o << "\\t"; break;
		default: {
			if ((cp1 >= ' ') && (cp1 < 127) && (cp1 != '"') && (cp1 != '\'') && (cp1 != '>') && (cp1 != '<')) {
				o << (char)cp1;
			}
			else {
				bool lower_case_hex = fmt_string_f::has_flag(FORMAT, fmt_string_f::LOWERCASE_HEX);
				if (cp1 <= 0xFF) {
					o << "\\x" << fmt(cp1, 16, 2, lower_case_hex);
				}
				else if (cp1 <= 0xFFFF) {
					o << "\\u" << fmt(cp1, 16, 4, lower_case_hex);
				}
				else {
					o << "\\U" << fmt(cp1, 16, 8, lower_case_hex);
				}
			}
		}
	}
}

/// @brief Simple escaping that uses a custom escape character
///
/// @tparam CHARTYPE    Character type (char, char32_t, etc)
/// @param o            Output string
/// @param code_point1  Input character
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_SIMPLE_ESCAPED(std::ostream& o, CHARTYPE code_point1)
{
	static_assert(std::is_integral<CHARTYPE>::value, "CHARTYPE must be integral, like 'char', 'char32_t', etc.)");
	std::make_unsigned_t<CHARTYPE> cp1 = (std::make_unsigned_t<CHARTYPE>)code_point1;
	constexpr char escapeChar = fmt_string_f::get_esc(FORMAT);

	switch(cp1) {
		case escapeChar: o << escapeChar << escapeChar; break;
		default: {
			bool forceHex = false;
			if constexpr (DASTD_ISSET(FORMAT, fmt_string_f::SIMPLE_ESC_DOUBLE_QUOTES_AS_HEX)) {
				if (cp1 == '"') forceHex=true;
			}
			if ((cp1 >= ' ') && (cp1 < 127) && !forceHex) {
				o << (char)cp1;
			}
			else {
				bool lower_case_hex = fmt_string_f::has_flag(FORMAT, fmt_string_f::LOWERCASE_HEX);
				if (cp1 <= 0xFF) {
					o << escapeChar << "x" << fmt(cp1, 16, 2, lower_case_hex);
				}
				else if (cp1 <= 0xFFFF) {
					o << escapeChar << "u" << fmt(cp1, 16, 4, lower_case_hex);
				}
				else {
					o << escapeChar << "U" << fmt(cp1, 16, 8, lower_case_hex);
				}
			}
		}
	}
}

// Encode the char32_t in the indicated fmt_string_settings settings
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_8bit_format(std::ostream& o, CHARTYPE code_point)
{
	switch(fmt_string_f::main_mode(FORMAT)) {
		case fmt_string_f::UTF8: print_UTF8(o, code_point); break;
		case fmt_string_f::C11_ESCAPED:
		case fmt_string_f::C11_ESCAPED_AUTOREQUOTE: print_C11_ESCAPED<CHARTYPE,FORMAT>(o, code_point); break;
		case fmt_string_f::JS_ESCAPED: print_JS_ESCAPED<CHARTYPE,FORMAT>(o, code_point); break;
		case fmt_string_f::SIMPLE_ESCAPED: print_SIMPLE_ESCAPED<CHARTYPE,FORMAT>(o, code_point); break;
		default:;
	}
}

// Encode the char32_t in the indicated fmt_string_settings settings
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_8bit_format(std::ostream& o, const CHARTYPE* string, size_t length)
{
	size_t i;
	switch(fmt_string_f::main_mode(FORMAT)) {
		case fmt_string_f::UTF8       : for (i=0; i<length; i++) print_UTF8(o, string[i]); break;
		case fmt_string_f::C11_ESCAPED: for (i=0; i<length; i++) print_C11_ESCAPED<CHARTYPE,FORMAT>(o, string[i]); break;
		case fmt_string_f::C11_ESCAPED_AUTOREQUOTE: for (i=0; i<length; i++) print_C11_ESCAPED_AUTOREQUOTE<CHARTYPE,FORMAT>(o, string[i], (i+1 < length ? string[i+1] : (CHARTYPE)0)); break;
		case fmt_string_f::JS_ESCAPED: for (i=0; i<length; i++) print_JS_ESCAPED<CHARTYPE,FORMAT>(o, string[i]); break;
		case fmt_string_f::SIMPLE_ESCAPED: for (i=0; i<length; i++) print_SIMPLE_ESCAPED<CHARTYPE,FORMAT>(o, string[i]); break;
		default:;
	}
}

// Encode the char32_t in the indicated fmt_string_settings settings
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_8bit_format(std::ostream& o, const CHARTYPE* stringZ)
{
	size_t i;
	switch(fmt_string_f::main_mode(FORMAT)) {
		case fmt_string_f::UTF8       : for (i=0; stringZ[i]; i++) print_UTF8(o, stringZ[i]); break;
		case fmt_string_f::C11_ESCAPED: for (i=0; stringZ[i]; i++) print_C11_ESCAPED<CHARTYPE,FORMAT>(o, stringZ[i]); break;
		case fmt_string_f::C11_ESCAPED_AUTOREQUOTE: for (i=0; stringZ[i]; i++) print_C11_ESCAPED_AUTOREQUOTE<CHARTYPE,FORMAT>(o, stringZ[i], stringZ[i+1]); break;
		case fmt_string_f::JS_ESCAPED: for (i=0; stringZ[i]; i++) print_JS_ESCAPED<CHARTYPE,FORMAT>(o, stringZ[i]); break;
		case fmt_string_f::SIMPLE_ESCAPED: for (i=0; stringZ[i]; i++) print_SIMPLE_ESCAPED<CHARTYPE,FORMAT>(o, stringZ[i]); break;
		default:;
	}
}

// Encode the char32_t in the indicated fmt_string_settings settings
template<class CHARTYPE, fmt_string_format_t FORMAT=fmt_string_f::FMT_DEFAULT>
void print_8bit_format(std::ostream& o, const std::basic_string_view<CHARTYPE>& text)
{
	switch(fmt_string_f::main_mode(FORMAT)) {
		case fmt_string_f::UTF8: for (auto code_point: text) print_UTF8(o, code_point); break;
		case fmt_string_f::C11_ESCAPED: print_C11_ESCAPED<CHARTYPE,FORMAT>(o, text, false); break;
		case fmt_string_f::C11_ESCAPED_AUTOREQUOTE: print_C11_ESCAPED<CHARTYPE,FORMAT>(o, text, true); break;
		case fmt_string_f::JS_ESCAPED: for (auto code_point: text) print_JS_ESCAPED<CHARTYPE,FORMAT>(o, code_point); break;
		case fmt_string_f::SIMPLE_ESCAPED: for (auto code_point: text) print_SIMPLE_ESCAPED<CHARTYPE,FORMAT>(o, code_point); break;
		default:;
	}
}

// Print the contents
template<class CHARTYPE, fmt_string_format_t FORMAT>
void fmt_string<CHARTYPE,FORMAT>::print(std::ostream& o) const
{
	bool add_double_quotes = fmt_string_f::has_flag(FORMAT, fmt_string_f::WITHIN_DOUBLE_QUOTES);
	if (add_double_quotes) o << "\"";
	switch(m_data_type) {
		case use_none: break;
		case use_string: print_8bit_format<CHARTYPE,FORMAT>(o, m_string); break;
		case use_ptrz: if (m_data.m_ptr) print_8bit_format<CHARTYPE,FORMAT>(o, m_data.m_ptr); break;
		case use_ptr_len: if (m_data.m_ptr) print_8bit_format<CHARTYPE,FORMAT>(o, m_data.m_ptr, m_length); break;
		case use_char32source: {
			char32string local_string;
			if (m_data.m_char32source) m_data.m_char32source->write_to_sink(local_string);
			print_8bit_format<char32_t,FORMAT>(o, local_string.c_str(), local_string.length());
			break;
		}
		case use_char32: print_8bit_format<char32_t,FORMAT>(o, m_data.m_ch32); break;
	}
	if (add_double_quotes) o << "\"";
}


} // namespace dastd

inline std::ostream& operator<<(std::ostream& o, const char32_t* data) {dastd::print_8bit_format(o, data, dastd::fmt_string_f::C11_ESCAPED); return o;}
inline std::ostream& operator<<(std::ostream& o, char32_t data) {dastd::print_C11_ESCAPED<char32_t>(o, data); return o;}
inline std::ostream& operator<<(std::ostream& o, const std::u32string_view& data) {dastd::print_C11_ESCAPED(o, data); return o;}
//inline std::ostream& operator<<(std::ostream& o, const dastd::flooder_ch32& data) {o << dastd::fmt_string(data); return o;}
