/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 27-APR-2024
**/
#pragma once
#include "source.hpp"
#include <functional>

namespace dastd {

/// @brief Sniffs the bytes coming from the producer and used or discarded by the consumer
///
/// This class can be used to intercept the traffic through a source<char>
/// even for debugging, logging or other needs.
class source_byte_sniffer: public source<char> {
	private:
		/// @brief Actual source
		source<char>& m_source;

		/// @brief Sniff function
		/// @param data       Pointer to the data address
		/// @param data_size  Number of bytes given
		/// @param discarded  Set to `true` if the
		std::function<void(const void* data, size_t data_size, bool discarded)> m_sniff_function;

	public:
		/// @brief Constructor
		/// @param source  Source that is to be sniffed
		/// @param sniff_function Function invoked every time some data is being transfered
		source_byte_sniffer(source<char>& source, std::function<void(const void* data, size_t data_size, bool discarded)> sniff_function): m_source(source), m_sniff_function(sniff_function) {}

		/// @brief Fetch one byte
		/// @param data   Character read from the stream
		/// @return       Returns `true` if one character has been fetched; returns `false` if no
		///               more characters are available and `data` is not valid
		/// @throw        It can throw a std::exception or one of its derivatives
		virtual bool tentative_read_char(char& data);

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read up to `data_size` bytes.
		/// If less (or even zero) bytes are available, it will read them
		/// and return.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of bytes it should try to read
		/// @return          Returns the number of bytes actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_read(char* data, size_t data_size);

		/// @brief Non-blocking method to discard data from the source
		///
		/// This method will attempt to discard up to `data_size` bytes.
		/// If less (or even zero) bytes are available, it will discard them
		/// and return.
		///
		/// @param data_size Max amount of bytes it should try to read
		/// @return          Returns the number of bytes actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		///
		/// Default implementation calls `tentative_read` in a dummy buffer.
		/// Other implementations can optimize this.
		virtual size_t tentative_discard(size_t data_size);
};


//------------------------------------------------------------------------------
// (brief) Fetch one byte
// (param) data   Character read from the stream
// (return)       Returns `true` if one character has been fetched; returns `false` if no
//               more characters are available and `data` is not valid
// (throw)        It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline bool source_byte_sniffer::tentative_read_char(char& data)
{
	bool ret = m_source.tentative_read_char(data);
	if (ret) m_sniff_function(&data, 1, false);
	return ret;
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source
//
// This method will attempt to read up to `data_size` bytes.
// If less (or even zero) bytes are available, it will read them
// and return.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of bytes it should try to read
// (return)          Returns the number of bytes actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline size_t source_byte_sniffer::tentative_read(char* data, size_t data_size)
{
	size_t ret = m_source.tentative_read(data, data_size);
	if (ret > 0) m_sniff_function(data, ret, false);
	return ret;
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to discard data from the source
//
// This method will attempt to discard up to `data_size` bytes.
// If less (or even zero) bytes are available, it will discard them
// and return.
//
// (param) data_size Max amount of bytes it should try to read
// (return)          Returns the number of bytes actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//
// Default implementation calls `tentative_read` in a dummy buffer.
// Other implementations can optimize this.
//------------------------------------------------------------------------------
inline size_t source_byte_sniffer::tentative_discard(size_t data_size)
{
	constexpr size_t BUFSIZE = 256;
	char buf[BUFSIZE];
	size_t totsize = 0;
	while (totsize < data_size) {
		size_t reqsize = std::min(data_size, BUFSIZE);
		size_t readsize = m_source.tentative_read(buf, reqsize);
		if (readsize > 0) m_sniff_function(buf, readsize, true);
		totsize += readsize;
		if (reqsize > readsize) break;
	}
	return totsize;
}


} // namespace dastd
