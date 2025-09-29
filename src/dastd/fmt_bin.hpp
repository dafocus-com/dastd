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
#include "fmt.hpp"
#include <ostream>
#include <sstream>
#include <vector>
namespace dastd {

enum class fmt_bin_format {
	HEX_DUMP,          ///< Executes a full fledged hexdump, with offsets and ASCII visualization (upper case hex numbers)
	HEX_DUMP_LC,       ///< Executes a full fledged hexdump, with offsets and ASCII visualization (lower case hex numbers)
	TWO_HEX_DIGITS,    ///< Write a sequence of space separated HEX pairs like "AB 12 7F ..." (upper case hex numbers)
	TWO_HEX_DIGITS_LC, ///< Write a sequence of space separated HEX pairs like "ab 12 7f ..." (lower case hex numbers)
	BITS,              ///< Write a sequence of 0 and 1 writing 8 bits per byte; starts with MSb
};

/// @brief Prints binary data in ASCII escaped form
///
/// In prints the string escaping with \xNN all the characters out of the
/// range 32-127, plus \\ for the backslash.
class fmt_bin {
	public:
		/// @brief Constructor
		/// @param ptr Binary pointer
		/// @param len Length of the data
		/// @param m_flags See DASTD_FMTBINASCII_xxx
		fmt_bin(const void* ptr, size_t len, fmt_bin_format format=fmt_bin_format::HEX_DUMP): m_ptr(ptr), m_len(len), m_format(format) {}

		/// @brief Constructor
		/// @param s String to be printed
		/// @param m_flags See DASTD_FMTBINASCII_xxx
		fmt_bin(const std::string_view& s, fmt_bin_format format=fmt_bin_format::HEX_DUMP): m_format(format) {m_ptr = s.data(); m_len=s.length();}

		/// @brief Constructor
		/// @param s Vector to be printed
		/// @param m_flags See DASTD_FMTBINASCII_xxx
		template <class CHARTYPE>
		fmt_bin(const std::vector<CHARTYPE>& s, fmt_bin_format format=fmt_bin_format::HEX_DUMP): m_format(format) {m_ptr = (const void*)s.data(); m_len=s.size();}

		/// @brief Print the contents
		void print(std::ostream& o) const;

		/// @brief Return as string
		std::string str() const {std::ostringstream ret; print(ret); return ret.str();}

	private:
		const void* m_ptr;
		size_t m_len;
		fmt_bin_format m_format;

		/// @brief Print the contents using HEX_DUMP format
		void print_hex_dump(std::ostream& o, bool lower_case) const;

		/// @brief Print the contents using TWO_HEX_DIGITS format
		void print_two_hex_digits(std::ostream& o, bool lower_case) const;

		/// @brief Print the contents using BITS format
		void print_bits(std::ostream& o) const;
};

DASTD_DEF_OSTREAM(fmt_bin)
DASTD_DEF_OSTREAM_TO_SINK(fmt_bin)

//------------------------------------------------------------------------------
// Print the contents
//------------------------------------------------------------------------------
inline void fmt_bin::print(std::ostream& o) const
{
	switch(m_format) {
		case fmt_bin_format::HEX_DUMP: print_hex_dump(o, false); break;
		case fmt_bin_format::HEX_DUMP_LC: print_hex_dump(o, true); break;
		case fmt_bin_format::TWO_HEX_DIGITS: print_two_hex_digits(o, false); break;
		case fmt_bin_format::TWO_HEX_DIGITS_LC: print_two_hex_digits(o, true); break;
		case fmt_bin_format::BITS: print_bits(o); break;
	}
}

//------------------------------------------------------------------------------
// (brief) Print the contents using TWO_HEX_DIGITS format
//------------------------------------------------------------------------------
inline void fmt_bin::print_two_hex_digits(std::ostream& o, bool lower_case) const
{
	size_t i;
	for(i=0; i<m_len; i++) {
		o << (i?" ":"") << fmt(((const uint8_t*)m_ptr)[i], 16, 2, lower_case);
	}
}

//------------------------------------------------------------------------------
// (brief) Print the contents using HEX_DUMP format
//------------------------------------------------------------------------------
inline void fmt_bin::print_hex_dump(std::ostream& o, bool lower_case) const
{
	static constexpr size_t bytes_per_line = 8;
	static constexpr uint32_t INVALID_CHAR = UINT32_MAX;

	if (m_len == 0) {o << "<empty>" << std::endl; return;}

	size_t remainder = (m_len % bytes_per_line);

	// The upper_end value is an exact multiple of bytes_per_line. It
	// can be bigger than m_len.
	size_t upper_end = (remainder==0 ? m_len : m_len+bytes_per_line-remainder);
	size_t lines = upper_end/bytes_per_line;
	size_t line, col, ofs;

	for (line=0; line<lines; line++) {
		o << "0x" << fmt(line*bytes_per_line, 16, 8, lower_case) << ":";
		for (col=0; col<bytes_per_line; col++) {
			ofs = line*bytes_per_line+col;
			uint32_t ch = (ofs < m_len ? ((const uint8_t*)m_ptr)[ofs] : INVALID_CHAR);
			if (ch == INVALID_CHAR) o << " --";
			else o << " " << fmt(ch, 16, 2, lower_case);
		}
		o << "  ";
		for (col=0; col<bytes_per_line; col++) {
			ofs = line*bytes_per_line+col;
			uint32_t ch = (ofs < m_len ? ((const uint8_t*)m_ptr)[ofs] : INVALID_CHAR);
			if (ch == INVALID_CHAR) o << ' ';
			else if (ch>=32 && ch<127) {
				o << ((char)ch);
			}
			else o << '.';
		}
		o << std::endl;
	}
}

//------------------------------------------------------------------------------
// (brief) Print the contents using BITS format
//------------------------------------------------------------------------------
inline void fmt_bin::print_bits(std::ostream& o) const
{
	size_t i;
	uint8_t s;
	for(i=0; i<m_len; i++) {
		for (s=0; s<8; s++) {
			o << ((((const uint8_t*)m_ptr)[i] & (1 << (7-s))) != 0 ? '1' : '0');
		}
	}
}


} // namespace dastd


