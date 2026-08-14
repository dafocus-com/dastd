/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "source_with_peek.hpp"
#include "string_or_vector.hpp"
#include <iostream>

namespace dastd {

/// @brief Binary data source that feeds data from an internal copy of a string_or_vector
template<class CHARTYPE>
class source_string_or_vector: public source_with_peek<CHARTYPE> {
	private:
		/// @brief Current offset
		size_t m_offset = 0;

		/// @brief Internal string or vector
		string_or_vector<CHARTYPE> m_string;

	public:
		/// @brief Constructor
		/// @param string Source string; it can be a std::basic_string<CHARTYPE>` (for example,
		///               a 'std::string' if `CHARTYPE=char`) or a `std::vector<CHARTYPE>` in
		///               case `CHARTYPE` is not a regular char type.
		source_string_or_vector(const string_or_vector<CHARTYPE>& string): m_string(string) {}

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
		virtual size_t tentative_read(CHARTYPE* data, size_t data_size) override;

		/// @brief Return the number of characters that could be read at the next tentative_read
		///
		/// This method returns the current max value that can be obtained by calling
		/// `tentative_read` right now. Note that between the call to `tentative_count` and
		/// `tentative_read`, the value might changed. So there is no guarantee that the
		/// following calls will conform to this value.
		///
		/// This call is meant to allow preallocation of data with a good guess.
		virtual size_t tentative_count() const override {return m_string.size() - m_offset;}

		/// @brief Non-blocking method to read data from the source without extracting it
		///
		/// This method will attempt to read up to `data_size` characters.
		/// If less (or even zero) characters are available, it will read them
		/// and return. The data will not be extracted from the buffer so the
		/// next time the same data will be returned again.
		/// Call `tentative_discard` to discard the data that has been actually used.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns the number of characters actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_peek(CHARTYPE* data, size_t data_size) override;

		/// @brief Fetch one byte
		/// @param data   Character read from the stream
		/// @return       Returns `true` if one character has been fetched; returns `false` if no
		///               more characters are available and `data` is not valid
		/// @throw        It can throw a std::exception or one of its derivatives
		virtual bool tentative_peek_char(CHARTYPE& data) {return (tentative_peek(&data, 1) == 1);}
};


//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source
//
// This method will attempt to read up to `data_size` characters.
// If less (or even zero) characters are available, it will read them
// and return.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of characters it should try to read
// (return)          Returns the number of characters actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
template<class CHARTYPE>
size_t source_string_or_vector<CHARTYPE>::tentative_read(CHARTYPE* data, size_t data_size)
{
	size_t characters_read = tentative_peek(data, data_size);
	m_offset += characters_read;
	return characters_read;
}


//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source without extracting it
//
// This method will attempt to read up to `data_size` characters.
// If less (or even zero) characters are available, it will read them
// and return. The data will not be extracted from the buffer so the
// next time the same data will be returned again.
// Call `tentative_discard` to discard the data that has been actually used.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of characters it should try to read
// (return)          Returns the number of characters actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
template<class CHARTYPE>
size_t source_string_or_vector<CHARTYPE>::tentative_peek(CHARTYPE* data, size_t data_size)
{
	size_t remaining = tentative_count();
	if (data_size > remaining) data_size = remaining;
	memcpy(data, m_string.data()+m_offset, data_size*sizeof(CHARTYPE));
	return data_size;
}

} // namespace dastd
