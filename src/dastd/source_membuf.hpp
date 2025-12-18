/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "source_with_peek.hpp"
#include <iostream>

namespace dastd {

/// @brief Binary data source bound to a const std::membuf
template<class CHARTYPE>
class source_membuf: public source_with_peek<CHARTYPE> {
	private:
		/// @brief Input buffer
		const CHARTYPE* m_buf;

		/// @brief Remaining characters in the input buffer
		size_t m_remaining;

	public:
		/// @brief Constructor
		/// @param buf Memory buffer
		/// @param length Length of the data in the memory buffer
		source_membuf(const CHARTYPE* buf, size_t length): m_buf(buf), m_remaining(length) {}

		/// @brief Constructor
		/// @param zbuf Zero terminated memory buffer
		source_membuf(const CHARTYPE* zbuf): m_buf(zbuf), m_remaining(std::char_traits<CHARTYPE>::length(zbuf)) {}

		/// @brief Constructor
		/// @param str String to be associated to this source<char>
		/// @param offset Start reading the string at the given offset
		/// @param length Do not read more than `length` characters from the string
		source_membuf(const std::basic_string_view<CHARTYPE>& str, size_t offset=0, size_t length=SIZE_MAX);

		/// @brief Constructor
		/// @param str String to be associated to this source<char>
		/// @param offset Start reading the string at the given offset
		/// @param length Do not read more than `length` characters from the string
		source_membuf(const std::basic_string<CHARTYPE>& str, size_t offset=0, size_t length=SIZE_MAX):
            source_membuf(static_cast<const std::basic_string_view<CHARTYPE>&>(str), offset, length) {}

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
		virtual size_t tentative_count() const override {return m_remaining;}

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

};


//------------------------------------------------------------------------------
// (brief) Constructor
// (param) String
//------------------------------------------------------------------------------
template<class CHARTYPE>
source_membuf<CHARTYPE>::source_membuf(const std::basic_string_view<CHARTYPE>& str, size_t offset, size_t length)
{
	if (offset > str.size()) {m_buf=nullptr; m_remaining=0;}
	else {
		size_t avail = str.size()-offset;
		m_remaining = (avail < length ? avail : length);
		m_buf = str.data();
	}
}

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
size_t source_membuf<CHARTYPE>::tentative_read(CHARTYPE* data, size_t data_size)
{
    size_t ret = tentative_peek(data, data_size);
	m_buf += ret;
	m_remaining -= ret;
	return ret;
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
size_t source_membuf<CHARTYPE>::tentative_peek(CHARTYPE* data, size_t data_size)
{
	if (data_size > m_remaining) data_size = m_remaining;
	memcpy(data, m_buf, data_size*sizeof(CHARTYPE));
	return data_size;
}

} // namespace dastd
