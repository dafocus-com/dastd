/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include "float.hpp"
#include <string>
#include <istream>
#include <fstream>

namespace dastd {
/// @brief Base class for hash calculation. Derived to support multiple hashing formats.
class hash {
	public:
		/// @brief Virtual destructor
		virtual ~hash() {}

	protected:
		/// @brief Pure virtual method that adds the indicated bytes to the current hash sum
		///
		/// @param bytes Pointer to the array of raw bytes to be processed
		/// @param length Number of bytes to be processed
		virtual void add_binary(const void* bytes, size_t length) = 0;

	public:
		/// @brief reset the hash calculator to its initial state.
		virtual void clear() = 0;

		/// @brief Print the current hash
		virtual void print(std::ostream& o) const = 0;

		/// @brief Adds the indicated raw bytes to the current hash sum
		///
		/// @param bytes Pointer to the array of raw bytes to be processed
		/// @param length Number of bytes to be processed
		///
		/// @return Returns this hash to allow chaining
		hash& add(const void* bytes, size_t length) {add_binary(bytes, length); return *this;}

		/// @brief Add a string to the hash
		/// @param str String to be added
		/// @return Returns this hash to allow chaining
		template<class CHARTYPE>
		hash& add(const std::basic_string<CHARTYPE>& str) {return add(str.data(), str.length());}

		/// @brief Add a string to the hash
		/// @param str String to be added
		/// @return Returns this hash to allow chaining
		template<class CHARTYPE>
		hash& add(const std::basic_string_view<CHARTYPE>& str) {return add(str.data(), str.length());}

		/// @brief Adds the indicated charaters to the current hash sum
		///
		/// @tparam CHARTYPE Type of the characters series
		/// @param chars Pointer to the array of characters to be processed
		/// @param length Number of bytes to be processed
		///
		/// @return Returns this hash to allow chaining
		template<class CHARTYPE>
		hash& add(const CHARTYPE* chars, size_t length) {
			static_assert(std::is_integral<CHARTYPE>::value, "Integral required");
			size_t i; for(i=0; i<length; i++) add(chars[i]); return *this;
		}

		/// @brief Adds the zero-terminated sequence of characters to the current hash sum
		///
		/// @tparam CHARTYPE Type of the characters series
		/// @param chars Pointer to the array of characters to be processed
		///
		/// @return Returns this hash to allow chaining
		template<class CHARTYPE>
		hash& add(const CHARTYPE* chars) {
			static_assert(std::is_integral<CHARTYPE>::value, "Integral required");
			size_t i; for(i=0; chars[i]; i++) add(chars[i]); return *this;
		}

		/// @brief Add a char to the string
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(char num) {add_binary(&num, 1); return *this;}

		/// @brief Add a uint8_t to the string
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(uint8_t num) {add_binary(&num, 1); return *this;}

		/// @brief Add a int8_t to the string
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(int8_t num) {add_binary(&num, 1); return *this;}

		/// @brief Add a uint16_t to the string (big-endian)
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(uint16_t num);

		/// @brief Add a int16_t to the string (big-endian)
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(int16_t num) {return add((uint16_t)num);}

		/// @brief Add a uint32_t to the string (big-endian)
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(uint32_t num);

		/// @brief Add a int32_t to the string (big-endian)
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(int32_t num) {return add((uint32_t)num);}

		/// @brief Add a uint64_t to the string (big-endian)
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(uint64_t num);

		/// @brief Add a int64_t to the string (big-endian)
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(int64_t num) {return add((uint64_t)num);}

		/// @brief Add a double to the string
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(double num) {return add(pack_f64(num));}

		/// @brief Add a double to the string
		/// @param num Number to be added
		/// @return Returns this hash to allow chaining
		hash& add(float num) {return add(pack_f32(num));}

