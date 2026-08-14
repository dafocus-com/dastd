/*
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
* @brief Endian-aware operations
**/
#pragma once
#include "defs.hpp"

namespace dastd {

/// @brief Takes a set of bytes and inverts them on another buffer
/// @param source The source bytes
/// @param target The binary buffer that will host the reverse bytes; it must be at least as big as sizeof(NUMTYPE)
inline void invert_bytes(const void* source, void* target, size_t size) {
	size_t i;
	for (i=0; i<size; i++) {
		((uint8_t*)target)[i] = ((const uint8_t*)source)[size-i-1];
	}
}

/// @brief Takes a set of bytes and inverts them in place
/// @param source The source bytes
/// @param target The binary buffer that will host the reverse bytes; it must be at least as big as sizeof(NUMTYPE)
inline void invert_bytes(void* data, size_t size) {
	size_t i;
	uint8_t temp;
	for (i=0; i<size/2; i++) {
		temp = ((uint8_t*)data)[i];
		((uint8_t*)data)[i] = ((uint8_t*)data)[size-i-1];
		((uint8_t*)data)[size-i-1] = temp;
	}
}

/// @brief Takes an integral number in the native form and converts it to little-endian
/// @tparam NUMTYPE Integral type
/// @param source The integral value to be inverted
/// @param target The binary buffer that will host the little endian encoded bytes; it must be at least as big as sizeof(NUMTYPE)
template<concept_integral_or_floating NUMTYPE>
void native_to_little_endian(NUMTYPE source, void* target) {
	#ifdef DASTD_LITTLE_ENDIAN
	memcpy(target, &source, sizeof(NUMTYPE));
	#else
	invert_bytes(&source, target, sizeof(NUMTYPE));
	#endif
}

/// @brief Takes an integral number in the native form and converts it to big-endian
/// @tparam NUMTYPE Integral type
/// @param source The integral value to be inverted
/// @param target The binary buffer that will host the big endian encoded bytes; it must be at least as big as sizeof(NUMTYPE)
template<concept_integral_or_floating NUMTYPE>
void native_to_big_endian(NUMTYPE source, void* target) {
	#ifdef DASTD_BIG_ENDIAN
	memcpy(target, &source, sizeof(NUMTYPE));
	#else
	invert_bytes(&source, target, sizeof(NUMTYPE));
	#endif
}

/// @brief Takes an integral number in the native form and converts it to little-endian
/// @tparam NUMTYPE Integral type
/// @param source The binary buffer that contains the number in little endian form; it must be at least as big as sizeof(NUMTYPE)
/// @param target The native value that will receive the decoded value
template<concept_integral_or_floating NUMTYPE>
void little_endian_to_native(const void* source, NUMTYPE& target) {
	#ifdef DASTD_LITTLE_ENDIAN
	memcpy(&target, source, sizeof(NUMTYPE));
	#else
	invert_bytes(source, &target, sizeof(NUMTYPE));
	#endif
}

/// @brief Takes an integral number in the native form and converts it to big-endian
/// @tparam NUMTYPE Integral type
/// @param source The binary buffer that contains the number in little endian form; it must be at least as big as sizeof(NUMTYPE)
/// @param target The native value that will receive the decoded value
template<concept_integral_or_floating NUMTYPE>
void big_endian_to_native(const void* source, NUMTYPE& target) {
	#ifdef DASTD_BIG_ENDIAN
	memcpy(&target, source, sizeof(NUMTYPE));
	#else
	invert_bytes(source, &target, sizeof(NUMTYPE));
	#endif
}

} // namespace dastd
