/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "sink_byte.hpp"

namespace dastd {

/// @brief Binary sink object that writes to a std::ostream
class sink_byte_ostream: public sink_byte {
	private:
		/// @brief Output stream
		std::ostream& m_ostr;

	public:
		/// @brief Constrtuctor
		/// @param ostr Target stream
		sink_byte_ostream(std::ostream& ostr): m_ostr(ostr) {}

		/// @brief Blocking method to write data to the sink
		///
		/// This method will return when the data has been sent. This method will
		/// do its best to queue the data and return immediately.
		/// In case of catastrophic error, it will throw an exception.
		///
		/// @param data      Pointer to the data to be sent
		/// @param data_size Amount of bytes it will send
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual void sink_write(const void* data, size_t data_size) {m_ostr.write((const char*)data, data_size);}
};


} // namespace dastd
