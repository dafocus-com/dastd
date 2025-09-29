/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "source.hpp"
#include <iostream>

namespace dastd {

/// @brief Binary data source bound to a std::istream
template<class CHARTYPE>
class source_istream: public source<CHARTYPE> {
	// This class currently supports only 1-byte long types
	static_assert(sizeof(CHARTYPE) == 1, "source_istream currently supports only 1-byte long characters");
	private:
		/// @brief Input stream
		std::istream& m_istr;

	public:
		/// @brief Constructor
		/// @param istr Input stream
		/// @param max_chars Maximum number of characters that can be read; it can be used
		///                  to limit the number of characters that can be extracted from the file.
		source_istream(std::istream& istr): m_istr(istr) {}

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read up to `data_size` characters.
		/// If less (or even zero) characters are available, it will read them
		/// and return.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns the number of characters actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_read(CHARTYPE* data, size_t data_size) override 
		{
			m_istr.read((CHARTYPE*)data, data_size); 
			return m_istr.gcount();
		}
};

} // namespace dastd
