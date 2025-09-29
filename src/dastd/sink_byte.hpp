/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "defs.hpp"
#include "ostream_basic.hpp"
#include "utf8.hpp"

namespace dastd {

/// @brief Simple binary sink interface
///
/// This class implements a sink object that is able to accept bytes
/// and place them somewhere.
class sink_byte: public ostream_basic {
	public:
		/// @brief Destructor
		virtual ~sink_byte() {}

		/// @brief Blocking method to write data to the sink
		///
		/// This method will return when the data has been sent. This method will
		/// do its best to queue the data and return immediately.
		/// In case of catastrophic error, it will throw an exception.
		///
		/// @param data      Pointer to the data to be sent
		/// @param data_size Amount of bytes it will send
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual void sink_write(const void* data, size_t data_size) = 0;

		/// @brief Write a zero-terminated string
		/// @param asciiz An ascii string terminated by zero
		virtual void sink_write(const char* asciiz) {sink_write(asciiz, strlen(asciiz));}

		/// @brief Blocking method to write data to the sink
		///
		/// This method will return when the data has been sent. This method will
		/// do its best to queue the data and return immediately.
		/// In case of catastrophic error, it will throw an exception.
		///
		/// @param data      String to be sent
		/// @param offset    First character of the string to be written
		/// @param length    Maximum number of characters to write (do not exceed the string)
		/// @throw           It can throw a std::exception or one of its derivatives
		void sink_write(const std::string_view& data, size_t offset=0, size_t length=SIZE_MAX);

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override {sink_write(&c, 1);}

		/// @brief Write one byte to the stream
		void write_byte(uint8_t c) {write_char((char_type)c);}

		/// @brief Write multiple characters to the target stream
		///
		/// The default is to loop on "write_char". It can be redefined for better
		/// optimization.
		virtual void write_chars(const char_type* s, std::streamsize n) override {sink_write(s, (size_t)n);}

		/// @brief Write a UNICODE string encoding it in UTF-8
		virtual void sink_write_utf8(const char32_t* u32str, size_t u32str_size);

		/// @brief Write a UNICODE string encoding it in UTF-8
		void sink_write_utf8(const std::u32string& str, size_t offset=0, size_t length=0);
};


//------------------------------------------------------------------------------
// (brief) Blocking method to write data to the sink
//
// This method will return when the data has been sent. This method will
// do its best to queue the data and return immediately.
// In case of catastrophic error, it will throw an exception.
//
// (param) data      String to be sent
// (param) offset    First character of the string to be written
// (param) length    Maximum number of characters to write (do not exceed the string)
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline void sink_byte::sink_write(const std::string_view& data, size_t offset, size_t length)
{
	if (offset >= data.size()) return;
	size_t maxlen = data.size()-offset;
	sink_write(data.data()+offset, std::min(maxlen, length));
}

//------------------------------------------------------------------------------
// (brief) Write a UNICODE string encoding it in UTF-8
//------------------------------------------------------------------------------
inline void sink_byte::sink_write_utf8(const char32_t* u32str, size_t u32str_size)
{
	char utf8[UTF8_CHAR_MAX_LEN];
	size_t i;
	for (i=0; i<u32str_size; i++) {
		size_t len = write_utf8_asciiz(utf8, u32str[i]);
		if (len) write_chars(utf8, len);
	}
}

//------------------------------------------------------------------------------
// (brief) Write a UNICODE string encoding it in UTF-8
//------------------------------------------------------------------------------
inline void sink_byte::sink_write_utf8(const std::u32string& str, size_t offset, size_t length)
{
	if (offset >= str.size()) return;
	size_t remaining = str.size()-offset;
	sink_write_utf8(str.data(), offset, std::min(remaining, length));
}

} // namespace dastd
