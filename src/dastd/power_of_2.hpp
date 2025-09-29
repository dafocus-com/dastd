/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 17-OCT-2024
**/
#pragma once
#include "defs.hpp"
namespace dastd {

constexpr uint32_t NO_BIT_POSITION = UINT32_MAX;

/// @brief Returns the position of the highest `1` bit
/// @tparam UNSIGNED_INTEGRAL Unsigned integral type
/// @return Returns the bit position of the highest bit set to `1`, considering bit 0 the LSB.
///         If the input value is zero, returns NO_BIT_POSITION
template<concept_unsigned_integral UNSIGNED_INTEGRAL>
inline constexpr uint32_t highest_1_bit_position(UNSIGNED_INTEGRAL n) {
	if (n==0) return NO_BIT_POSITION;
	uint32_t pos=0;
	for (;;) {
		n >>= 1;
		if (n==0) return pos;
		pos++;
	}
}


/// @brief Returns true if the number is power of two
/// @tparam INTEGRAL Integral type
/// @return Returns `true` if the given number is greater than zero and it is a power of 2
template<concept_integral INTEGRAL>
inline constexpr bool is_power_of_2(INTEGRAL n) {
	return (n > 0) && ((n & (n - 1)) == 0);
}

/// @brief Rounds to the nearest upper power of two
/// @tparam UNSIGNED_INTEGRAL Unsigned integral type
/// @return Returns the nearest upper power of two; returns 0 in case the value is out of range
template<concept_unsigned_integral UNSIGNED_INTEGRAL>
inline constexpr UNSIGNED_INTEGRAL round_to_nearest_higher_power_of_2(UNSIGNED_INTEGRAL n) {
	if (n == 0) return 0;

	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	if constexpr(sizeof(n) >= 2) {
		n |= n >> 8;
		if constexpr(sizeof(n) >= 4) {
			n |= n >> 16;
			if constexpr(sizeof(n) >= 8) {
				n |= n >> 32;
			}
		}
	}
	n++;
	return n;
}


} // namespace dastd
