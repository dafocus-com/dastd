/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "sink_byte.hpp"

namespace dastd {

/// @brief Binary sink object that writes to a std::string
class sink_byte_string: public sink_byte {
	private:
		/// @brief Output stream
		std::string& m_str;

	public:
		/// @brief Constrtuctor
		/// @param str Target stream
		sink_byte_string(std::string& str): m_str(str) {}

		/// @brief Blocking method to write data to the sink
		///
		/// This method will return when the data has been sent. This method will
		/// do its best to queue the data and return immediately.
		/// In case of catastrophic error, it will throw an exception.
		///
		/// @param data      Pointer to the data to be sent
		/// @param data_size Amount of bytes it will send
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual void sink_write(const void* data, size_t data_size) {m_str.append((const char*)data, data_size);}
};


} // namespace dastd
