/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "sysrecog.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <ostream>
#include <memory>
#include <ostream>

// Requires at least C++ 20
static_assert(DASTD_CPP_VER >= 20);

#ifdef DASTD_WINMSC
	// This entry addresses an issue with "std::mutex". If the MSVC runtime has been updated by
	// some older application, the executables compiled with the current version will crash
	// when using a mutex the first time.
	// The solution is to either define "_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR" or to link
	// statically the runtime libraries and avoid any risk.
	//
	// See:
	// https://stackoverflow.com/questions/78598141/first-stdmutexlock-crashes-in-application-built-with-latest-visual-studio
	//
	// To enable the static libraries, in the .vcxproj find the <ClCompile> sections for debug and release
	// and add the following entries (for debug and release):
	//
	//  <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
	//  <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
	//
	// Then add ";DASTD_MSC_STATIC" right before each "</PreprocessorDefinitions>"
	//
	#if !defined _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR && !defined DASTD_MSC_STATIC
	#ifdef NDEBUG
	#error "Either define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR or select 'Configuration Properties|C/C++|Code Generation|Runtime Library|Multi-threaded (/MT)' and define DASTD_MSC_STATIC"
	#else
	#error "Either define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR or select 'Configuration Properties|C/C++|Code Generation|Runtime Library|Multi-threaded Debug (/MTd)' and define DASTD_MSC_STATIC"
	#endif
	#endif
#endif


#define DASTD_DEF_OSTREAM(t) inline std::ostream& operator<< (std::ostream& o, const t& n) {n.print(o); return o;}
#define DASTD_DEF_OSTREAM_SHARED_PTR(t) inline std::ostream& operator<< (std::ostream& o, std::shared_ptr<const t> n) {if (n) n->print(o); else o << "nullptr"; return o;}
#define DASTD_NUMCMP(a,b) ((a)<(b)?-1:((a)==(b)?0:1))


#ifndef UINT8_MIN
#define UINT8_MIN 0
#endif

#ifndef UINT16_MIN
#define UINT16_MIN 0
#endif

#ifndef UINT32_MIN
#define UINT32_MIN 0
#endif

#ifndef UINT64_MIN
#define UINT64_MIN 0
#endif

namespace dastd {
	/// @brief Seconds from epoch (01-JAN-1970), 64-bit version
	using epoch64_t = int64_t;

	/// @brief Seconds from epoch (01-JAN-1970), 32-bit version
	using epoch32_t = int32_t;

	/// @brief Microseconds from epoch (01-JAN-1970)
	using epoch64us_t = int64_t;
	inline constexpr epoch64us_t epoch64us_INVALID = std::numeric_limits<epoch64us_t>::max();
} // namespace dastd

/*------------------------------------------------------------------------------
	These macros can be used to handle unsigned values (8, 16, 32 or 64 bit)
	as maps of n-bit unsigned values.
		var         is the "composite" value
		fstbit      is the first bit on which we want to operate; 0 is the least
								significant bit
		len         is the number of bits we want to manipulate; they go from
								fstbit upward; for example, on a 32-bit word we can store data
								having fstbit=28 and len=4; we can't set len bigger than 4 in
								this case.
		value       is the unsigned value to be set.

		BINSET      takes "var" (an r-value) and returns a copy of "var" where
								"value" has been written at fstbit using "len" bits;
		BINGET      takes "var" and extracts "len" bits starting from "fstbit";
------------------------------------------------------------------------------*/
#define DASTD_BINSET(var,fstbit,len,value) (((var) & ~(((1<<(len))-1) << (fstbit)))|((value) << (fstbit)))
#define DASTD_BINGET(var,fstbit,len) (((var) >> (fstbit)) & ((1<<(len))-1))

/* True if 'a' has the bits 'b' set */
#define DASTD_ISSET(a,b) (((a) & (b)) == (b))

/* True if 'a' has none of the bits 'b' set */
#define DASTD_NONEISSET(a,b) (((a) & (b)) == 0)

/* True if 'a' has any of the bits 'b' set */
#define DASTD_ANYISSET(a,b) (((a) & (b)) != 0)

/* Set the bit(s) 'b' of 'a' */
#define DASTD_SETBIT(a,b) ((a) |= (b))

/* Clear the bit(s) 'b' of 'a' */
#define DASTD_RESETBIT(a,b) ((a) &= ~(b))

/* Set the bit(s) 'b' of 'a' after having cleared the bits set to 1 in "mask" */
#define DASTD_SETBIT_MASKED(a,b,mask) ((a) = ((a) & ~(mask)) | (b))

