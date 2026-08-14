/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 03-MAY-2024
**/
#pragma once
#include "defs.hpp"
#include <iostream>
#include <cmath>
#include "string_tools.hpp"
#include "strtointegral.hpp"
namespace dastd {

/// @brief Container able to manage a number in multiple forms
///
/// This class is designed to handle a number originated as text
/// and made available under multiple forms.
/// The number is rendered as `double` and, if suitabile, as an
/// integral type of any kind up to `uint64_t`.
class multinum {
	private:
		/// @brief Double value
		double m_value_double = 0;

		/// @brief Intergral container.
		///
		/// It will try to store all values in the `int64_t` variant, except
		/// for the case when the value is so big that will fit only in the
		/// `uint64_t` variant.
		union {
			uint64_t u64;
			int64_t i64;
		} m_value_integral = {.u64 = 0};

		/// @brief Enumerator for the decoding level
		enum class level_t {
			/// @brief The decoding has not executed or the result is invalid
			INVALID     = 0,
			/// @brief Only the double value is available
			DOUBLE_ONLY = 1,
			/// @brief The value is available in `m_value_integral.i64`
			INT64       = 2,
			/// @brief The value is positive and very big, available in `m_value_integral.u64`
			UINT64      = 3,
		};

		/// @brief Decoding level
		level_t m_level = level_t::INVALID;

		/// @brief Internal parsing code. Relies on a temporary std::string provided by the caller
		void internal_parse(std::string& text);

		/// @brief Set the double value and, if suitable, the integers
		void set_double(double d);

	public:
		/// @brief Constructor
		multinum() {}

		/// @brief Copy constructor
		multinum(const multinum& o) {operator=(o);}

		/// @brief Assign operator
		multinum& operator=(const multinum& o);

		/// @brief Constructor with numeric value
		template<concept_integral_or_floating NUMERIC>
		multinum(NUMERIC v) {set<NUMERIC>(v);}

		/// @brief Clear to zero
		void clear() {m_level=level_t::INVALID; m_value_double=0; m_value_integral.u64 = 0;}

		/// @brief Return true if this object contains a valid number
		bool valid() const {return m_level >= level_t::DOUBLE_ONLY;}

		/// @brief Print operator
		void print(std::ostream& o) const;

		/// @brief Read the string and parse the number there contained
		/// @tparam CHARTYPE Type of the character
		/// @param str       Pointer to the string begin
		/// @param len       Length of the string
		template<class CHARTYPE>
		void parse(const CHARTYPE* str, size_t len);

		/// @brief Read the zero terminated string and parse the number there contained
		/// @tparam CHARTYPE Type of the character
		/// @param str       Pointer to the string begin;
		template<class CHARTYPE>
		void parse(const CHARTYPE* str) {return parse(str, count_nonzero<uint32_t>(str));}

		/// @brief Read the zero terminated string and parse the number there contained
		/// @tparam STRTYPE  Type of the string. The string will be accessed with a regular
		///                  iterator and its elements must be castable to `char32_t`.
		/// @param str       Pointer to the string begin;
		template<class STRTYPE>
		void parse(const STRTYPE& str);

		/// @brief Attempt to retrieve a value of the indicated NUMTYPE
		/// @tparam NUMTYPE A valid integral or floating numtype
		/// @param min_value Minimum allowed value
		/// @param max_value Minimum allowed value
		/// @return Returns a pair where `first` is the value and `second` is `true` if the value is valid.
		template<concept_integral_or_floating NUMTYPE>
		std::pair<NUMTYPE,bool> get(NUMTYPE min_value=std::numeric_limits<NUMTYPE>::lowest(), NUMTYPE max_value=std::numeric_limits<NUMTYPE>::max()) const;

