/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 19-AUG-2025
* 
* Includes the intrinsic on various platforms and provides with system dependend
* function to know if the architecture is supported.
* It covers the main intrisic features, subject to grow in the future.
**/
#pragma once
#include "sysrecog.hpp"
#include <cstdlib>
#include <cassert>

#ifdef DASTD_WINMSC
	#include <xmmintrin.h>
	#include <mmintrin.h>
	#include <immintrin.h>
	#include <intrin.h>
	#define DASTR_X86_INTRISIC
#endif
#if defined(__x86_64__) && (defined DASTD_WINGCC || defined DASTD_LINUX)
	#include <x86intrin.h>
	#define DASTR_X86_INTRISIC
	#define DASTR_X86_INTRISIC_GCC
#endif


namespace dastd::intrinsic {

/// @brief 
/// @details Allocate size bytes of uninitialized storage whose alignment is specified by `alignment` 
/// (implicitly creating objects in the destination area). The `size` parameter must be an integral multiple of `alignment`.
/// 
/// Use `aligned_free()` to free this memory.
/// 
/// @param alignment Number of bytes of alignment
/// @param size      Required memory size
/// @return          Returns the memory pointer
inline void* aligned_alloc(size_t alignment, size_t size)
{
	assert(size > 0);
	assert((size % alignment) == 0);
	#ifdef DASTD_WINMSC
	return _aligned_malloc(size, alignment);
	#else
	return std::aligned_alloc(alignment, size);
	#endif
}

inline void aligned_free(void* ptr)
{
	#ifdef DASTD_WINMSC
	return _aligned_free(ptr);
	#else
	return std::free(ptr);
	#endif
}

}; // namespace dastd
