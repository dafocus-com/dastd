/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 14-APR-2024
**/
#pragma once
#include "sink_byte.hpp"

namespace dastd {

/// @brief Implements a sink_byte that, by nature, is done char by char
///
/// The main pure virtual method of `sink_byte` is `sink_write`, that is designed
/// to process a set of bytes at once.
/// However some implementations require the bytes to processed one by one.
/// This base class impelements the `sink_write` method as a loop that calls `write_char`.
class sink_byte_by_char: public sink_byte {
	public:
		/// @brief Blocking method to write data to the sink
		///
		/// This method will return when the data has been sent. This method will
		/// do its best to queue the data and return immediately.
		/// In case of catastrophic error, it will throw an exception.
		///
		/// @param data      Pointer to the data to be sent
		/// @param data_size Amount of bytes it will send
		/// @throw           It can throw a `sink_byte_percent_decode_exception` in case of invalid percent encoding
		virtual void sink_write(const void* data, size_t data_size) override {for (size_t i=0; i<data_size; i++) write_char(((const char_type*)data)[i]);}

		/// @brief Write a zero-terminated string
		/// @param asciiz An ascii string terminated by zero
		virtual void sink_write(const char* asciiz) override {for(; *asciiz; asciiz++) write_char(*asciiz);}

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override = 0;
};

} //