		/// @brief Set the internal value starting from any numeric value
		/// @tparam NUMTYPE A valid integral or floating numtype
		/// @param val
		template<concept_integral_or_floating NUMTYPE>
		void set(NUMTYPE val);
};

DASTD_DEF_OSTREAM(multinum);

//------------------------------------------------------------------------------
// (brief) Print operator
//------------------------------------------------------------------------------
inline void multinum::print(std::ostream& o) const
{
	switch(m_level) {
		case level_t::INVALID: o << "INVALID"; break;
		case level_t::DOUBLE_ONLY: o << m_value_double; break;
		case level_t::INT64: o << m_value_integral.i64; break;
		case level_t::UINT64: o << m_value_integral.u64; break;
	}
}

//------------------------------------------------------------------------------
// (brief) Assign operator
//------------------------------------------------------------------------------
inline multinum& multinum::operator=(const multinum& o)
{
	m_value_double = o.m_value_double;
	m_level = o.m_level;
	if (m_level >= level_t::UINT64) m_value_integral.u64 = o.m_value_integral.u64;
	else m_value_integral.i64 = o.m_value_integral.i64;
	return *this;
}

//------------------------------------------------------------------------------
// (brief) Read the string and parse the number there contained
//------------------------------------------------------------------------------
template<class CHARTYPE>
void multinum::parse(const CHARTYPE* str, size_t len)
{
	clear();
	std::string tmpstr;
	size_t i;
	tmpstr.reserve(len);
	for (i=0; i<len; i++) {
		char32_t ch32 = cast_to_unsigned<char32_t, CHARTYPE>(str[i]);
		// If out of range characters are found, don't decode
		if ((ch32 < 32) || (ch32 > 126)) return;
		tmpstr.push_back((char)ch32);
	}
	internal_parse(tmpstr);
}

//------------------------------------------------------------------------------
// (brief) Read the zero terminated string and parse the number there contained
//------------------------------------------------------------------------------
template<class STRTYPE>
void multinum::parse(const STRTYPE& str)
{
	clear();
	std::string tmpstr;
	for(auto ch: str) {
		char32_t ch32 = cast_to_unsigned<char32_t>(ch);
		// If out of range characters are found, don't decode
		if ((ch32 < 32) || (ch32 > 126)) return;
		tmpstr.push_back((char)ch32);
	}
	internal_parse(tmpstr);
}

//------------------------------------------------------------------------------
// (brief) Internal parsing code. Relies on a temporary std::string provided by the caller
//------------------------------------------------------------------------------
inline void multinum::internal_parse(std::string& text)
{
	clear();
	trim_leading_spaces(text);
	trim_trailing_spaces(text);

	// Attempt the conversion of the floating point
	try {
		m_value_double = std::stod(text);
	}
	catch(...) {
		return;
	}
	m_level = level_t::DOUBLE_ONLY;

	// Attempt the conversion to int64_t
	auto pair_i64 = strtointegral<int64_t,char>(text);
	if (pair_i64.second) {
		m_value_integral.i64 = pair_i64.first;
		m_level = level_t::INT64;
	}
	// Attempt the conversion to uint64_t (in case the number is too big for int64_t)
	else {
	 	bool is_neg = (text[0] == '-');
		if (!is_neg) {
			auto pair_u64 = strtointegral<uint64_t,char>(text);
			if (pair_u64.second) {
				m_value_integral.u64 = pair_u64.first;
				m_level = level_t::UINT64;
			}
		}
	}

	// Attempt an upgrade to integer
	if (m_level == level_t::DOUBLE_ONLY) {
		set_double(m_value_double);
	}
}

//------------------------------------------------------------------------------
// (brief) Attempt to retrieve a value of the indicated NUMTYPE
// (tparam) NUMTYPE A valid integral or floating numtype
// (param) min_value Minimum allowed value
// (param) max_value Minimum allowed value
// (return) Returns a pair where `first` is the value and `second` is `true` if the value is valid.
//------------------------------------------------------------------------------
template<concept_integral_or_floating NUMTYPE>
std::pair<NUMTYPE,bool> multinum::get(NUMTYPE min_value, NUMTYPE max_value) const
{
	std::pair<NUMTYPE,bool> invalid((NUMTYPE)0,false);
	if constexpr (std::is_floating_point_v<NUMTYPE>) {
		// If we have no double value, return failure
		if (m_level < level_t::DOUBLE_ONLY) return invalid;

		// If the requested floating point type is smaller than our value,
		// we must compare max/min values after having casted to our type.
		// This will serve also to verify if our double is out of range for
		// a smaller type
		if constexpr (sizeof(double) >= sizeof(NUMTYPE)) {
			// If the value is out of range, return failure
			if ((m_value_double < ((double)min_value)) || (m_value_double > ((double)max_value))) return invalid;
		}
		else {
			// If the value is out of range, return failure
			if ((((NUMTYPE)m_value_double) < min_value) || (((NUMTYPE)m_value_double) > max_value)) return invalid;
		}
		return std::pair<NUMTYPE,bool>((NUMTYPE)m_value_double, true);
	}
	// If the required value is integral
	else if constexpr (std::is_integral_v<NUMTYPE> && (sizeof(NUMTYPE) <= sizeof(uint64_t))) {
		// If the value is marked as UINT64, by design it can be hosted only by an uint64_t.
		// Any other type will be too small.
		if (m_level == level_t::UINT64) {
			if constexpr (std::is_signed_v<NUMTYPE> || sizeof(NUMTYPE)<sizeof(uint64_t)) return invalid;
			else {
				// If the value is out of range, return failure
				if (m_value_integral.u64 < min_value || m_value_integral.u64 > max_value) return invalid;

				return std::pair<NUMTYPE,bool>((NUMTYPE)m_value_integral.u64, true);
			}
		}
		// We have an integral that fits into a int64_t
		else if (m_level == level_t::INT64) {
			// Special case of casting to uint64_t
			if constexpr (std::is_unsigned_v<NUMTYPE> && sizeof(NUMTYPE) >= sizeof(uint64_t)) {
				if (m_value_integral.i64 < (int64_t)min_value) return invalid;
				if (((uint64_t)m_value_integral.i64) > max_value) return invalid;
				return std::pair<NUMTYPE,bool>((NUMTYPE)(m_value_integral.i64), true);
			}
			else {
				// If the value is out of range, return failure
				if (m_value_integral.i64 < (int64_t)min_value || m_value_integral.i64 > (int64_t)max_value) return invalid;
				return std::pair<NUMTYPE,bool>((NUMTYPE)(m_value_integral.i64), true);
			}
		}
		// No integral is available
		else return invalid;
	}
	static_assert(std::is_floating_point_v<NUMTYPE> || (std::is_integral_v<NUMTYPE> && (sizeof(NUMTYPE) <= sizeof(uint64_t))), "Invalid conversion type: must be a floating point or an integral value of a maximum size of 64 bits");
}


//------------------------------------------------------------------------------
// (brief) Set the internal value starting from any numeric value
// (tparam) NUMTYPE A valid integral or floating numtype
// (param) val
//------------------------------------------------------------------------------
template<concept_integral_or_floating NUMTYPE>
void multinum::set(NUMTYPE val)
{
	static_assert(std::is_floating_point_v<NUMTYPE> || (std::is_integral_v<NUMTYPE> && (sizeof(NUMTYPE) <= sizeof(uint64_t))), "Invalid conversion type: must be a floating point or an integral value of a maximum size of 64 bits");

	m_level = level_t::INVALID;

	if constexpr (std::is_floating_point_v<NUMTYPE>) {
		set_double(val);
	}
	// If the required value is signed integral or unsigned small, it must fit in int64_t
	else if constexpr (std::is_signed_v<NUMTYPE> || (sizeof(NUMTYPE) < sizeof(uint64_t))) {
		m_value_integral.i64 = (int64_t)val;
		m_value_double = (double)val;
		m_level = level_t::INT64;
	}
	else {
		m_value_double = (double)val;
		if (val <= (uint64_t)INT64_MAX) {
			m_value_integral.i64 = (int64_t)val;
			m_level = level_t::INT64;
		}
		else {
			m_value_integral.u64 = (int64_t)val;
			m_level = level_t::UINT64;
		}
	}
}

//------------------------------------------------------------------------------
// (brief) Set the double value and, if suitable, the integers
//------------------------------------------------------------------------------
inline void multinum::set_double(double d)
{
	m_value_double = d;
	m_level = level_t::DOUBLE_ONLY;

	// Understand if it is an integer and it can be promoted.
	// Limit in the 32-bit range to avoid weird conversions
	if (std::ceil(d) == d) {
		if ((d >= (double)INT32_MIN) && (d <= (double)INT32_MAX)) {
			m_value_integral.i64 = (int64_t)d;
			m_level = level_t::INT64;
		}
	}
}

} // namespace dastd
