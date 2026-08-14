/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 20-MAR-2024
**/
#pragma once
#include "sink_ch32.hpp"
#include "fmt.hpp"
#include <iostream>

namespace dastd {


/// @brief Implementation of a `sink_ch32` that writes plain-ASCII HTML
///
/// Writes the input text with the HTML escapes like &gt; and so.
/// The output is the relayed to a std::ostream.
///
/// Passing a START_HTML character, it will disable the translation of the
/// "<", ">", etc. symbols, allowing inserting HTML.
/// The STOP_HTML will resume the regular translation.
class sink_ch32_html: public sink_ch32 {
	public:
		static constexpr char32_t START_HTML = U'\U0010FFF0';
		static constexpr char32_t STOP_HTML  = U'\U0010FFF1';

	private:
		/// @brief Target stream
		std::ostream& m_target;

		/// @brief String used to represent the newline (\n) character
		std::string m_newline;

		/// @brief True if HTML inlining is active
		bool m_html_inlining = false;

	public:
		/// @brief Constructor
		///
		/// @param target Stream that will receive the actual output
		/// @param last_wasEOL If true, it will start immediately with an indent; otherwise, the first line will not receive the indent.
		sink_ch32_html(std::ostream& target, std::string_view newline="\n"): m_target(target), m_newline(newline) {}

	protected:
		/// @brief Put one char32_t character in the sink
		///
		/// @param code_point The character to be written
		virtual void internal_sink_write_char32(char32_t code_point) override;
};


//------------------------------------------------------------------------------
// (brief) Put one char32_t character in the sink
//
// (param) code_point The character to be written
//------------------------------------------------------------------------------
inline void sink_ch32_html::internal_sink_write_char32(char32_t code_point)
{
	bool print_regular_char=false;
	switch(code_point) {
		case START_HTML: m_html_inlining=true; return;
		case STOP_HTML: m_html_inlining=false; return;
	}

	if (m_html_inlining) {
		print_regular_char = true;
	}
	else {
	  switch(code_point) {
		  case '<' : m_target << "&lt;"; break;
		  case '>' : m_target << "&gt;"; break;
		  case '"' : m_target << "&quot;"; break;
		  case '\'': m_target << "&#39;"; break;
	    case '&' : m_target << "&amp;"; break;
		  default: print_regular_char = true;
	  }
	}
	if (print_regular_char) {
	  switch(code_point) {
			case '\n': m_target << m_newline; break;
			case '\r': break;
		  default: {
		    if ((code_point >= 32) && (code_point <= 127)) m_target << (char)code_point;
		    else m_target << "&#x" << fmt(code_point, 16, 0) << ";";
			}
	  }
	}
}


} // namespace dastd
