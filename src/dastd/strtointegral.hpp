/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 14-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include <limits>
#include <cctype>
#include <vector>
namespace dastd {

constexpr uint8_t NOT_A_INTEGRAL_CHAR = 36;

/// @brief Convert a character 0-9A-Za-z into a 0..35 number
///
/// @param ch The character to be converted
///
/// @return Returns the number or NOT_A_INTEGRAL_CHAR in case of error.
template<class NUMTYPE, class CHARTYPE>
NUMTYPE char_to_integral(CHARTYPE ch)
{
	if (ch >= '0' && ch <= '9') return (NUMTYPE)(ch-'0');
	if (ch >= 'A' && ch <= 'Z') return (NUMTYPE)(ch-'A'+10);
	if (ch >= 'a' && ch <= 'z') return (NUMTYPE)(ch-'a'+10);
	return (NUMTYPE)NOT_A_INTEGRAL_CHAR;
}


/// @brief Constant values and enums used by the strtointegral
class strtointegral_constants {
	public:
		static inline constexpr unsigned detect_base = 0;
		static constexpr size_t calc_size = SIZE_MAX;

	/// @brief Parsing result enumeration
	enum result_type {
		NOT_PROCESSED,      ///< Not yet processed
		VALID,              ///< Result is valid
		INVALID_CHAR,       ///< Error due to an invalid character
		VALUE_OUT_OF_RANGE, ///< The overall value is out of range
	};
};

/// @brief Convert a string into an integral value (signed or unsigned)
///
/// Example:
///
///     strtointegral_t<int32_t, char> i32conv;
///     i32conv.convert("-1234");
///     auto pair = data();
///     if (pair.first) cout << "Valid: " << pair.second;
///     else cout << "Invalid";
///
template<class NUMTYPE, class CHARTYPE>
class strtointegral_t: public strtointegral_constants {
	public:

		static_assert(std::is_integral<NUMTYPE>::value, "Integral required");

		/// @brief Execute the conversion from a constant string
		///
		/// @param str Pointer to a sequence of characters to be analyzed
		/// @param str_len Number of characters that have to be taken from str
		/// @param base Numeric base from 2 to 36 or `detect_base` for automatic base detection with 0x, 0b and 0 (octal) prefixes.
		/// @param min_value Minimum allowed value
		/// @param max_value Maximum allowed value
		void convert(const CHARTYPE* str, size_t str_len=calc_size, unsigned base=detect_base, NUMTYPE min_value=std::numeric_limits<NUMTYPE>::lowest(), NUMTYPE max_value=std::numeric_limits<NUMTYPE>::max());

		/// @brief Execute the conversion from a basic_string
		///
		/// @param str Vector of characters
		/// @param base Numeric base from 2 to 36 or `detect_base` for automatic base detection with 0x, 0b and 0 (octal) prefixes.
		/// @param min_value Minimum allowed value
		/// @param max_value Maximum allowed value
		void convert(const std::basic_string_view<CHARTYPE>& str, unsigned base=detect_base, NUMTYPE min_value=std::numeric_limits<NUMTYPE>::lowest(), NUMTYPE max_value=std::numeric_limits<NUMTYPE>::max());

		/// @brief Return the data
		///
		/// Returns the data even if partially parsed. For example, if max value is
		/// 100 and the input string is "105", it returns "10" because reading
		/// the last "5" would have caused an overflow.
		///
		/// @return Return the data even if partially parsed.
		NUMTYPE data_as_is() const {return m_data;}

		/// @brief Return the data if valid
		/// @param data_on_fail Value returned as default in case of failure
		/// @return Return a pair containing as `first` the data and `second` set to `true`
		///         in case the parsing is valid; returns `data_on_fail' and `false` otherwise.
		std::pair<NUMTYPE,bool> data(NUMTYPE data_on_fail=0) const {return (result()==VALID ? std::make_pair(m_data,true) : std::make_pair(data_on_fail,false));}

