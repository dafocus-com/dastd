/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-MAR-2024
**/
#pragma once
#include "defs.hpp"
#include <istream>

namespace dastd {

/// @brief Implementation of a std::istream that reads its data from a constant buffer
class istream_membuf: public std::istream {
	private:
		/// @brief Internal implementation of a "std::streambuf" that uses the indicated buffer
		struct membuf: std::streambuf
		{
			membuf(const void* buffer, size_t length) {this->setg((char*)buffer, (char*)buffer, ((char*)buffer)+length);}
		};

		/// @brief Instance of membuf used by this istream
		membuf m_membuf;

	public:
		/// @brief Constructor
		/// @param buffer Pointer to the buffer
		/// @param length Length of the buffer
		istream_membuf(const void* buffer, size_t length): std::istream(&m_membuf), m_membuf(buffer, length) {}

		/// @brief Constructor
		/// @param str String view used as a source of data
		istream_membuf(std::string_view str): std::istream(&m_membuf), m_membuf(str.data(), str.size()) {}
};

} // namespace dastd