/* Returns the bits after having applied a mask */
#define DASTD_GETBIT_MASKED(a,mask) ((a) & (mask))

/* Given 'n', returns the value having bit 'n' set */
#define DASTD_BITN(n) (1 << (n))

/* Returns true if the bit 'n' of 'a' is set. n=0 is the least significant bit */
#define DASTD_BITN_ISSET(a,n) (((a) & (1 << (n))) != 0)

namespace dastd {
inline constexpr char32_t CHAR32_INVALID = UINT_LEAST32_MAX;


/// @brief Safe version of strerror
inline std::string dastrerror(int err) {
	#ifdef DASTD_WINMSC
	const size_t BUFSIZE = 512;
	char buf[BUFSIZE];
	strerror_s(buf, BUFSIZE, err);
	buf[BUFSIZE-1] = '\0';
	return std::string(buf);
	#else
	return std::string(strerror(err));
	#endif
}

/// @brief Safe version of strerror
inline std::string dastrerror() {return dastrerror(errno);}

/// @brief Metaprogramming template to detect types suitable to become `std::basic_string` elements
///
/// This operates exacly as `std::is_unsigned_v<T>` except for the fact that is is true
/// if the `T` is a valid character type used in regular strings.
///
/// @tparam CHARTYPE Type to be checked
template<class CHARTYPE>
inline constexpr bool is_char_v = (std::is_same_v<CHARTYPE,char> || std::is_same_v<CHARTYPE,wchar_t> || std::is_same_v<CHARTYPE,char8_t> || std::is_same_v<CHARTYPE,char16_t> || std::is_same_v<CHARTYPE,char32_t>);

/// @brief Cast a value to a given unsigned value
///
/// The need of this utility is when we need to transfer the bits
/// and not convert the number.
///
/// Example:
///
///    char c = -1;              // 'c' is 0xFF
///    uint32_t n = (uint32_t)c; // 'n' is 0xFFFFFFFF
///    uint32_t m = cast_to_unsigned<uint32_t>(c); // 'n' is 0x000000FF
///
/// @tparam TARGET    Target type; must be an unsigned integral value
/// @tparam SOURCE  Source type; it can be an object as long as it has a cast to TARGET operator
/// @param  src       Source data
/// @return           Returns the converted value
template<class TARGET, class SOURCE>
TARGET cast_to_unsigned(SOURCE src)
{
	static_assert(std::is_unsigned_v<TARGET>);
	if constexpr (std::is_integral_v<SOURCE>) {
		static_assert(sizeof(TARGET) >= sizeof(SOURCE), "TARGET unsigned type must be equal or bigger than the SOURCE type");
		using uSOURCE = std::make_unsigned_t<SOURCE>;
		return (TARGET)(uSOURCE)src;
	}
	else {
		return (TARGET)src;
	}
}

/// @brief Counts the characters of the array that return non-zero or when casted to `CASTTYPE`
/// @tparam CASTTYPE Type each element is casted to
/// @tparam CHARTYPE Type of the pointer
/// @param ptr Pointer to the memory array
/// @return Returns the number of non-zero elements
///
/// This function can be used as a generic `strlen`.
template<class CASTTYPE, class CHARTYPE>
size_t count_nonzero(const CHARTYPE* ptr) {
	size_t i;
	for (i=0; (CASTTYPE)(ptr[i]); i++);
	return i;
}


/// @brief These constexprs can be used to verify if class "C" is an instance of
///        template "T"
///
/// Example:
///
///    // Fails if `TYPE` is not an instance of template `std::vector`
///    static_assert(dastd::is_instance_of_v<TYPE, std::vector>);
///
template<class C, template<class...> class T>
inline constexpr bool is_instance_of_v = std::false_type{};

template<template<class...> class T, class... Vs>
inline constexpr bool is_instance_of_v<T<Vs...>,T> = std::true_type{};

/// @brief Concept for integral types
template <class T>
concept concept_integral = std::is_integral_v<T>;

/// @brief Concept for integral types or floating point
template <class T>
concept concept_integral_or_floating = std::is_integral_v<T> || std::is_floating_point_v<T>;

/// @brief Concept for integral unsigned types
template <class T>
concept concept_unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;


/// @brief Concept for 8-bit integral types
template <class T>
concept concept_integral_8bit = std::is_integral_v<T> && (sizeof(T) == 1);

// This macro can be used to take advantage of the `<<` operator of `std::ostream`
// where a `std::string` is required.
//
// For example:
//
//      throw std::runtime_error( DSTR("The error is due to value " << val << " being too big") );
//
#define DSTR(s) (static_cast<const std::ostringstream>(std::ostringstream() << s).str())

} // namespace dastd
