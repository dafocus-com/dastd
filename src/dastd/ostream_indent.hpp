/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "ostream_basic.hpp"

namespace dastd {

/// @brief Implementation of a `ostream` that applies a variable indenting to each line
///
/// This class detects the end of lines and uses a virtual method to apply an indent to
/// the line itself.
/// The output is the relayed to another std::ostream.
class ostream_indent_base: public ostream_basic {
	public:
		/// @brief Constructor
		///
		/// @param target Stream that will receive the actual output
		/// @param last_wasEOL If true, it will start immediately with an indent; otherwise, the first line will not receive the indent.
		ostream_indent_base(std::ostream& target, bool last_wasEOL=true): m_target(target), m_last_wasEOL(last_wasEOL) {}

	protected:

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override {
			if (m_last_wasEOL) {m_last_wasEOL=false; write_indent(m_target);}
			if (c == '\n') m_last_wasEOL=true;
			m_target.put(c);
		}

		/// @brief Virtual method to be redefined: it must write the indent according to
		/// whatever rule is implemented
		virtual void write_indent(std::ostream& out) = 0;

		/// @brief Implementation of the "flush" action
		///
		/// @return Return `false` in case of error, `true` if ok.
		virtual bool sync() override {m_target.flush(); return !m_target.bad();}

	private:
		// Target stream, i.e. where the actual output will go
		std::ostream& m_target;

		// This marker remembers whether the last character was an end of line.
		// If true, the next action will be to write the indent.
		bool m_last_wasEOL;
};

/// @brief Implementation of the `indent_ostream_base` that indents using programmable ident levels
///
class ostream_indent: public ostream_indent_base {
	public:
		// @brief Constructor
		//
		// @param target Stream that will receive the actual output
		// @param last_wasEOL If true, it will start immediately with an indent; otherwise, the first line will not receive the indent.
		ostream_indent(std::ostream& target, int indent_level=1, bool last_wasEOL=true, const std::string_view& indent_string="\t"):
			ostream_indent_base(target, last_wasEOL), m_indent_level(indent_level), m_indent_string(indent_string) {}

		// Increase the tab level for the given amount
		void indent(int amount) {m_indent_level += amount; if (m_indent_level < 0) m_indent_level=0;}

		// Increase indent by one
		void inc_indent() {indent(1);}

		// Decrease indent by one
		void dec_indent() {indent(-1);}

		// Set a new indent string
		void set_indent_string(const std::string_view& indent_string) {m_indent_string=indent_string;}

		/// @brief Return the indent
		int get_indent() const {return m_indent_level;}

		ostream_indent& operator++(int) {inc_indent(); return *this;}
		ostream_indent& operator--(int) {dec_indent(); return *this;}

	protected:
		// Tab level
		int m_indent_level = 0;

		// Indent string
		std::string m_indent_string;

		// Virtual method to be redefined: it must write the indent according to
		// whatever rule is implemented
		virtual void write_indent(std::ostream& o) override {int i; for(i=0; i<m_indent_level; i++) o << m_indent_string;}
};

} // namespace dastd
