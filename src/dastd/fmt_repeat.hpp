/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 21-MAR-2023
*
* @brief Implements a formatter that repeats the same string a given number of times
*
**/
#pragma once
#include "defs.hpp"
#include "sink_ch32_ostream.hpp"
#include <ostream>
namespace dastd {

/// @brief Formatter that repeats the same string a given number of times
class fmt_repeat {
	private:
		/// @brief Text repeated over
		std::u32string m_text;

		/// @brief Number of times the text is to be repeated
		size_t m_repetitions;

	public:
		/// @brief Constructor
		fmt_repeat(const char* text, size_t repetitions): m_repetitions(repetitions) {write_utf8_to_u32string(text, strlen(text), m_text);}

		/// @brief Constructor
		fmt_repeat(const std::string& text, size_t repetitions): m_repetitions(repetitions) {write_utf8_to_u32string(text.c_str(), text.size(), m_text);}

		/// @brief Constructor
		fmt_repeat(const char32_t* text, size_t repetitions): m_text(text), m_repetitions(repetitions) {}

		/// @brief Constructor
		fmt_repeat(const std::u32string& text, size_t repetitions): m_text(text), m_repetitions(repetitions) {}

		// Print the contents (UTF-8)
		void print(std::ostream& o) const {sink_ch32_ostream<fmt_string_f::UTF8> out(o); print(out);}

		// Print the contents (UTF-32)
		void print(sink_ch32& o) const {size_t i; for(i=0; i<m_repetitions; i++) o << m_text;}
};

inline std::ostream& operator<< (std::ostream& o, const fmt_repeat& n) {n.print(o); return o;}
inline sink_ch32& operator<<(sink_ch32& sink, const fmt_repeat& n) {n.print(sink); return sink;}


} // namespace dastd
