/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 30-AUG-2025
**/
#pragma once
#include <limits>
#include <iostream>
#include "marshal_dec.hpp"
#include "marshal_enc.hpp"

namespace dastd {

/// @brief This class can be used to represent a simple value that can be non-set or set to one of its values
/// @tparam TYPE 
template<class TYPE>
class optval {
	private:
		/// @brief Set to `true` if the value is valid; `false` if not set
		bool m_is_set = false;

		/// @brief The transported value
		TYPE m_value = ((TYPE)0);

	public:
		/// @brief Constructor
		optval() = default;

		/// @brief Copy constructor
		/// @param other The source value
		optval(const optval& other): m_is_set(other.m_is_set), m_value(other.m_value) {}

		/// @brief Operator=
		/// @param other The source value
		optval& operator=(const optval& other) {m_value=other.m_value; m_is_set=other.m_is_set; return *this;}

		/// @brief Constructor with its value type; it automatically configure the object as 'is set'
		/// @param value Value to be set
		optval(TYPE value): m_is_set(true), m_value(value) {}

		/// @brief Operator= with its value type; it automatically configure the object as 'is set'
		/// @param value Value to be set
		optval& operator=(TYPE value) {m_value=value; m_is_set=true; return *this;}

		/// @brief Reset the object to 
		void clear() {m_is_set=((TYPE)0); m_value=false;}

		/// @brief Check if the value is set
		/// @return Returns `true` if set, `false` if not set
		bool is_set() const {return m_is_set;}

		/// @brief Check if the value is not set
		/// @return Returns `true` if not set, `false` if set
		bool is_not_set() const {return !is_set();}

		/// @brief Read the internal value; the object must be set, otherwise it asserts
		TYPE get() const {assert(m_is_set); return m_value;}

		/// @brief Print the contents
		/// @param o Output stream
		void print(std::ostream& o) const;

		/// @brief Encode
		/// @param enc 
		void encode(marshal_enc& enc) const;

		/// @brief Set as valid and encode
		/// @param decoder
		void decode(marshal_dec& dec);
};

template<class TYPE>
inline void optval<TYPE>::print(std::ostream& o) const
{
	if (!m_is_set) o << "NOT_SET";
	else {
		if constexpr (std::is_same_v<TYPE, bool>) {
			o << (m_value ? "true" : "false");
		}
		else {
			o << m_value;
		}
	}
}

template<class TYPE>
inline std::ostream& operator<< (std::ostream& o, const optval<TYPE>& n) {n.print(o); return o;}

// Encode
template<class TYPE>
inline void optval<TYPE>::encode(marshal_enc& enc) const 
{
	assert(m_is_set);
	if constexpr (std::is_same_v<TYPE, bool>) {
		enc.encode_bool(m_value);
	}
	else {
		enc.encode<TYPE>(m_value);
	}
}

// Set as valid and encode
template<class TYPE>
inline void optval<TYPE>::decode(marshal_dec& dec) 
{
	if constexpr (std::is_same_v<TYPE, bool>) {
		m_value = dec.decode_bool(); 
	}
	else {
		m_value = dec.decode<TYPE>(); 
	}
	m_is_set=true;
}

using opt_bool_t = dastd::optval<bool>;
using opt_uint8_t = dastd::optval<uint8_t>;
using opt_int8_t = dastd::optval<int8_t>;
using opt_uint16_t = dastd::optval<uint16_t>;
using opt_int16_t = dastd::optval<int16_t>;
using opt_uint32_t = dastd::optval<uint32_t>;
using opt_int32_t = dastd::optval<int32_t>;
using opt_uint64_t = dastd::optval<uint64_t>;
using opt_int64_t = dastd::optval<int64_t>;

} // namespace dastd