		/// @brief Return the processing result
		result_type result() const {return m_result;}

		/// @brief Return the number of valid characters
		///
		/// This indicates the number of valid characters that have been taken from
		/// the input string. The remaining characters have not been used
		/// to calculate the value.
		///
		/// @return Number of valid characters
		size_t valid_length() const {return m_valid_length;}

		/// @brief Clear the object and make it ready for another parsing
		void clear() {m_data=0; m_result=NOT_PROCESSED; m_valid_length=0; m_base=detect_base;}

	private:
		/// @brief Internal data
		NUMTYPE m_data = 0;

		/// @brief Parsing result
		result_type m_result = NOT_PROCESSED;

		/// @brief Number of characters analyzed from str
		///
		/// In case of failure, this points to the character that caused it.
		size_t m_valid_length = 0;

		/// @brief Number base
		unsigned m_base = detect_base;

		/// @brief End of file character
		static constexpr CHARTYPE end_of_string = 0;

		/// @brief Initial state
		static constexpr unsigned state_initial = 0;

		/// @brief Structure used to maintain the state of the conversion
		struct conversion_state {
			/// @brief True if the number is negative
			bool m_negative = false;

			/// @brief Current state of the conversion
			unsigned m_state = state_initial;
		};

		/// @brief Execute the one conversion step
		///
		/// Parse a string one character at a time. It sets the internal state with the result.
		///
		/// @param ch Character to be analyzed; they are to be provided in sequence; pass a `end_of_string` character when the string is terminated.
		/// @param min_value Minimum allowed value
		/// @param max_value Maximum allowed value
		/// @param state Conversion state object
		///
		/// @return Returns 'true' if ok, 'false' in case of error
		bool convert_step(CHARTYPE ch, NUMTYPE min_value, NUMTYPE max_value, conversion_state& state);

