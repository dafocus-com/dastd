/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 19-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include "fmt.hpp"

namespace dastd {

/// @brief Takes a string and converts it into an unique valid identifiers
///
/// This formatter class takes a whatever string and converts it to a valid
/// identifier formed by the characters in the range [0-9A-Za-z_].
/// Different input strings are guaranteed to produce different identifiers.
/// It also tries to convert the most common symbols into a readable string
/// in the identifier. For example, "!" will become "EXCLAMATION_MARK".
/// It uses the underscore (_) char to escape the sequences.
/// A single underscore is encoded as a double underscore.
///
/// @tparam CHARTYPE A character type like `char` or `char32_t`
template<class CHARTYPE>
class fmt_identifier {
	public:
		/// @brief Constructor
		/// @param chars Pointer to a zero-terminated sequence of characters
		fmt_identifier(const CHARTYPE* chars): m_chars(chars) {m_chars_len = std::char_traits<CHARTYPE>::length(chars);}

		/// @brief Constructor
		/// @param string Pointer to a zero-terminated sequence of characters
		fmt_identifier(const std::basic_string_view<CHARTYPE>& string): m_chars(string.data()), m_chars_len(string.length()) {}

		/// @brief Print the formatted value on a ostream
		///
		/// @param o Target stream
		void print(std::ostream& o) const;

	private:
		/// @brief Pointer to the array of CHARTYPE to be encoded
		const CHARTYPE* m_chars;

		/// @brief Number of characters to be printed
		size_t m_chars_len;
};

// Stream on a std::ostream
template<class CHARTYPE>
std::ostream& operator<< (std::ostream& o, const fmt_identifier<CHARTYPE>& n) {n.print(o); return o;}

// Print the formatted value on a ostream
template<class CHARTYPE>
void fmt_identifier<CHARTYPE>::print(std::ostream& o) const
{
	static_assert(std::is_integral<CHARTYPE>::value, "Integral required");
	static_assert(sizeof(CHARTYPE) <= 4);
	size_t i;
	for (i=0; i<m_chars_len; i++) {
		std::make_unsigned_t<CHARTYPE> ch = (std::make_unsigned_t<CHARTYPE>)m_chars[i];
		if constexpr (sizeof(CHARTYPE) >= 3) {
			if (ch > 0x0000FFFF) {
				o << "_U" << fmt(ch, 16, 8);
				continue;
			}
		}
		if constexpr (sizeof(CHARTYPE) >= 2) {
			if (ch > 0x00FF) {
				o << "_u" << fmt(ch, 16, 4);
				continue;
			}
		}
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
			o << (char)ch;
			continue;
		}
		switch(ch) {
			case '_': o << "__"; break;
			case 0x20: o << "_SPACE"; break;
			case 0x21: o << "_EXCLAMATION_MARK"; break;
			case 0x22: o << "_QUOTATION_MARK"; break;
			case 0x23: o << "_NUMBER_SIGN"; break;
			case 0x24: o << "_DOLLAR_SIGN"; break;
			case 0x25: o << "_PERCENT_SIGN"; break;
			case 0x26: o << "_AMPERSAND"; break;
			case 0x27: o << "_APOSTROPHE"; break;
			case 0x28: o << "_LEFT_PARENTHESIS"; break;
			case 0x29: o << "_RIGHT_PARENTHESIS"; break;
			case 0x2A: o << "_ASTERISK"; break;
			case 0x2B: o << "_PLUS_SIGN"; break;
			case 0x2C: o << "_COMMA"; break;
			case 0x2D: o << "_HYPHEN_MINUS"; break;
			case 0x2E: o << "_FULL_STOP"; break;
			case 0x2F: o << "_SOLIDUS"; break;
			case 0x3A: o << "_COLON"; break;
			case 0x3B: o << "_SEMICOLON"; break;
			case 0x3C: o << "_LESS_THAN_SIGN"; break;
			case 0x3D: o << "_EQUALS_SIGN"; break;
			case 0x3E: o << "_GREATER_THAN_SIGN"; break;
			case 0x3F: o << "_QUESTION_MARK"; break;
			case 0x40: o << "_COMMERCIAL_AT"; break;
			case 0x5B: o << "_LEFT_SQUARE_BRACKET"; break;
			case 0x5C: o << "_REVERSE_SOLIDUS"; break;
			case 0x5D: o << "_RIGHT_SQUARE_BRACKET"; break;
			case 0x5E: o << "_CIRCUMFLEX_ACCENT"; break;
			case 0x60: o << "_GRAVE_ACCENT"; break;
			case 0x7B: o << "_LEFT_CURLY_BRACKET"; break;
			case 0x7C: o << "_VERTICAL_LINE"; break;
			case 0x7D: o << "_RIGHT_CURLY_BRACKET"; break;
			case 0x7E: o << "_TILDE"; break;
			default: o << "_x" << fmt(ch, 16, 2);
		}
	}
}

} // namespace dastd
