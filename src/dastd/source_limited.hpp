/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "source.hpp"

namespace dastd {

/// @brief Wraps a "dastd::source" limiting the number of characters that can be extracted
template<class CHARTYPE>
class source_limited: public source<CHARTYPE> {
	private:
		/// @brief Input stream
		source<CHARTYPE>& m_sub_source;
		
		/// @brief Remaining characters
		size_t m_remaining = 0;

	public:
		/// @brief Constructor
		/// @param istr Input stream
		/// @param remaining Maximum number of characters that can be read
		source_limited(source<CHARTYPE>& sub_source, size_t remaining): m_sub_source(sub_source), m_remaining(remaining) {}

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
			if (data_size > m_remaining) data_size = m_remaining;
			size_t chars_read = m_sub_source.tentative_read(data, data_size);
			assert(chars_read <= data_size);
			data_size -= chars_read;
			return chars_read;
		}
};

} // namespace dastd
