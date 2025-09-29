/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 30-JAN-2024
*
**/
#pragma once
#include "defs.hpp"

namespace dastd {

/// @brief Locale id
///
/// Locale ids are 32-bits unsigned values formed in this way (from most
/// significant to least signficant)
///
///	octet 0, 1: ISO 639-1 lower case language code (for example "en" for
///	            English)
///	octet 2, 3: 0x00 0x00 or ISO 3166-1 alpha-2 lower case country code
///	            (for example "us" for United States of America)
typedef uint32_t locale_id;

/// @brief Constant value to be used for invalid locale IDs
constexpr locale_id LOCALE_ID_INVALID = 0x00000000;

/// @brief Given a valid language string returns the locale_id
/// @param language_string   A language string like "en", "enus" or "en-us"
/// @return Returns the `locale_id` or LOCALE_ID_INVALID if the string is not valid.
locale_id text_to_locale_id(const char* language_string);

/// @brief Convert the localeCode into a string.
/// @param lid A valid locale_id
/// @return Returns the string like "en" or "en-uk"; in case localeCode is invalid, returns an empty string.
std::string locale_id_to_text(locale_id lid);

/// @brief	Macro to define a locale code by using the two lowercase letters
inline constexpr locale_id locale_id_2(char ch1, char ch2) {return ((locale_id)(((((uint32_t)(ch1))&0xFF) << 24) | ((((uint32_t)(ch2))&0xFF) << 16)));}

constexpr locale_id locale_id_en = locale_id_2('e', 'n');
constexpr locale_id locale_id_it = locale_id_2('i', 't');



/// @brief Check if a character can be used as locale_id
/// @param c The character to be checked
/// @return Returns the lowercase character or zero if the character is not valid
inline char locale_id_valid_char(char c)
{
	if (c >= 'A' && c <= 'Z') return c-'A'+'a';
	if (c >= 'a' && c <= 'z') return c;
	return 0;
}

// Given a valid language string returns the locale_id
inline locale_id text_to_locale_id(const char* language_string)
{
	char buffer[4];
	int i;
	char c;
	memset(buffer, 0, sizeof(buffer));

	int len = (int)strlen(language_string);
	switch (len) {
		case 4:
		case 2: {
			for(i=0; i<len; i++) {
				c = locale_id_valid_char(language_string[i]);
				if (c == 0) return LOCALE_ID_INVALID;
				buffer[i] = c;
			}
			break;
		}
		case 5: {
			if (language_string[2] != '-') return LOCALE_ID_INVALID;
			for(i=0; i<4; i++) {
				c = locale_id_valid_char(language_string[i < 2 ? i : i+1]);
				if (c == 0) return LOCALE_ID_INVALID;
				buffer[i] = c;
			}
			break;
		}
		default: return LOCALE_ID_INVALID;
	}

	locale_id ret = 0;
	for (i=0; i<4; i++) {
		ret <<= 8;
		ret |= (unsigned char)buffer[i];
	}
	return ret;
}

// Convert the localeCode into a string.
inline std::string locale_id_to_text(locale_id lid)
{
	std::string str;
	if (lid == LOCALE_ID_INVALID) return str;
	char ch[4];
	unsigned i;
	for (i=0; i<4; i++) {
		ch[3-i] = (char)(lid & 0xFF);
		lid >>= 8;
	}
	str.push_back(ch[0]);
	str.push_back(ch[1]);
	if (ch[2]) {
		str.push_back('-');
		str.push_back(ch[2]);
		str.push_back(ch[3]);
	}
	return str;
}

} // namespace dastd
