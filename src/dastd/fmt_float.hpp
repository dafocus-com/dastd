/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 31-OCT-2024
*
* @brief   This module implements a formatter that can be used to print
*          on regular std::ostream streams a floating point value.
*          Temporary instances of these objects take a source object and print
*          it using a given format.
*
**/
#pragma once
#include "defs.hpp"
#include "fmt32.hpp"
#include "string_tools.hpp"
#include <ostream>
#include <sstream>
#include <limits>
#include <charconv>

namespace dastd {

class char32string;
class flooder_ch32;

/// @brief Formats for fmt_float
enum class fmt_float_f {
	FIXED,       ///< Same as printf %f
	SCIENTIFIC,  ///< Same as printf %a
	GENERAL      ///< Same as printf %g
};

/// @brief Helper class that allows formatting a number while printing on a stream
///
/// The `fmt_float` helper class can be embedded in a stream << sequence:
///
///     o << "Value 0x" << fmt_float(number, 16, 4) << std::endl;
///
template<class NUMTYPE, fmt_float_f FORMAT>
class fmt_float {
	static_assert(std::is_floating_point<NUMTYPE>::value, "Floating point required");
	static constexpr size_t k_buf_size = 66;
	public:
		static constexpr uint32_t k_SHORTEST = std::numeric_limits<uint32_t>::max();

		/// @brief Constructor
		/// @param value Value to be formatted
		/// @param pad_width Padding width; the formatter will add spaces on the left to reach the this value
		/// @param precision Number of decimals to be used; specify `k_SHORTEST` to have the minimum number of decimals
		fmt_float(NUMTYPE value, uint32_t pad_width=0, uint32_t precision = k_SHORTEST):
			m_value(value), m_pad_width(pad_width), m_precision(precision)
			{}

		/// @brief Print the formatted value on a ostream
		///
		/// @param o Target stream
		void print(std::ostream& o) const;

		/// @brief Return in string form
		std::string str() const {std::ostringstream s; print(s); return s.str();}

	private:
		NUMTYPE m_value;
		uint32_t m_pad_width;
		uint32_t m_precision;
};

using fmt_fix = fmt_float<double, fmt_float_f::FIXED>;
using fmt_sci = fmt_float<double, fmt_float_f::SCIENTIFIC>;
using fmt_gen = fmt_float<double, fmt_float_f::GENERAL>;


/// Print the formatted value on a ostream
template<class NUMTYPE, fmt_float_f FORMAT>
void fmt_float<NUMTYPE, FORMAT>::print(std::ostream& o) const
{
	char buf[k_buf_size+1] = {};
	std::chars_format fmt;
	switch(FORMAT) {
		case fmt_float_f::FIXED: fmt = std::chars_format::fixed; break;
		case fmt_float_f::SCIENTIFIC: fmt = std::chars_format::scientific; break;
		default: fmt = std::chars_format::general; break;
	}
	if (m_precision == k_SHORTEST) {
		std::to_chars(buf, buf+k_buf_size, m_value, fmt);
	}
	else {
		std::to_chars(buf, buf+k_buf_size, m_value, fmt, (int)m_precision);
	}
	if (m_pad_width > 0) {
		size_t len = strlen(buf);
		if (len < m_pad_width) {
			for (size_t i=0; i < m_pad_width-len; i++) o << ' ';
		}
	}
	o << buf;
}

// Stream on a std::ostream
template<class NUMTYPE, fmt_float_f FORMAT>
std::ostream& operator<<(std::ostream& o, const fmt_float<NUMTYPE,FORMAT>& n) {n.print(o); return o;}

// Stream on a sink_ch32
template<class NUMTYPE, fmt_float_f FORMAT>
sink_ch32& operator<<(sink_ch32& sink, const fmt_float<NUMTYPE,FORMAT>& s) {fmt32utf8<const fmt_float<NUMTYPE,FORMAT>&> fm(s); fm.write_to_sink(sink); return sink;}


} // namespace dastd