		/// @brief Add binary data from an input stream
		///
		/// This method reads data from the input stream `in` until either `maxlen` bytes are read
		/// or `in.gcount()` returns less bytes than those requested.
		/// The caller can check the `in` state flags to verify `eofbit`, `failbit` etc.
		///
		/// @param in The input stream
		/// @param maxlen The maximum length read; it can stop before in case of reaching EOF
		/// @return Returns this hash to allow chaining
		hash& add(std::istream& in, size_t maxlen = SIZE_MAX);

		/// @brief Add the contents of a file to the hash
		/// @param file_name Name of the file to be loaded
		/// @param valid `true` if the file has been loaded; `false` in case of error (see `errno`)
		/// @return Returns this hash to allow chaining
		hash& add_file(const std::string& file_name, bool& valid);
};
DASTD_DEF_OSTREAM(hash);

/// @brief Add a uint16_t to the string (big-endian)
/// @param num Number to be added
/// @return Returns this hash to allow chaining
inline hash& hash::add(uint16_t num)
{
	uint8_t bytes[2];
	bytes[0] = ((uint8_t)((num >> 8) & 0xFF));
	bytes[1] = ((uint8_t)((num >> 0) & 0xFF));
	add(bytes, 2);
	return *this;
}

/// @brief Add a uint32_t to the string (big-endian)
/// @param num Number to be added
/// @return Returns this hash to allow chaining
inline hash& hash::add(uint32_t num)
{
	uint8_t bytes[4];
	bytes[0] = ((uint8_t)((num >> 24) & 0xFF));
	bytes[1] = ((uint8_t)((num >> 16) & 0xFF));
	bytes[2] = ((uint8_t)((num >> 8) & 0xFF));
	bytes[3] = ((uint8_t)((num >> 0) & 0xFF));
	add(bytes, 4);
	return *this;
}

/// @brief Add a uint64_t to the string (big-endian)
/// @param num Number to be added
/// @return Returns this hash to allow chaining
inline hash& hash::add(uint64_t num)
{
	uint8_t bytes[8];
	bytes[0] = ((uint8_t)((num >> 56) & 0xFF));
	bytes[1] = ((uint8_t)((num >> 48) & 0xFF));
	bytes[2] = ((uint8_t)((num >> 40) & 0xFF));
	bytes[3] = ((uint8_t)((num >> 32) & 0xFF));
	bytes[4] = ((uint8_t)((num >> 24) & 0xFF));
	bytes[5] = ((uint8_t)((num >> 16) & 0xFF));
	bytes[6] = ((uint8_t)((num >> 8) & 0xFF));
	bytes[7] = ((uint8_t)((num >> 0) & 0xFF));
	add(bytes, 8);
	return *this;
}

/// @brief Add binary data from an input stream
/// @param in The input stream
/// @param maxlen The maximum length read; it can stop before in case of reaching EOF
/// @return Returns this hash to allow chaining
inline hash& hash::add(std::istream& in, size_t maxlen)
{
	static constexpr size_t buf_size = 1024;
	char buf[buf_size];
	size_t total_bytes_read = 0;
	while (total_bytes_read < maxlen) {
		size_t bytes_to_read = (maxlen - total_bytes_read < buf_size ? (maxlen - total_bytes_read) : buf_size);
		in.read(buf, bytes_to_read);
		add(buf, (size_t)in.gcount());
		total_bytes_read += (size_t)in.gcount();
		if (in.gcount() < (std::streamsize)bytes_to_read) break;
	}

	return *this;
}

/// @brief Add the contents of a file to the hash
/// @param file_name Name of the file to be loaded
/// @param valid `true` if the file has been loaded; `false` in case of error (see `errno`)
/// @return Returns this hash to allow chaining
inline hash& hash::add_file(const std::string& file_name, bool& valid)
{
	valid = false;
	std::ifstream file(file_name, std::ios_base::in|std::ios_base::binary);
	if (file.fail()) {
		return *this;
	}
	add(file);
	file.close();
	valid = true;
	return *this;
}

} // namespace dastd
