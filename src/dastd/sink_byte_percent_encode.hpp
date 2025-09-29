/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 12-APR-2024
**/
#pragma once
#include "sink_byte_by_char.hpp"
#include "fmt.hpp"
#include <iostream>

namespace dastd {


/// @brief Implementation of a sink that writes a RFC-3986 percent-encoded string for URLs and URIs
///
/// Handles the "percent encoded" strings used in URLs and URIs.
/// These strings contain only the "unreserved characters" as specified by
/// RFC 3986 section 2.3 (January 2005). All the other characters are encoded
/// UTF-8 and inserted with the "%xx" encoding.
class sink_byte_percent_encode: public sink_byte_by_char {
	private:
		/// @brief Target stream
		std::ostream& m_target;

	public:
		/// @brief Constructor
		///
		/// @param target Stream that will receive the actual output
		/// @param last_wasEOL If true, it will start immediately with an indent; otherwise, the first line will not receive the indent.
		sink_byte_percent_encode(std::ostream& target): m_target(target) {}

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override;
};

/// @brief Encode a percent encoded string
/// @param to_be_encoded String to be percent encoded
/// @return Returns the encoded string
/// @throw Throws a std::exception in case of error
inline std::string percent_encode(const std::string& to_be_encoded)
{
	std::ostringstream ret;
	sink_byte_percent_encode encoder(ret);
	encoder << to_be_encoded;
	return ret.str();
}


//------------------------------------------------------------------------------
// (brief) Write one character to the target stream
//------------------------------------------------------------------------------
inline void sink_byte_percent_encode::write_char(char_type c)
{
	if (
		((c >= 'A') && (c <= 'Z'))
		||
		((c >= 'a') && (c <= 'z'))
		||
		((c >= '0') && (c <= '9'))
		||
		(c == '-')
		||
		(c == '_')
		||
		(c == '.')
		||
		(c == '~')
	) {
		m_target << c;
	}
	else {
		m_target << '%' << fmt((unsigned)(unsigned char)c, 16, 2);
	}
}


} // namespace dastd
