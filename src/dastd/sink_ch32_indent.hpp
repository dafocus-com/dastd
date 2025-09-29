/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "sink_ch32.hpp"
#include "char32string.hpp"

namespace dastd {

/// @brief Implementation of a `sink_ch32` that applies a variable indenting to each line
///
/// This class detects the end of lines and uses a virtual method to apply an indent to
/// the line itself.
/// The output is the relayed to another sink_ch32.
class char32sink_indent_base: public sink_ch32 {
	public:
		// @brief Constructor
		//
		// @param target Stream that will receive the actual output
		// @param last_wasEOL If true, it will start immediately with an indent; otherwise, the first line will not receive the indent.
		char32sink_indent_base(sink_ch32& target, bool last_wasEOL=true): m_target(target), m_last_wasEOL(last_wasEOL) {}

	protected:

		/// @brief Put one char32_t character in the sink
		///
		/// @param code_point The character to be written
		virtual void internal_sink_write_char32(char32_t code_point) override {
			if (m_last_wasEOL) {m_last_wasEOL=false; write_indent(m_target);}
			if (code_point == '\n') m_last_wasEOL=true;
			m_target.sink_write(code_point);
		}

		// Virtual method to be redefined: it must write the indent according to
		// whatever rule is implemented
		virtual void write_indent(sink_ch32& out) = 0;

	private:
		// Target stream, i.e. where the actual output will go
		sink_ch32& m_target;

		// This marker remembers whether the last character was an end of line.
		// If true, the next action will be to write the indent.
		bool m_last_wasEOL;
};

/// @brief Implementation of the `indent_char32sink_base` that indents using programmable ident levels
///
class sink_ch32_indent: public char32sink_indent_base {
	public:
		// @brief Constructor
		//
		// @param target Stream that will receive the actual output
		// @param last_wasEOL If true, it will start immediately with an indent; otherwise, the first line will not receive the indent.
		sink_ch32_indent(sink_ch32& target, int indent_level=1, bool last_wasEOL=true, const std::u32string_view& indent_string=U"\t"):
			char32sink_indent_base(target, last_wasEOL), m_indent_level(indent_level), m_indent_string(indent_string) {}

		// Increase the tab level for the given amount
		void indent(int amount) {m_indent_level += amount; if (m_indent_level < 0) m_indent_level=0;}

		// Increase indent by one
		void inc_indent() {indent(1);}

		// Decrease indent by one
		void dec_indent() {indent(-1);}

		// Set a new indent string
		void set_indent_string(const std::u32string_view& indent_string) {m_indent_string=indent_string;}

	protected:
		// Tab level
		int m_indent_level = 0;

		// Indent string
		char32string m_indent_string;

		// Virtual method to be redefined: it must write the indent according to
		// whatever rule is implemented
		virtual void write_indent(sink_ch32& o) override {int i; for(i=0; i<m_indent_level; i++) o << m_indent_string;}
};

} // namespace dastd
