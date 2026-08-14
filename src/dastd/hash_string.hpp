/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 13-DEC-2025
**/
#pragma once
#include "hash.hpp"
#include "fmt_string.hpp"
#include <string>

namespace dastd {
/// @brief Writes the bytes on a string without calculating anything
///
/// This class writes on a string the bytes that are used to calculate the hash.
/// It can be used to debug the string used to calculate the hash.
///
class hash_string: public hash {
	protected:
		/// @brief Internal string
		std::string m_binary_data;

		/// @brief Virtual method that adds the indicated bytes to the current hash sum using CRC-32
		///
		/// @param bytes Pointer to the array of raw bytes to be processed
		/// @param length Number of bytes to be processed
		///
		/// @return Returns this hash to allow chaining
		virtual void add_binary(const void* bytes, size_t length) override {m_binary_data.append((const char*)bytes, length);}

	public:
		/// @brief reset the hash calculator to its initial state.
		virtual void clear() override {m_binary_data.clear();}

		/// @brief Print the current hash
		virtual void print(std::ostream& o) const override {o << fmt_s(m_binary_data);}

		/// @brief Get the current CRC-32
		///
		/// @return The CRC-32 calculated so far
		const std::string& get() const {return m_binary_data;}

		/// @brief operator==
		bool operator==(const hash_string& o) const {return get() == o.get();}

		/// @brief operator!=
		bool operator!=(const hash_string& o) const {return get() != o.get();}
};
}
