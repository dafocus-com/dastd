/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 14-SEP-2023
**/
#pragma once
#include "defs.hpp"
#include "text_line_info.hpp"

namespace dastd {

/// @brief Calculate the minimun indent among all the lines
///
/// Reads all the lines and returns the leading spaces of the
/// line that has less.
/// It does not consider the lines made only by spaces/tabs/etc.
///
/// @tparam STRTYPE   Type of the string to be parsed
/// @param text       String to be parsed
/// @param tab_len     Each tab counts as `tab_len` spaces
/// @return           Number minimum indent space
template<typename STRTYPE>
uint32_t calc_min_indent(const STRTYPE& text, uint32_t tab_len);

/// @brief Executes the de-indentation of the input text
///
/// Using the rules indicated in `calc_min_indent`, it does
/// the following operations:
///  - removes `calc_min_indent` leading spaces and tabs
///    from all the non-empty lines
///  - removes all spaces/tabs from the empty lines, i.e.
///    the lines formed only by spaces/tabs.
///  - removes the trailing spaces/tabs
///  - terminates all the lines with '\n', including the
///    last one if it was not terminated.
///  - replaces leading tabs with spaces
///
/// @tparam STRTYPE   Type of the string to be parsed
/// @param text       String to be parsed
/// @param tab_len     Each tab counts as `tab_len` spaces
/// @return           The unindented string
template<typename STRTYPE>
STRTYPE deindent(const STRTYPE& text, uint32_t tab_len);


//-------------------------------------------------------------
// INLINE IMPLEMENTATION
//-------------------------------------------------------------

// Calculate the minimun indent among all the lines
template<typename STRTYPE>
uint32_t calc_min_indent(const STRTYPE& text, uint32_t tab_len) {
	static constexpr uint32_t NOT_CALCULATED = UINT32_MAX;
	uint32_t min_indent = NOT_CALCULATED;
	typename STRTYPE::const_iterator iter = text.begin();
	text_line_info tli;

	while (iter != text.end()) {
		tli.analyze_line(text, iter);
		// Avoid the lines either empty or made only of spaces
		if (tli.m_non_space_chars > 0) {
			uint32_t indent = (uint32_t)(tli.m_leading_spaces + tli.m_leading_tabs * tab_len);
			if (indent < min_indent) min_indent = indent;
		}
	}

	return (min_indent == NOT_CALCULATED ? 0 : min_indent);
}

// @brief Executes the de-indentation of the input text
template<typename STRTYPE>
STRTYPE deindent(const STRTYPE& text, uint32_t tab_len)
{
	STRTYPE unindented;
	uint32_t min_indent = calc_min_indent(text, tab_len);
	text_line_info tli;

	typename STRTYPE::const_iterator iter1 = text.begin();
	typename STRTYPE::const_iterator iter2;

	while (iter1 != text.end()) {
		iter2 = iter1;
		tli.analyze_line(text, iter1);

		// Avoid the lines either empty or made only of spaces
		if (tli.m_non_space_chars > 0) {
			// Calculate the actual indent of this line
			uint32_t indent = (uint32_t)(tli.m_leading_spaces + tli.m_leading_tabs * tab_len);

			// Skip the characters involved from the input
			iter2 += (tli.m_leading_spaces + tli.m_leading_tabs);

			// Add the correct number of spaces
			assert(indent >= min_indent);
			unindented.append(indent - min_indent, ' ');

			// Add the rest of the text
			unindented.append(iter2, iter2+tli.body_len());
		}
		unindented += '\n';
	}

	return unindented;
}

} // namespace dastd
