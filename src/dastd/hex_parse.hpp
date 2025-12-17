/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 31-MAR-2025
**/
#pragma once
#include "defs.hpp"
#include "source_with_peek.hpp"
#include "source_string_or_vector.hpp"
#include <vector>
#include <cstdint>

namespace dastd {

constexpr uint8_t INVALID_HEX_CHAR = 16;

/// @brief Converts a character to an 0..15 value
/// @return Returns the value of the char or `INVALID_HEX_CHAR` if invalid
template<class CHARTYPE>
inline constexpr uint8_t hex_char(CHARTYPE ch)
{
	if ((ch >= '0') && (ch <= '9')) return (uint8_t)((ch)-'0');
	if ((ch >= 'A') && (ch <= 'F')) return (uint8_t)((ch)-'A')+10;
	if ((ch >= 'a') && (ch <= 'f')) return (uint8_t)((ch)-'a')+10;
	return INVALID_HEX_CHAR;
}

/// @brief Reads an hex string from a source of characters and coverts it into a sequence of bytes
///
/// It automatically skips spaces, tabs, CRs and newlines.
///
/// @return Returns the number of characters read from `src` before stopping because `src` terminated or
///         an invalid character has been found.
template<class CHARTYPE>
inline size_t hex_parse(source_with_peek<CHARTYPE>& src, std::vector<uint8_t>& bytes)
{
	size_t count=0;
	uint8_t byte = 0;
	CHARTYPE hex[2];
	CHARTYPE c;
	size_t n;

	for(;;) {
		for(;;) {
			// Skip spaces
			if (src.tentative_peek(&c, 1) != 1) break;
			if ((c != ' ') && (c != '\t') && (c != '\r') && (c != '\n')) break;
			src.tentative_discard(1);
			count++;
		}

		// Try to read an hex pair
		n = src.tentative_peek(hex, 2);
		if (n != 2) break;

		uint8_t h = hex_char(hex[0]);
		uint8_t l = hex_char(hex[1]);
		if ((h == INVALID_HEX_CHAR) || (l == INVALID_HEX_CHAR)) break;
		byte = (h << 4) | l;
		bytes.push_back(byte);
		src.tentative_discard(2);
		count += 2;
	}

	return count;
}

inline size_t hex_parse(const std::string& src, std::vector<uint8_t>& bytes)
{
	dastd::string_or_vector<char> sov(src);
	dastd::source_string_or_vector<char> inp(sov);
	return hex_parse<char>(inp, bytes);
}

/// @brief Convert two hex characters into a byte
/// @tparam CHARTYPE Type of the character
/// @param ch1 First characrer
/// @param ch2 Second character
/// @param value Corresponding byte valye
/// @return Returns `true` if valid, `false` if one or both the characters are not valid hex chars
template<class CHARTYPE>
inline constexpr bool hex_char_pair(CHARTYPE ch1, CHARTYPE ch2, uint8_t& value)
{
	value = hex_char<CHARTYPE>(ch1);
	if (value == INVALID_HEX_CHAR) return false;
	value <<= 4;
	uint8_t b2 = hex_char<CHARTYPE>(ch2);
	if (b2 == INVALID_HEX_CHAR) return false;
	value |= b2;
	return true;
}


}
