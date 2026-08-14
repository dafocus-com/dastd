/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 21-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include <cstring>

namespace dastd {

/// @brief Executes a search&replace of all instances of fixed string into another string
/// @tparam CHARTYPE Type of the character; for example, 'char' or 'char32_t'
/// @param value     String that will have its content replaced
/// @param search    String to be searched for
/// @param replace   String to be replaced in place of every `search` match
template<class CHARTYPE>
void search_and_replace(
	std::basic_string<CHARTYPE>& value,
	const std::basic_string<CHARTYPE>& search,
	const std::basic_string<CHARTYPE>& replace)
{
	for (auto next = value.find(search); next != std::basic_string<CHARTYPE>::npos; next = value.find(search, next)) {
		if ((next == 0) || (value[next - 1] != '\r')) {
			value.replace(next, search.length(), replace);
		}
		next += replace.length();
	}
}

/// @brief Remove all the leading spaces of a multi-line string
/// @tparam CHARTYPE Type of the character; for example, 'char' or 'char32_t'
/// @param value     String that will have its content replace
template<class CHARTYPE>
void trim_leading_spaces(std::basic_string<CHARTYPE>& value)
{
	size_t src, tgt;
	unsigned state=0;
	for(src=0,tgt=0; src<value.length(); src++) {
		CHARTYPE ch=value[src];
		switch(state) {
			// State 0: skipping leading spaces
			case 0: {
				if (!std::isspace(ch)) {
					assert(tgt <= src);
					value[tgt]=ch;
					tgt++;
					if (ch != '\n') state=1;
				}
				break;
			}
			// State 1: adding regular characters
			case 1: {
				assert(tgt <= src);
				value[tgt]=ch;
				tgt++;
				if (ch == '\n') state=0;
				break;
			}
		}
	}
	if (tgt < src) value.resize(tgt);
}

/// @brief Remove all the trailing spaces of a multi-line string
/// @tparam CHARTYPE Type of the character; for example, 'char' or 'char32_t'
/// @param value     String that will have its content replace
template<class CHARTYPE>
void trim_trailing_spaces(std::basic_string<CHARTYPE>& value)
{
	size_t line_start=0;

	bool terminate = false;
	while(!terminate) {
		size_t pos = value.find('\n', line_start);
		if (pos == std::basic_string<CHARTYPE>::npos) {
			terminate = true;
			pos = value.length();
		}
		size_t trim_len = 0;
		if (pos - line_start > 0) {
			size_t i;
			for (i=pos; i>line_start && std::isspace(value[i-1]); i--, trim_len++);
		}
		if (trim_len) value.erase(pos-trim_len, trim_len);
		line_start = pos+1-trim_len;
	}
}

/// @brief Case-inseneitve comparison of two strings
/// @param schema1 First string
/// @param s2 Second string
/// @return Returns `<0` if `schema1<s2`, 0 if `schema1==s2` and `>0` if `schema1>s2`
inline int icompare(std::string_view schema1, std::string_view s2)
{
	size_t sz1 = schema1.size();
	size_t sz2 = s2.size();
	size_t len=std::min(sz1, sz2);
	size_t i;
	for (i=0; i<len; i++) {
		uint8_t c1 = (uint8_t)std::tolower(schema1[i]);
		uint8_t c2 = (uint8_t)std::tolower(s2[i]);
		if (c1 < c2) return -1;
		if (c1 > c2) return +1;
	}
	if (sz1 < sz2) return -1;
	if (sz1 > sz2) return +1;
	return 0;
}

/// @brief Shortens the string by returning a cut-down version if too long
/// @tparam CHARTYPE The string character type (e.g. `char`, `char32_t`, etc)
/// @param text The text to be cut
/// @param max_len  The maximum length; if the string is shorter, it will not be cut
/// @param ellipsis Ellipsis to replace the part that has been cut (defaults to `...`)
/// @return Returns the string shortened within the indicated limits
template<class CHARTYPE>
std::basic_string<CHARTYPE> shorten_string(const std::basic_string<CHARTYPE>& text, size_t max_len, const CHARTYPE* ellipsis=nullptr)
{
	size_t ellipsis_len = (ellipsis ? std::char_traits<CHARTYPE>::length(ellipsis) : 3);
	assert(ellipsis_len+2 <= max_len);
	if (text.size() <= max_len) return text;
	std::basic_string<CHARTYPE> ret;
	size_t avail = max_len - ellipsis_len;
	size_t trail = avail/2;
	size_t lead = avail-trail;

	ret = text.substr(0, lead);
	if (ellipsis) ret.append(ellipsis);
	else {
		size_t i;
		for (i=0; i<ellipsis_len; i++) ret.push_back((CHARTYPE)'.');
	}
	ret.append(text.substr(text.size()-trail, trail));
	return ret;
}

constexpr size_t REVSTR_CALC = SIZE_MAX;
///  @brief Reverses a sequence of bytes.
///
///  Reverses a string. Yes, there is strrev, but is seems that some environments
///  don't have it.
///  This function can reverse whatever binary buffer.
///
///  @param buffer Input buffer that contain the bytes to be reversed
///  @param length Length of the buffer. Use `REVSTR_CALC` if `buffer~ is a ASCIIZ buffer and the length can be calculated with `strlen`.
template<class STRTYPE>
void reverse_string(STRTYPE* buffer, size_t length = REVSTR_CALC)
{
	if (length == REVSTR_CALC) length = std::char_traits<STRTYPE>::length(buffer);
	size_t end = length>>1;
	size_t i;
	STRTYPE c;

	for (i=0; i<end; i++) {
		c = buffer[i];
		buffer[i] = buffer[length-1-i];
		buffer[length-1-i] = c;
	}
}


} // namespace dastd
