/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "sink_byte_by_char.hpp"
#include "exception.hpp"
#include "fmt_string.hpp"

namespace dastd {

DASTD_DEF_EXCEPTION(sink_byte_percent_decode_exception)

/// @brief Decodes RFC-3986 percent-encoded strings for URLs and URIs into another sink_byte strings
class sink_byte_percent_decode: public sink_byte_by_char {
	private:
		/// @brief Sink that will receive the string once decoded
		std::ostream& m_output;

		/// @brief Encoding state
		enum class state {
			/// @brief The decoder is expecting a regular character or the % sign
			EXPECTING_CHAR_OR_PERCENT,
			/// @brief The decoder has received a '%' sign and it now expects the first hex char
			EXPECTING_HEX1,
			/// @brief The decoder has received a '%' sign and the first hex char: it now expects the second hex char
			EXPECTING_HEX2
		};

		/// @brief Current execution state
		state m_state = state::EXPECTING_CHAR_OR_PERCENT;

		/// @brief Percent character being formed
		uint8_t m_hex_char;

	public:
		/// @brief Constructor
		/// @param output Output stream
		sink_byte_percent_decode(std::ostream& output): m_output(output) {}

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override;
};


/// @brief Decode a percent encoded string
/// @param encoded Percent encoded string to be decodied
/// @return Returns the decoded string
/// @throw Throws a std::exception in case of error
inline std::string percent_decode(std::string_view encoded)
{
	std::ostringstream ret;
	sink_byte_percent_decode decoder(ret);
	decoder << encoded;
	return ret.str();
}

//------------------------------------------------------------------------------
// (brief) Write one character to the target stream
//------------------------------------------------------------------------------
inline void sink_byte_percent_decode::write_char(char_type c)
{
	if (m_state == state::EXPECTING_CHAR_OR_PERCENT) {
		switch(c) {
			case '%': m_state = state::EXPECTING_HEX1; m_hex_char=0; break;
			case '+': m_output.put(' '); break;
			default: m_output.put(c);
		}
	}
	else {
		m_hex_char <<= 4;
		if (c >= '0' && c <= '9') {
			m_hex_char |= (uint8_t)(c - '0');
		}
		else if (c >= 'A' && c <= 'F') {
			m_hex_char |= (uint8_t)(c - 'A')+10;
		}
		else if (c >= 'a' && c <= 'f') {
			m_hex_char |= (uint8_t)(c - 'a')+10;
		}
		else {
			DASTD_THROW(sink_byte_percent_decode_exception, "sink_byte_percent_decode expected [0-9a-fA-F], got " << fmt_cq(c));
		}
		switch(m_state) {
			case state::EXPECTING_HEX1: m_state=state::EXPECTING_HEX2; break;
			case state::EXPECTING_HEX2: m_state=state::EXPECTING_CHAR_OR_PERCENT; m_output.put((char_type)m_hex_char); break;
			case state::EXPECTING_CHAR_OR_PERCENT: assert(0); break;
		}
	}
}

} //
