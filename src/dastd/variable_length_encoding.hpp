/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 24-JUL-2026
* 
* This module implements LEB128 and VLQ, two standard methods of encoding
* an integer value using a variable number of bytes.
* The encoding is executed by reserving the MSB of each byte as a flag that
* tells whether another byte follows, and encoding the value in the remaining 7 bit.
* This type of encoding is suited for values that are normally small but can sometimes
* grow.
* 
* LEB128 (Little Endian Base 128) and VLQ are the same bu the first one incorporates
* the bits from the least significative, while the second encodes from the most significative.
* 
* Info:
*   https://en.wikipedia.org/wiki/Variable-length_quantity
*		https://en.wikipedia.org/wiki/LEB128
* 
* Encoding example of number 624485 (from Wikipedia):
* 
*        MSB ------------------ LSB
*              10011000011101100101  In raw binary
*             010011000011101100101  Padded to a multiple of 7 bits
*         0100110  0001110  1100101  Split into 7-bit groups
*
*   LEB128 encoding:
* 
*        00100110 10001110 11100101  Add high 1 bits on all but last from right to left
*            0x26     0x8E     0xE5  In hexadecimal
*        -> 0xE5 0x8E 0x26           Output stream
*
*   VLQ encoding:
* 
*        10100110 10001110 01100101  Add high 1 bits on all but last from left to right
*            0xA6     0x8E     0x65  In hexadecimal
*        -> 0xA6 0x8E 0x65           Output stream
**/
#pragma once
#include "defs.hpp"
#include "bit_manip.hpp"
#include <limits>
#include <bit>
#include <algorithm>

