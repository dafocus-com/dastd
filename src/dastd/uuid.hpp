/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 08-Nov-2025
**/
#pragma once
#include "defs.hpp"
#include "sysrecog.hpp"
#include <cstdint>
#include <compare>
#include <cassert>
#include <string_view>
#include <array>
#include "fmt.hpp"
#include "hex_parse.hpp"
#include "random.hpp"

namespace dastd {

/// @brief Implements a 128-bit UUID
class uuid {
	public:
		/// @brief Maximum number of bytes
		static constexpr size_t UUID_LEN_IN_BYTES = 16;

		/// @brief Size of the UUID string encoded (number of characters)
		static constexpr size_t ENCODED_LEN = 36;

		/// @brief Binary array of 16 bytes containing the UUID
		using array_t = uint8_t[UUID_LEN_IN_BYTES];

	private:
		union {
			struct {
				/// @brief Most significant bytes
				uint64_t m_msb;

				/// @brief Least significant bytes
				uint64_t m_lsb;
			} m_ml;

			/// @brief Access the structure as a sequence of bytes
			array_t m_bytes;
		} m_value;
		static_assert(sizeof(m_value) == UUID_LEN_IN_BYTES, "uuid structure must be packed");

		/// @brief Used to format the 8-4-4-4-12 format
		///
		/// For each of the 16 hex octets (formed by two digits), the corresponding position is set to
		/// 1 if the following pair must be preceeded by a dash.
		static constexpr uint8_t k_dash[UUID_LEN_IN_BYTES] = {0,0,0,0,1,0,1,0,1,0,1,0,0,0,0,0};

		#ifdef DASTD_LITTLE_ENDIAN
		static constexpr size_t k_real_to_mem[UUID_LEN_IN_BYTES] = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
		#else
		static constexpr size_t k_real_to_mem[UUID_LEN_IN_BYTES] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		#endif

	public:
		/// @brief Constructor
		constexpr uuid() {clear();}

		/// @brief Constructor
		constexpr uuid(uint64_t msb, uint64_t lsb) {set(msb, lsb);}

		/// @brief Copy constructor
		constexpr uuid(const uuid& o) {operator=(o);}

		/// @brief Parse the given "8-4-4-4-12" string
		///
		/// In case of error, this object will be `is_nil()`
		constexpr uuid(const std::string_view& str) {parse(str);}

		/// @brief Constructor from a binary array
		uuid(const array_t& buffer) {set_binary(buffer);}

		/// @brief Generate a random UUID
		void set_random();

		/// @brief Operator=
		constexpr uuid& operator=(const uuid& o) {m_value.m_ml.m_lsb=o.m_value.m_ml.m_lsb; m_value.m_ml.m_msb=o.m_value.m_ml.m_msb; return *this;}

		/// @brief Clear to NIL UUID
		constexpr void clear() {m_value.m_ml.m_msb=0; m_value.m_ml.m_lsb=0;}

		/// @brief Check if the UUID is NUL-UUID
		constexpr bool is_nil() const {return (m_value.m_ml.m_msb == 0) && (m_value.m_ml.m_lsb == 0);}

		/// @brief Constructor
		constexpr void set(uint64_t msb, uint64_t lsb) {m_value.m_ml.m_msb = msb; m_value.m_ml.m_lsb = lsb;}

		/// @brief Compare with another UUID
		/// @param o The other UUID
		/// @return Returns <0 if this is less than `o`; returns >0 if this is greater than `o`; 
		///         otherwise, if identical, returns 0.
		constexpr int compare(const uuid& o) const;

		/// @brief Get the indicated byte of the UUID sequence
		/// @param pos Position from 0 to 15, where 0 is the most significative
		/// @return Returns the byte
		constexpr uint8_t get_byte(size_t pos) const;

		/// @brief Set the indicated byte of the UUID sequence
		/// @param pos Position from 0 to 15, where 0 is the most significative
		/// @param value Value of the byte to be set
		constexpr void set_byte(size_t pos, uint8_t value);

