/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 14-SEP-2023
**/
#pragma once
#include "defs.hpp"

namespace dastd {

/// @brief Structure that describes the leading and trailing spaces of a text line
struct text_line_info {
	/// @brief Length of the line, including the NL at the end if present
	size_t m_length = 0;

	/// @brief Number of leading tabs
	size_t m_leading_tabs = 0;

	/// @brief Number of leading spaces
	size_t m_leading_spaces = 0;

	/// @brief Number of trailing separators
	///
	/// Contains the number of spaces, tabs, CR and NL that are at the end
	/// of the string.
	size_t m_trailing_voids = 0;

	/// @brief Number of non-space characters
	size_t m_non_space_chars = 0;

	/// @brief True if the last character is a newline.
	bool m_has_new_line = false;

	/// @brief Calculate the length of the body, i.e. excluding leading and trailing separators
	size_t body_len() const {return m_length-m_leading_tabs-m_leading_spaces-m_trailing_voids;}

	/// @brief Reset all values to zero
	void clear() {m_length=0; m_leading_tabs=0; m_leading_spaces=0; m_trailing_voids=0; m_non_space_chars=0; m_has_new_line=false;}

	/// @brief True if identical
	bool operator==(const text_line_info& o) {return (m_length==o.m_length) && (m_leading_tabs==o.m_leading_tabs) && (m_leading_spaces==o.m_leading_spaces) && (m_trailing_voids==o.m_trailing_voids) && (m_non_space_chars==o.m_non_space_chars) && (m_has_new_line==o.m_has_new_line);}

	/// @brief True if different
	bool operator!=(const text_line_info& o) {return !operator==(o);}

	/// @brief Print the contents
	void print(std::ostream& o) const;

	/// @brief Calculate the line information inside this object
	///
	/// @tparam STRTYPE Type of the string; must be iterable with data comparable to chars.
	/// @param text     Text to be analyzed
	/// @param iter     Iterator that will be used to iterate; it will point to the first unused character.
	template<typename STRTYPE>
	void analyze_line(const STRTYPE& text, typename STRTYPE::const_iterator& iter);
};

DASTD_DEF_OSTREAM(text_line_info)

// Calculate the line information inside this object
template<typename STRTYPE>
void text_line_info::analyze_line(const STRTYPE& text, typename STRTYPE::const_iterator& iter)
{
	bool is_begin=true;
	clear();

	while(iter != text.end()) {
		m_length++;
		auto ch = *iter;
		iter++;

		if (is_begin) {
			if (ch == ' ') {m_leading_spaces++; continue;}
			if (ch == '\t') {m_leading_tabs++; continue;}
			is_begin=false;
		}
		if ((ch==' ') || (ch=='\t') || (ch=='\r') || (ch=='\n')) m_trailing_voids++;
		else {m_trailing_voids = 0; m_non_space_chars++;}
		if (ch == '\n') {
			m_has_new_line = true;
			return;
		}
	}
}

// Print the contents
inline void text_line_info::print(std::ostream& o) const {
	o << "length=" << m_length << " leading_tabs=" << m_leading_tabs << " leading_spaces=" << m_leading_spaces << " trailing_voids=" << m_trailing_voids << " non_space_chars=" << m_non_space_chars << " has_new_line=" << m_has_new_line;
}


} // namespace dastd