namespace dastd {

/// @brief Indicates an overflow in the input buffer
static constexpr size_t k_varlen_decode_overflow = std::numeric_limits<size_t>::max();

/// @brief Encodes the given integral unsigned number into the buffer using ULEB128
/// @tparam NUMTYPE Integral unsigned type of the data
/// @param number The number to be encoded
/// @param target The target buffer; it must be big enough to encode the biggest value, i.e. enough
///               bytes to accomodate all the bits of `NUMTYPE` plus 1 extra bit every 8 bits of `NUMTYPE`.
/// @return The number of bytes used in buffer
template<concept_unsigned_integral NUMTYPE>
size_t uleb128_encode(NUMTYPE number, void* target);

/// @brief Decodes the given integral unsigned number from the buffer using ULEB128
/// @tparam NUMTYPE Integral unsigned type of the data
/// @param number The reference to the number that will be set
/// @param source The source buffer
/// @param source_max_length The maximum length of the source buffer
/// @return The number of bytes read from the buffer or `k_varlen_decode_overflow` in case of 
///         overflow of the input data or the capacity of `NUMTYPE`.
template<concept_unsigned_integral NUMTYPE>
size_t uleb128_decode(NUMTYPE& number, const void* source, size_t source_max_length);

/// @brief Encodes the given integral unsigned number into the buffer using VLQ
/// @tparam NUMTYPE Integral unsigned type of the data
/// @param number The number to be encoded
/// @param target The target buffer; it must be big enough to encode the biggest value, i.e. enough
///               bytes to accomodate all the bits of `NUMTYPE` plus 1 extra bit every 8 bits of `NUMTYPE`.
/// @return The number of bytes used in buffer
template<concept_unsigned_integral NUMTYPE>
size_t vlq_encode(NUMTYPE number, void* target);

/// @brief Decodes the given integral unsigned number from the buffer using VLQ
/// @tparam NUMTYPE Integral unsigned type of the data
/// @param number The reference to the number that will be set
/// @param source The source buffer
/// @param source_max_length The maximum length of the source buffer
/// @return The number of bytes read from the buffer or `k_varlen_decode_overflow` in case of 
///         overflow of the input data or the capacity of `NUMTYPE`.
template<concept_unsigned_integral NUMTYPE>
size_t vlq_decode(NUMTYPE& number, const void* source, size_t source_max_length);

/*
 _       _ _
(_)_ __ | (_)_ __   ___  ___
| | '_ \| | | '_ \ / _ \/ __|
| | | | | | | | | |  __/\__ \
|_|_| |_|_|_|_| |_|\___||___/

*/

// Encodes the given integral unsigned number into the buffer using ULEB128
template<concept_unsigned_integral NUMTYPE>
inline size_t uleb128_encode(NUMTYPE number, void* target)
{
	uint8_t* ptr = reinterpret_cast<uint8_t*>(target);

	// Number zero is encoded as a single zero byte
	if (number == 0) {
		(*ptr) = 0x00;
		return 1;
	}
	
	// Get the number of bits required
	unsigned bits = std::bit_width(number);
	size_t size = (bits + 6) / 7;
	size_t pos = 0;
	int bits_offset_in_number = -1;

	while(bits > 0) {
		unsigned bits_to_take = std::min<unsigned>(7, bits);
		bits_offset_in_number += bits_to_take;
		ptr[pos] = (pos < size-1 ? 0x80 : 0x00) | (static_cast<uint8_t>(get_bits_from_word<NUMTYPE>(number, bits_offset_in_number, bits_to_take)));
		bits -= bits_to_take;
		pos++;
	}
	return size;
}

// Decodes the given integral unsigned number from the buffer using ULEB128
template<concept_unsigned_integral NUMTYPE>
inline size_t uleb128_decode(NUMTYPE& number, const void* source, size_t source_max_length)
{
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(source);
	unsigned bits_encoded = 0;
	bool has_next_byte = false;
	size_t bytes_consumed = 0;
	number = 0;
	while (source_max_length > 0) {
		bytes_consumed++;
		has_next_byte = (((*ptr) & 0x80) == 0x80);
		NUMTYPE value = static_cast<NUMTYPE>((*ptr) & 0x7F);

		number |= (value << bits_encoded);
		if (has_next_byte) {
			bits_encoded += 7;
		}
		else {
			bits_encoded += std::bit_width(value);
			break;
		}
		ptr++;
		source_max_length--;
	}

	// If the "has next" bit is set but we ran out of bytes, the data is
	// badly formed.
	// If we ended up encoding too many bits, we must fail as well.
	if (has_next_byte || (bits_encoded > (sizeof(NUMTYPE) * 8))) {
		number = 0;
		return k_varlen_decode_overflow;
	}

	return bytes_consumed;
}

// Encodes the given integral unsigned number into the buffer using VLQ
template<concept_unsigned_integral NUMTYPE>
inline size_t vlq_encode(NUMTYPE number, void* target)
{
	uint8_t* ptr = reinterpret_cast<uint8_t*>(target);

	// Number zero is encoded as a single zero byte
	if (number == 0) {
		(*ptr) = 0x00;
		return 1;
	}
	
	// Get the number of bits required
	unsigned bits = std::bit_width(number);
	size_t size = (bits + 6) / 7;
	size_t pos = size-1;
	int bits_offset_in_number = -1;

	while(bits > 0) {
		unsigned bits_to_take = std::min<unsigned>(7, bits);
		bits_offset_in_number += bits_to_take;
		ptr[pos] = (pos < size-1 ? 0x80 : 0x00) | (static_cast<uint8_t>(get_bits_from_word<NUMTYPE>(number, bits_offset_in_number, bits_to_take)));
		bits -= bits_to_take;
		pos--;
	}
	return size;
}

// Decodes the given integral unsigned number from the buffer using VLQ
template<concept_unsigned_integral NUMTYPE>
inline size_t vlq_decode(NUMTYPE& number, const void* source, size_t source_max_length)
{
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(source);
	unsigned bits_encoded = 0;
	bool has_next_byte = false;
	size_t bytes_consumed = 0;
	number = 0;
	bool is_first_octet = true;

	while (source_max_length > 0) {
		bytes_consumed++;
		has_next_byte = (((*ptr) & 0x80) == 0x80);
		NUMTYPE value = static_cast<NUMTYPE>((*ptr) & 0x7F);

		unsigned bits_in_this_octet = 7;
		if (is_first_octet) {
			bits_in_this_octet = std::bit_width(value);
			is_first_octet = false;
		}

		number <<= 7;
		number |= (value);
		
		bits_encoded += bits_in_this_octet;

		if (!has_next_byte) break;
		ptr++;
		source_max_length--;
	}

	// If the "has next" bit is set but we ran out of bytes, the data is
	// badly formed.
	// If we ended up encoding too many bits, we must fail as well.
	if (has_next_byte || (bits_encoded > (sizeof(NUMTYPE) * 8))) {
		number = 0;
		return k_varlen_decode_overflow;
	}

	return bytes_consumed;
}


} // namespace dastd