    /// @brief Three-way comparison operator
    /// @param lhs Left element
    /// @param rhs Right element
    /// @return Returns a strong operator for comparison
    friend constexpr std::strong_ordering operator<=>(const uuid& lhs, const uuid& rhs)
    {
			int diff = lhs.compare(rhs);
			if (diff < 0) return std::strong_ordering::less;
			if (diff > 0) return std::strong_ordering::greater;
      return std::strong_ordering::equivalent;
    }

		/// @brief Operator ==
		/// @param o Other element being compared with `this`
		/// @return Returns `true` if identical, `false` if different
		constexpr bool operator==(const uuid& o) const {return (m_value.m_ml.m_msb==o.m_value.m_ml.m_msb) && (m_value.m_ml.m_lsb==o.m_value.m_ml.m_lsb);}

		/// @brief Operator !=
		/// @param o Other element being compared with `this`
		/// @return Returns `true` if identical, `false` if different
		constexpr bool operator!=(const uuid& o) const {return !operator==(o);}

		/// @brief Get the UUID in binary form, big-endian encoded
		/// @param buffer The target buffer
		void get_binary(array_t& buffer) const;

		/// @brief Set the UUID in binary form, big-endian encoded
		/// @param buffer The source buffer
		void set_binary(const array_t& buffer);

		/// @brief Print the UUID in the standard 8-4-4-4-12 form
		/// @param o The output stream
		void print(std::ostream& o) const;

		/// @brief Parse the given "8-4-4-4-12" string
		/// @param str String to be parsed
		/// @return Returns `true` if ok, `false` in case of error
		bool constexpr parse(const std::string_view& str);
};

DASTD_DEF_OSTREAM(uuid)

// Print the UUID in the standard 8-4-4-4-12 form
inline void uuid::print(std::ostream& o) const 
{
	for(size_t i=0; i<16; i++) {
		if (k_dash[i]) o << '-';
		o << fmt(get_byte(i), 16, 2, false);
	}
}

// Compare with another UUID
inline constexpr int uuid::compare(const uuid& o) const
{
	if (m_value.m_ml.m_msb < o.m_value.m_ml.m_msb) return -1;
	if (m_value.m_ml.m_msb > o.m_value.m_ml.m_msb) return +1;
	if (m_value.m_ml.m_lsb < o.m_value.m_ml.m_lsb) return -1;
	if (m_value.m_ml.m_lsb > o.m_value.m_ml.m_lsb) return +1;
	return 0;
}

// Get the indicated byte of the UUID sequence
inline constexpr uint8_t uuid::get_byte(size_t pos) const
{
	return m_value.m_bytes[k_real_to_mem[pos]];
}

// Set the indicated byte of the UUID sequence
inline constexpr void uuid::set_byte(size_t pos, uint8_t value)
{
	m_value.m_bytes[k_real_to_mem[pos]] = value;
}

// Parse the given "8-4-4-4-12" string
inline constexpr bool uuid::parse(const std::string_view& str)
{
	if (str.size() < ENCODED_LEN) {clear(); return false;}
	size_t inp;
	size_t out;
	uint8_t value;
	for (inp=0, out=0; inp < ENCODED_LEN; inp+=2, out++) {
		if (k_dash[out]) {
			if (str[inp] != '-') {clear(); return false;}
			inp++;
		}
		if (!hex_char_pair(str[inp], str[inp+1], value)) {clear(); return false;}
		set_byte(out, value);
	}
	return (out == UUID_LEN_IN_BYTES);
}

// Get the UUID in binary form, big-endian encoded
inline void uuid::get_binary(array_t& buffer) const
{
	for(size_t i=0; i<UUID_LEN_IN_BYTES; i++) {
		buffer[i] = get_byte(i);
	}
}

// Set the UUID in binary form, big-endian encoded
inline void uuid::set_binary(const array_t& buffer)
{
	for(size_t i=0; i<UUID_LEN_IN_BYTES; i++) {
		set_byte(i, buffer[i]);
	}
}

// Generate a random UUID
inline void uuid::set_random()
{
	g_random.random_bytes(m_value.m_bytes, UUID_LEN_IN_BYTES);

	// Set the most significant bit to avoid having this UUID appear as a legacy Apollo NCS UUID
	m_value.m_bytes[0] |= 0x80;
}


} // namespace dastd
