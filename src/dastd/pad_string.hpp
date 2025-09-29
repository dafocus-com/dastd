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
**/
#pragma once
#include "defs.hpp"
#include "fmt32.hpp"
#include "sink_ch32_ostream.hpp"
#include <string>
#include <sstream>
namespace dastd {


namespace pad_string_align {
	constexpr uint32_t LEFT   = 0x00;
	constexpr uint32_t CENTER = 0x01;
	constexpr uint32_t RIGHT  = 0x02;

	/// @brief If set, the string is truncated if too long
	constexpr uint32_t TRUNCATE  = 0x04;
};

/*==============================================================================
	Prints a string with padding
==============================================================================*/
class pad_string {
	private:
		/// @brief String to be printed
		std::u32string m_text;

		/// @brief Padding mode
		size_t m_width;

		/// @brief Required padding
		uint32_t m_align;

		/// @brief Required padding
		char32_t m_padding_char = ' ';

	public:
		/// @brief Constructor
		pad_string(const char* text, size_t width, uint32_t align=pad_string_align::LEFT, char32_t padding_char=' '): m_width(width), m_align(align), m_padding_char(padding_char) {write_utf8_to_u32string(text, strlen(text), m_text);}

		/// @brief Constructor
		pad_string(const std::string_view& text, size_t width, uint32_t align=pad_string_align::LEFT, char32_t padding_char=' '): m_width(width), m_align(align), m_padding_char(padding_char) {write_utf8_to_u32string(text.data(), text.size(), m_text);}

		/// @brief Constructor
		pad_string(const char32_t* text, size_t width, uint32_t align=pad_string_align::LEFT, char32_t padding_char=' '): m_text(text), m_width(width), m_align(align), m_padding_char(padding_char) {}

		/// @brief Constructor
		pad_string(const std::u32string_view& text, size_t width, uint32_t align=pad_string_align::LEFT, char32_t padding_char=' '): m_text(text), m_width(width), m_align(align), m_padding_char(padding_char) {}

		// Print the contents (UTF-8)
		void print(std::ostream& o) const {sink_ch32_ostream<fmt_string_f::UTF8>out(o); print(out);}

		// Print the contents (UTF-32)
		void print(sink_ch32& o) const;

		// Return as string
		std::string str() const {std::ostringstream ret; print(ret); return ret.str();}

};

inline std::ostream& operator<< (std::ostream& o, const pad_string& n) {n.print(o); return o;}
inline sink_ch32& operator<<(sink_ch32& sink, const pad_string& n) {n.print(sink); return sink;}

// Print the contents
inline void pad_string::print(sink_ch32& o) const
{
	if (m_text.length() >= m_width) {
		if (DASTD_ISSET(m_align, pad_string_align::TRUNCATE)) o.sink_write(m_text.c_str(), m_width);
		else o << m_text;
		return;
	}

	size_t padding = m_width - m_text.length();
	size_t padding_left=0, padding_right=0;
	switch(m_align & 0x03) {
		case pad_string_align::LEFT: padding_right=padding; break;
		case pad_string_align::RIGHT: padding_left=padding; break;
		case pad_string_align::CENTER: padding_left=padding/2; padding_right=padding-padding_left; break;
	}
	unsigned i;
	for (i=0; i<padding_left; i++) o << m_padding_char;
	o << m_text;
	for (i=0; i<padding_right; i++) o << m_padding_char;
}

} // namespace dastd