		/// @brief Process one character in the current base
		///
		/// Converts the character into a number of the current base.
		/// It makes sure that:
		/// - the character is in the correct range for the base
		/// - adding this character, the number does not go out of range
		///   for it integral size (i.e. uses more bits than those available)
		///
		/// @param ch Character to be analyzed
		/// @param state Conversion state object
		///
		/// @return Returns 'true' if ok, 'false' in case of error
		bool add_char(CHARTYPE ch, conversion_state& state);
};



inline std::ostream& operator<<(std::ostream& o, strtointegral_constants::result_type rt) {
	switch(rt) {
		case strtointegral_constants::NOT_PROCESSED     : o << "NOT_PROCESSED"; break; ///< Not yet processed
		case strtointegral_constants::VALID             : o << "VALID"; break; ///< Result is valid
		case strtointegral_constants::INVALID_CHAR      : o << "INVALID_CHAR"; break; ///< Error due to an invalid character
		case strtointegral_constants::VALUE_OUT_OF_RANGE: o << "VALUE_OUT_OF_RANGE"; break; ///< The overall value is out of range
	}
	return o;
}

/// Execute the one conversion step
template<class NUMTYPE, class CHARTYPE>
bool strtointegral_t<NUMTYPE,CHARTYPE>::add_char(CHARTYPE ch, conversion_state& state)
{
	NUMTYPE number = char_to_integral<NUMTYPE,CHARTYPE>(ch);
	if (ch == NOT_A_INTEGRAL_CHAR) {
		// The character is invalid
		m_result=INVALID_CHAR;
		return false;
	}

	// Numerice value not expected in this base
	if (number >= (NUMTYPE)m_base) {m_result=INVALID_CHAR; return false;}

	NUMTYPE tmp_data;
	if constexpr (std::is_signed<NUMTYPE>::value) {
		tmp_data = m_data * m_base + (state.m_negative ? -number : number);
	}
	else {
		assert(!state.m_negative);
		tmp_data = (NUMTYPE)(m_data * m_base + number);
	}

	// Now, make sure it did not go out of the int64_t range
	if (state.m_negative) {
		if (tmp_data > m_data) {m_result=VALUE_OUT_OF_RANGE; return false;}
	} else {
		if (tmp_data < m_data) {m_result=VALUE_OUT_OF_RANGE; return false;}
	}

	// Everything is ok, go ahead
	m_data = tmp_data;
	return true;
}

/// Execute the one conversion step
template<class NUMTYPE, class CHARTYPE>
bool strtointegral_t<NUMTYPE,CHARTYPE>::convert_step(CHARTYPE ch, NUMTYPE min_value, NUMTYPE max_value, conversion_state& state)
{
	bool valid;
	if (state.m_state == state_initial) {
		state.m_state = (m_base == detect_base ? 1 : 4);
	}
	if (ch>0 && ch<=127 && isspace((int)ch)) ch = ' ';

	switch(state.m_state) {
		//------------------------------------------------------------------------------------------
		// STATE 1
		// This state.m_state is called when the base is to be detected.
		//------------------------------------------------------------------------------------------
		case 1: {
			switch (ch) {
				case ' ': valid = true; break;
				case '+': state.m_state=2; valid = true; break;
				case '-': state.m_state=2; {
					if constexpr (std::is_signed<NUMTYPE>::value) {
						state.m_negative = true;
						valid = true;
					}
					else {
						// Minus sign not allowed on unsigned values
						m_result = INVALID_CHAR;
						valid = true;
					}
					break;
				}
				case '0': state.m_state=3; valid = true; break;
				default: {
					m_base = 10;
					state.m_state = 6;
					valid = add_char(ch, state);
				}
			}
			break;
		}
		//------------------------------------------------------------------------------------------
		// STATE 2
		// This state.m_state is called when the base is to be detected and a + or - has already been
		// detected
		//------------------------------------------------------------------------------------------
		case 2: {
			switch (ch) {
				case '0': state.m_state=3; valid = true; break;
				default: {
					m_base = 10;
					state.m_state = 6;
					valid = add_char(ch, state);
				}
			}
			break;
		}
		//------------------------------------------------------------------------------------------
		// STATE 3
		// This state.m_state is called when the base is to be detected and a '0' has been detected.
		// Now it can follow a 'x' for hex, 'b' for decimal or an octal number
		//------------------------------------------------------------------------------------------
		case 3: {
			switch (ch) {
				case 'X':
				case 'x': m_base=16; state.m_state=5; valid = true; break;
				case 'B':
				case 'b': m_base=2; state.m_state=5; valid = true; break;
				case end_of_string: valid = true; m_result = VALID; break;
				default: {
					m_base = 8;
					state.m_state = 6;
					valid = add_char(ch, state);
				}
			}
			break;
		}
		//------------------------------------------------------------------------------------------
		// STATE 4
		// This state.m_state is initially invoked when the base has been given in advance.
		//------------------------------------------------------------------------------------------
		case 4: {
			switch (ch) {
				case ' ': valid = true; break;
				case '+': state.m_state=5; valid = true; break;
				case '-': state.m_state=5; {
					if constexpr (std::is_signed<NUMTYPE>::value) {
						state.m_negative=true;
						valid = true;
					}
					else {
						// Minus sign not allowed on unsigned values
						m_result = INVALID_CHAR;
						valid = false;
					}
					break;
				}
				default: {
					state.m_state = 6;
					valid = add_char(ch, state);
				}
			}
			break;
		}
		//------------------------------------------------------------------------------------------
		// STATE 5
		// This state.m_state requires at least one valid digit to be present
		//------------------------------------------------------------------------------------------
		case 5: {
			state.m_state = 6;
			valid = add_char(ch, state);
			break;
		}

		//------------------------------------------------------------------------------------------
		// STATE 6
		// This state.m_state allowes zero or one one valid digit to be present
		//------------------------------------------------------------------------------------------
		case 6: {
			if (ch == end_of_string) {
				// Make sure it did not go out of the user defined ranges
				if (m_data < min_value) {m_result=VALUE_OUT_OF_RANGE; valid = false;}
				else if (m_data > max_value) {m_result=VALUE_OUT_OF_RANGE; valid = false;}
				else {
					m_result = VALID;
					valid = true;
				}
			}
			else {
				valid = add_char(ch, state);
			}
			break;
		}

		// Invalid state.m_state
		default: assert(false); valid = false;
	}
	if ((ch != end_of_string) && valid) m_valid_length++;
	return valid;
}


/// Execute the conversion from a constant string
template<class NUMTYPE, class CHARTYPE>
void strtointegral_t<NUMTYPE,CHARTYPE>::convert(const CHARTYPE* str, size_t str_len, unsigned base, NUMTYPE min_value, NUMTYPE max_value)
{
	clear();
	conversion_state state;

	if (str_len == calc_size) {
		str_len = std::char_traits<CHARTYPE>::length(str);
	}
	m_base = base;
	size_t i;
	for (i=0; i<str_len; i++) {
		if (!convert_step(str[i], min_value, max_value, state)) return;
	}
	convert_step(end_of_string, min_value, max_value, state);
	return;
}

/// Execute the conversion from a vector
template<class NUMTYPE, class CHARTYPE>
void strtointegral_t<NUMTYPE,CHARTYPE>::convert(const std::basic_string_view<CHARTYPE>& str, unsigned base, NUMTYPE min_value, NUMTYPE max_value)
{
	clear();
	conversion_state state;

	m_base = base;
	for (auto ch: str) {
		if (!convert_step(ch, min_value, max_value, state)) return;
	}
	convert_step(end_of_string, min_value, max_value, state);
	return;
}

/// @brief Execute the conversion from a constant string
///
/// @param str Pointer to a sequence of characters to be analyzed
/// @param str_len Number of characters that have to be taken from str
/// @param base Numeric base from 2 to 36 or `detect_base` for automatic base detection with 0x, 0b and 0 (octal) prefixes.
/// @param min_value Minimum allowed value
/// @param max_value Maximum allowed value
///
/// @return Return a pair containing as `first` the data and `second` set to `true`
///         in case the parsing is valid; returns `data_on_fail' and `false` otherwise.
template<class NUMTYPE, class CHARTYPE>
std::pair<NUMTYPE,bool> strtointegral(const CHARTYPE* str, NUMTYPE data_on_fail=0, size_t str_len=strtointegral_constants::calc_size, unsigned base=strtointegral_constants::detect_base, NUMTYPE min_value=std::numeric_limits<NUMTYPE>::lowest(), NUMTYPE max_value=std::numeric_limits<NUMTYPE>::max())
{
	strtointegral_t<NUMTYPE,CHARTYPE> sit;
	sit.convert(str, str_len, base, min_value, max_value);
	return sit.data(data_on_fail);
}

/// @brief Execute the conversion from a basic_string
///
/// @param str Vector of characters
/// @param base Numeric base from 2 to 36 or `detect_base` for automatic base detection with 0x, 0b and 0 (octal) prefixes.
/// @param min_value Minimum allowed value
/// @param max_value Maximum allowed value
///
/// @return Return a pair containing as `first` the data and `second` set to `true`
///         in case the parsing is valid; returns `data_on_fail' and `false` otherwise.
template<class NUMTYPE, class CHARTYPE>
std::pair<NUMTYPE,bool> strtointegral(const std::basic_string_view<CHARTYPE>& str, NUMTYPE data_on_fail=0, unsigned base=strtointegral_constants::detect_base, NUMTYPE min_value=std::numeric_limits<NUMTYPE>::lowest(), NUMTYPE max_value=std::numeric_limits<NUMTYPE>::max())
{
	strtointegral_t<NUMTYPE,CHARTYPE> sit;
	sit.convert(str, base, min_value, max_value);
	return sit.data(data_on_fail);
}


} // namespace dastd
