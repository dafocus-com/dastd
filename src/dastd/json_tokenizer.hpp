/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 01-MAY-2024
*
* Implementation of a RFC 8259 compliant JSON tokenizer.
*
* In addition to RFC 8259 rules, this tokenizer will automatically recognize
* as "spaces" comments in the slash-star form (like C) and in the single
* line slash-slash form and hash (#) form.
* 
* 
* CHARACTER FETCHING
* ^^^^^^^^^^^^^^^^^^
* The tokenizer operates with one lookahead character. When instanced, the
* `first_char` parameter passed to the constructor will contain the first characters
* of the string. This character will be consumed by the tokenizer.
* From now on, every time we call `process_char(ch)` we must consume the previous char
* and peek `ch` without consuming it. If the call returns that it is done, `ch` has
* not been used and must remain in the stream for further consumption.
**/
#pragma once
#include "defs.hpp"
#include "source_with_peek.hpp"
#include "string_or_vector.hpp"
#include "char32string.hpp"
#include "strtointegral.hpp"
#include "multinum.hpp"
#include "utf16.hpp"

namespace dastd {


/// @brief Return values
enum class json_tokenizer_ret {
	/// @brief A parsing error has occured; the string parsed so far is in 'm_raw_token'
	C_ERROR,
	/// @brief The result is still being processed. Send more characters.
	C_NEED_MORE_CHARS,
	/// @brief In the input data there is nothing else
	C_NOTHING_MORE,
	/// @brief The character submitted is a space and it has been discarded
	C_SPACE,
	/// @brief Matched '{'
	C_BRACE_OPEN,
	/// @brief Matched '}'
	C_BRACE_CLOSE,
	/// @brief Matched '['
	C_BRACKET_OPEN,
	/// @brief Matched ']'
	C_BRACKET_CLOSE,
	/// @brief Matched ','
	C_COMMA,
	/// @brief Matched ':'
	C_COLON,
	/// @brief Matched 'true'
	C_TRUE,
	/// @brief Matched 'false'
	C_FALSE,
	/// @brief Matched 'null'
	C_NULL,
	/// @brief Matched a string
	C_STRING,
	/// @brief Matched a number, `get_multinum()` to retrieve the value
	C_NUMBER,
	/// @brief Matched a string that contains a valid number; use
	///        `get_multinum()` to get the number or `get_string()`
	///        for the string
	///
	/// Returned only if flag `NUMBERS_IN_STRINGS` is set.
	C_STRING_AND_NUMBER,
};

/// @brief Splits a flow of characters into RFC 8259 compliant JSON tokens
///
/// @tparam CHARTYPE  Type of the character; it can be a regular character type, like `char`
///                   or `char32_t`, or a more complex structure that might contain additional
///                   information like the position where the character is located. In this case,
///                   the structure must support a cast to "char32_t" that returns the character.
///
/// @tparam RAWSTRING Container used to store the sequence of CHARTYPEs used to assemble
///                   the current token. The `json_tokenizer` class will use only the
///                   `push_back` and the `clear` methods, thus any regular STD C++ container
///                   should fit. By default, it uses a regular `std::string`-like if
///                   CHARTYPE is a valid character type; otherwise, it uses a `std::vector`.
template<class CHARTYPE, class RAWSTRING = string_or_vector<CHARTYPE>>
class json_tokenizer_base {
	private:
		/// @brief Contains the complete raw token
		///
		/// For example, in case of a string, contains everything from the initial
		/// double quote to the final one included.
		RAWSTRING m_raw_token;

		/// @brief Decoded string (escapes resolved, double quotes removed, etc.)
		///
		/// It contains the token that has been matched in case
		char32string m_string;

		/// @brief The string can be converted to a number
		///
		/// Used in case the `NUMBERS_IN_STRINGS` is active.
		/// This flag is reset if the decoder detects a character that
		/// will prevent the conversion to a number. In that case the
		/// conversion will not be attempted.
		bool m_string_can_converted_to_number = false;

		/// @brief Contains the number, valid in case it returned C_NUMBER
		multinum m_multinum;

		/// @brief Configuration flags
		uint32_t m_flags = 0;

		/// @brief Holds the UNICODE characters being formed with the \uNNNN escape
		char32_t m_escaped_char = 0;

		/// @brief Number of HEX characters that still need to be read to fill the `m_escaped_char`
		unsigned m_escaped_char_remaining = 0;

		/// @brief Current runtime state
		enum class state_t {
			/// @brief Expecting the first character of a token; skips the spaces
			IDLING,

			/// @brief It reached EOF, there is nothing else to do
			REACHED_EOF,

			/// @brief Inside a quoted string
			IN_STRING,

			/// @brief Inside a number
			IN_NUMBER,

			/// @brief Inside a keyword, i.e. an unquoted string like `true`, `false` or `null`
			IN_KEYWORD,

			/// @brief Inside a comment
			IN_COMMENT,

			/// @brief Set in case of error
			ABORTED_DUE_TO_ERROR
		};

		/// @brief Current runtime state
		state_t m_state = state_t::IDLING;

		/// @brief Current runtime sub-state
		///
		/// This is used by subprocessing within strings and numbers
		enum class sub_state_t {
			/// @brief Normal condition, set when entering
			NORMAL,

			/// @brief A backslash has been matched: waiting for the escaped char
			IN_STRING_AFTER_BACKSLASH,

			/// @brief A \u sequence is begun; reading the hex characters
			///
			/// This state is completed by `m_escaped_char` and `m_escaped_char_remaining`
			IN_STRING_READING_HEX_CHARS,

			/// @brief In slash-star comment: matched the first /, do not consider the following *
			IN_COMMENT_SLASHSTAR_DISCARD_FIRST_STAR,

			/// @brief In slash-star comment: matched the first /*
			IN_COMMENT_SLASHSTAR_INSIDE,

			/// @brief In slash-star comment: matched a star: if a slash follows, end of the comment
			IN_COMMENT_SLASHSTAR_MATCHED_POTENTIAL_CLOSING_STAR,

			/// @brief Inside slash-slash comments; waiting for an end of line marker
			IN_COMMENT_SLASHSLASH,
		};

		/// @brief Current runtime sub-state
		sub_state_t m_sub_state = sub_state_t::NORMAL;

		/// @brief Numeric parser state
		uint8_t m_numeric_parser_state = 0;

		/// @brief EOF marker
		static constexpr char32_t CH32_EOF = UINT_LEAST32_MAX;

		/// @brief Buffered character
		CHARTYPE m_prev_char;

		/// @brief Last returned status on `process_char` or `process_eof`
		json_tokenizer_ret m_last_process_ret = json_tokenizer_ret::C_NEED_MORE_CHARS;

		/// @brief Process one character
		/// @param ch32_curr Character to be processed or EOF
		/// @param ch32_next Next character in the queue
		/// @return Returns the result of this step
		json_tokenizer_ret process_char_internal(char32_t ch32_curr, char32_t ch32_next);

		/// @brief Process one character inside a string (i.e., after the initial '"')
		/// @param ch32_curr Character to be processed or EOF
		/// @return Returns the result of this step
		json_tokenizer_ret process_char_string(char32_t ch32_curr);

		/// @brief Process one character inside a comment
		/// @param ch32_curr Character to be processed or EOF
		/// @return Returns the result of this step
		json_tokenizer_ret process_char_comment(char32_t ch32_curr);

		/// @brief Process one character of a keyword
		/// @param ch32_curr Character to be processed or EOF
		/// @param ch32_next Next character in the queue
		/// @return Returns the result of this step
		json_tokenizer_ret process_char_keyword(char32_t ch32_curr, char32_t ch32_next);

		/// @brief Process one character of a number
		/// @param ch32_curr Character to be processed or EOF
		/// @param ch32_next Next character in the queue
		/// @return Returns the result of this step
		json_tokenizer_ret process_char_number(char32_t ch32_curr, char32_t ch32_next);

		/// @brief Returns true if the character is valid for a keyword
		static bool is_valid_keyword_char(char32_t ch32) {return (((ch32 >= 'a') && (ch32 <= 'z')) || ((ch32 >= 'A') && (ch32 <= 'Z')) || ((ch32 >= '0') && (ch32 <= '9')) || (ch32 == '_') || (ch32 == '-') || (ch32 == '.'));}

		/// @brief Returns true if the character is valid for a keyword
		static bool is_valid_number_char(char32_t ch32) {return (((ch32 >= '0') && (ch32 <= '9')) || (ch32 == 'E') || (ch32 == 'e') || (ch32 == '+') || (ch32 == '-') || (ch32 == '.'));}

		/// @brief Clear all the public data
		void clear() {m_raw_token.clear(); m_string.clear(); m_multinum.clear(); m_string_can_converted_to_number=DASTD_ISSET(m_flags, NUMBERS_IN_STRINGS);}

		/// @brief Add a character to m_string when parsing string
		/// @param ch32 The character to be added
		///
		/// In case of `NUMBERS_IN_STRINGS` enabled, it checks if the character being added
		/// is 100% invalid for numbers and clears the `m_string_can_converted_to_number` flag.
		void add_char_to_string(char32_t ch32);

	protected:
		/// @brief Process one character
		/// @param ch Character to be processed
		/// @return Returns the result of this step
		///
		/// Note: if the method returns `C_NEED_MORE_CHARS` but there are no more
		/// characters, call `process_eof` to retrieve the conclusive data.
		json_tokenizer_ret internal_process_char(CHARTYPE ch);

		/// @brief Call when there are no more characters to feed
		/// @return Returns the result of this step
		json_tokenizer_ret internal_process_eof();

		/// @brief Set the first character before starting.
		///
		/// Used by derived classes that can not do it in the constructor
		void set_first_char(CHARTYPE first_char) {m_prev_char = first_char;}

	public:
		//--------------------------------------------------------
		// FLAGS
		//--------------------------------------------------------

		/// @brief Parse numbers in strings
		///
		/// If this flag is set, the parser will try to decode numbers in strings as well.
		/// For example, a string like "123" will be decoded as number 123 as well.
		/// In that case it will return `C_STRING_AND_NUMBER`.
		static constexpr uint32_t NUMBERS_IN_STRINGS = 1;

		//--------------------------------------------------------
		// UPDATE ACCESS SECTION
		//--------------------------------------------------------
		/// @brief Constructor
        /// @param first_char  Anticipate the first character (see CHARACTER FETCHING in the header)
		/// @param flags See the "FLAGS" section above
		json_tokenizer_base(CHARTYPE first_char, uint32_t flags): m_flags(flags), m_prev_char(first_char) {clear();}

		//--------------------------------------------------------
		// CONST ACCESS SECTION
		//--------------------------------------------------------

		/// @brief Last returned status on `process_char` or `process_eof`
		json_tokenizer_ret get_last_process_ret() const {return m_last_process_ret;}

		/// @brief Get the raw token parsed so far
		const RAWSTRING& get_raw_token() const {return m_raw_token;}

		/// @brief Get the string (valid only in case it returned C_STRING)
		const char32string& get_string() const {return m_string;}

		/// @brief Get the number value, valid in case of C_NUMBER
		const multinum& get_multinum() const {return m_multinum;}
};

/// @brief Implementation that operates with a single character
template<class CHARTYPE, class RAWSTRING = string_or_vector<CHARTYPE>>
class json_tokenizer: public json_tokenizer_base<CHARTYPE,RAWSTRING> {
	public:
		//--------------------------------------------------------
		// UPDATE ACCESS SECTION
		//--------------------------------------------------------
		/// @brief Constructor
		/// @param first_char  Anticipate the first character (see CHARACTER FETCHING in the header)
		/// @param flags See the "FLAGS" section above
		json_tokenizer(CHARTYPE first_char, uint32_t flags): json_tokenizer_base<CHARTYPE,RAWSTRING>(first_char, flags) {}

		/// @brief Process one character
		/// @param ch Character to be processed
		/// @return Returns the result of this step
		///
		/// Note: if the method returns `C_NEED_MORE_CHARS` but there are no more
		/// characters, call `process_eof` to retrieve the conclusive data.
		json_tokenizer_ret process_char(CHARTYPE ch) {return json_tokenizer_base<CHARTYPE,RAWSTRING>::internal_process_char(ch);}

		/// @brief Call when there are no more characters to feed
		/// @return Returns the result of this step
		json_tokenizer_ret process_eof() {return json_tokenizer_base<CHARTYPE,RAWSTRING>::internal_process_eof();}
};

/// @brief Implementation that automatically fetches data from a source
template<class CHARTYPE, class RAWSTRING = string_or_vector<CHARTYPE>>
class json_tokenizer_sourced: public json_tokenizer_base<CHARTYPE,RAWSTRING> {
	private:
		/// @brief Source used to fetch characters
		source_with_peek<CHARTYPE>& m_source;

	public:
		//--------------------------------------------------------
		// UPDATE ACCESS SECTION
		//--------------------------------------------------------
		/// @brief Constructor
		/// @param flags See the "FLAGS" section above
		json_tokenizer_sourced(source_with_peek<CHARTYPE>& source, uint32_t flags=0): json_tokenizer_base<CHARTYPE,RAWSTRING>((CHARTYPE)0, flags), m_source(source) 
		{
			CHARTYPE ch;
			if (source.tentative_peek_char(ch)) this->set_first_char(ch);
			else this->internal_process_eof();
		}

		/// @brief Extract the next token
		/// @return Returns the result of this step or `C_NOTHING_MORE` if there are no more tokens
		///
		/// Note: C_SPACEs are silently skipped
		json_tokenizer_ret fetch_token();
};


/// @brief Print the `json_tokenizer_ret` type
inline std::ostream& operator<<(std::ostream& o, json_tokenizer_ret e) {
	switch(e) {
		case json_tokenizer_ret::C_ERROR: o << "C_ERROR"; break;
		case json_tokenizer_ret::C_NOTHING_MORE: o << "C_NOTHING_MORE"; break;
		case json_tokenizer_ret::C_NEED_MORE_CHARS: o << "C_NEED_MORE_CHARS"; break;
		case json_tokenizer_ret::C_SPACE: o << "C_SPACE"; break;
		case json_tokenizer_ret::C_BRACE_OPEN: o << "C_BRACE_OPEN"; break;
		case json_tokenizer_ret::C_BRACE_CLOSE: o << "C_BRACE_CLOSE"; break;
		case json_tokenizer_ret::C_BRACKET_OPEN: o << "C_BRACKET_OPEN"; break;
		case json_tokenizer_ret::C_BRACKET_CLOSE: o << "C_BRACKET_CLOSE"; break;
		case json_tokenizer_ret::C_COMMA: o << "C_COMMA"; break;
		case json_tokenizer_ret::C_COLON: o << "C_COLON"; break;
		case json_tokenizer_ret::C_TRUE: o << "C_TRUE"; break;
		case json_tokenizer_ret::C_FALSE: o << "C_FALSE"; break;
		case json_tokenizer_ret::C_NULL: o << "C_NULL"; break;
		case json_tokenizer_ret::C_STRING: o << "C_STRING"; break;
		case json_tokenizer_ret::C_NUMBER: o << "C_NUMBER"; break;
		case json_tokenizer_ret::C_STRING_AND_NUMBER: o << "C_STRING_AND_NUMBER"; break;

		default: o << "UNKNOWN(" << (unsigned)e << ")";
	}
	return o;
}

//------------------------------------------------------------------------------
// (brief) Process one character
// (param) ch Character to be processed
// (return) Returns the result of this step
//
// Note: if the method returns `C_NEED_MORE_CHARS` but there are no more
// characters, call `process_eof` to retrieve the conclusive data.
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::internal_process_char(CHARTYPE ch)
{
	switch(m_state) {
		case state_t::REACHED_EOF: {
			m_last_process_ret = json_tokenizer_ret::C_NOTHING_MORE;
			break;
		}

		default: {
			if (m_state == state_t::IDLING) clear();
			char32_t ch32_curr = cast_to_unsigned<char32_t>(m_prev_char);
			char32_t ch32_next = cast_to_unsigned<char32_t>(ch);
			m_last_process_ret = process_char_internal(ch32_curr, ch32_next);

			m_raw_token.push_back(m_prev_char);
			m_prev_char = ch;
		}
	}
	return m_last_process_ret;
}

//------------------------------------------------------------------------------
// (brief) Call when there are no more characters to feed
// (return) Returns the result of this step
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::internal_process_eof()
{
	switch (m_state) {
		case state_t::REACHED_EOF: {
			m_last_process_ret = json_tokenizer_ret::C_NOTHING_MORE;
			break;
		}
		default: {
			if (m_state == state_t::IDLING) clear();
			char32_t ch32_curr = cast_to_unsigned<char32_t>(m_prev_char);
			m_last_process_ret = process_char_internal(ch32_curr, CH32_EOF);
			m_raw_token.push_back(m_prev_char);

			m_state = state_t::REACHED_EOF;
		}
	}
	return m_last_process_ret;
}

//------------------------------------------------------------------------------
// (brief) Process one character
// (param) ch32_curr Character to be processed or EOF
// (param) ch32_next Next character in the queue
// (return) Returns the result of this step
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::process_char_internal(char32_t ch32_curr, char32_t ch32_next)
{
	switch(m_state) {
		//-------------------------------------------------------------------
		// If we are in IDLING, we can identify the single characters
		// and the beginning of strings, numbers and keywords
		//-------------------------------------------------------------------
		case state_t::IDLING: {
			// The decoded string is always cleared when entering a new token
			m_string.clear();

			switch(ch32_curr) {
				// Single characters
				case '{': return json_tokenizer_ret::C_BRACE_OPEN;
				case '}': return json_tokenizer_ret::C_BRACE_CLOSE;
				case '[': return json_tokenizer_ret::C_BRACKET_OPEN;
				case ']': return json_tokenizer_ret::C_BRACKET_CLOSE;
				case ',': return json_tokenizer_ret::C_COMMA;
				case ':': return json_tokenizer_ret::C_COLON;

				// Spaces
				case ' ':
				case '\r':
				case '\n':
				case '\t': return json_tokenizer_ret::C_SPACE;

				// Detect comments
				case '#': {
					m_state = state_t::IN_COMMENT;
					m_sub_state = sub_state_t::IN_COMMENT_SLASHSLASH;
					return json_tokenizer_ret::C_NEED_MORE_CHARS;
				};

				// Detect comments
				case '/': {
					if (ch32_next == '*') {
						m_state = state_t::IN_COMMENT;
						m_sub_state = sub_state_t::IN_COMMENT_SLASHSTAR_DISCARD_FIRST_STAR;
						return json_tokenizer_ret::C_NEED_MORE_CHARS;
					}
					if (ch32_next == '/') {
						m_state = state_t::IN_COMMENT;
						m_sub_state = sub_state_t::IN_COMMENT_SLASHSLASH;
						return json_tokenizer_ret::C_NEED_MORE_CHARS;
					}

					// In the remaining cases, we issue an error
					m_state = state_t::ABORTED_DUE_TO_ERROR;
					return json_tokenizer_ret::C_ERROR;
				}

				// Detect the beginning of a string
				case '"': {
					m_state = state_t::IN_STRING;
					m_sub_state = sub_state_t::NORMAL;
					return json_tokenizer_ret::C_NEED_MORE_CHARS;
				}

				default: {
					// Detect the beginning of a number.
					// We accept a number 0-9 and the symbols -
					if (((ch32_curr >= '0') && (ch32_curr <= '9')) || (ch32_curr == '-')) {
						m_state = state_t::IN_NUMBER;
						m_sub_state = sub_state_t::NORMAL;
						m_numeric_parser_state = 0;
						return process_char_number(ch32_curr, ch32_next);
					}
					// Detect the beginning of a keyword. It must be a letter A-Z
					// We accept a number 0-9 and the symbols +, - and . (for example, .3)
					if (is_valid_keyword_char(ch32_curr)) {
						m_state = state_t::IN_KEYWORD;
						m_sub_state = sub_state_t::NORMAL;
						return process_char_keyword(ch32_curr, ch32_next);
					}

					// In the remaining cases, we issue an error
					m_state = state_t::ABORTED_DUE_TO_ERROR;
					return json_tokenizer_ret::C_ERROR;
				}
			}
		}

		//-------------------------------------------------------------------
		// Process strings
		//-------------------------------------------------------------------
		case state_t::IN_STRING: return process_char_string(ch32_curr);

		//-------------------------------------------------------------------
		// Process comments
		//-------------------------------------------------------------------
		case state_t::IN_COMMENT: return process_char_comment(ch32_curr);

		//-------------------------------------------------------------------
		// Process strings
		//-------------------------------------------------------------------
		case state_t::IN_KEYWORD: return process_char_keyword(ch32_curr, ch32_next);

		//-------------------------------------------------------------------
		// Process numbers
		//-------------------------------------------------------------------
		case state_t::IN_NUMBER: return process_char_number(ch32_curr, ch32_next);

		//-------------------------------------------------------------------
		// Already reached EOF before, there is nothing else it can do
		//-------------------------------------------------------------------
		case state_t::REACHED_EOF: return json_tokenizer_ret::C_NOTHING_MORE;

		//-------------------------------------------------------------------
		// Already aborted due to error, keep on reporting error
		//-------------------------------------------------------------------
		case state_t::ABORTED_DUE_TO_ERROR: return json_tokenizer_ret::C_ERROR;
	}

	assert(0);
	return json_tokenizer_ret::C_ERROR;
}



//------------------------------------------------------------------------------
// (brief) Add a character to m_string when parsing string
// (param) ch32 The character to be added
//
// In case of `NUMBERS_IN_STRINGS` enabled, it checks if the character being added
// is 100% invalid for numbers and sets the `m_string_can_converted_to_number` flag.
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
void json_tokenizer_base<CHARTYPE,RAWSTRING>::add_char_to_string(char32_t ch32)
{
	// Resolve UTF-16 surrogates
	if ((m_string.size() > 0) && (ch32 < 0x10000) && (detect_utf16_char((char16_t)ch32) == utf16_SECOND)) {
		char32_t prev = m_string[m_string.size()-1];
		if ((prev < 0x10000) && (detect_utf16_char((char16_t)prev) == utf16_FIRST)) {
			char16_t surrogates[3];
			surrogates[0] = (char16_t)prev;
			surrogates[1] = (char16_t)ch32;
			surrogates[2] = 0;
			char32_t codepoint;
			if (read_utf16_asciiz(surrogates, codepoint) == 2) {
				m_string[m_string.size()-1] = codepoint;
				return;
			}
		}
	}

	m_string.push_back(ch32);
	// If the string can still be converted to a number, let's see if it contains
	// some character that will prevent it.
	if (m_string_can_converted_to_number) {
		if (ch32 == ' ') return;
		if ((ch32 >= '0') && (ch32 <= '9')) return;
		if ((ch32 >= 'A') && (ch32 <= 'F')) return;
		if ((ch32 >= 'a') && (ch32 <= 'f')) return;
		if (ch32 == '.') return;
		if (ch32 == 'X') return;
		if (ch32 == 'X') return;
		if (ch32 == '+') return;
		if (ch32 == '-') return;
		m_string_can_converted_to_number = false;
	}
}

//------------------------------------------------------------------------------
// (brief) Process one character inside a string (i.e., after the initial '"')
// (param) ch32_curr Character to be processed or EOF
// (return) Returns the result of this step
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::process_char_string(char32_t ch32_curr)
{
	// The string is unterminated
	if (ch32_curr == CH32_EOF) {
		m_state = state_t::ABORTED_DUE_TO_ERROR;
		return json_tokenizer_ret::C_ERROR;
	}

	switch(m_sub_state) {
		// Normal condition, set when entering
		case sub_state_t::NORMAL: {
			switch (ch32_curr) {
				// Got closed quotes: the string is terminated
				case '"': {
					m_state = state_t::IDLING;
					if (m_string_can_converted_to_number) {
						m_multinum.parse(m_string);
						if (m_multinum.valid()) return json_tokenizer_ret::C_STRING_AND_NUMBER;
					}
					return json_tokenizer_ret::C_STRING;
				}
				// Got a backslash: start parsing the escapes
				case '\\': {
					m_sub_state = sub_state_t::IN_STRING_AFTER_BACKSLASH;
					return json_tokenizer_ret::C_NEED_MORE_CHARS;
				}
				// Got whatever else character: ok
				default: {
					add_char_to_string(ch32_curr);
					return json_tokenizer_ret::C_NEED_MORE_CHARS;
				}
			}
			break;
		}

		// A backslash has been matched: waiting for the escaped char
		case sub_state_t::IN_STRING_AFTER_BACKSLASH: {
			switch (ch32_curr) {
				// Usual escapes
				case 'b': add_char_to_string('\b'); break;
				case 'f': add_char_to_string('\f'); break;
				case 'n': add_char_to_string('\n'); break;
				case 'r': add_char_to_string('\r'); break;
				case 't': add_char_to_string('\t'); break;
				// Special case for \uNNNN
				case 'u': {
					m_escaped_char = 0;
					m_escaped_char_remaining = 4;
					m_sub_state = sub_state_t::IN_STRING_READING_HEX_CHARS;
					return json_tokenizer_ret::C_NEED_MORE_CHARS;
				}
				// By default we accept everything after the backslash
				default: add_char_to_string(ch32_curr);
			}
			m_sub_state = sub_state_t::NORMAL;
			return json_tokenizer_ret::C_NEED_MORE_CHARS;
		}

		// A "\u" sequence is begun; reading the hex characters
		// This state is completed by `m_escaped_char` and `m_escaped_char_remaining`
		case sub_state_t::IN_STRING_READING_HEX_CHARS: {
			assert(m_escaped_char_remaining > 0);
			char32_t digit;
			if ((ch32_curr >= '0') && (ch32_curr <= '9')) {
				digit = ch32_curr-'0';
			}
			else if ((ch32_curr >= 'A') && (ch32_curr <= 'F')) {
				digit = ch32_curr-'A'+10;
			}
			else if ((ch32_curr >= 'a') && (ch32_curr <= 'f')) {
				digit = ch32_curr-'a'+10;
			}
			else {
				m_state = state_t::ABORTED_DUE_TO_ERROR;
				return json_tokenizer_ret::C_ERROR;
			}
			m_escaped_char = (m_escaped_char << 4) | digit;
			m_escaped_char_remaining--;
			if (m_escaped_char_remaining == 0) {
				add_char_to_string(m_escaped_char);
				m_sub_state = sub_state_t::NORMAL;
			}
			return json_tokenizer_ret::C_NEED_MORE_CHARS;
		}

		// Other substates are not interesting for us
		default: assert(0);
	}

	// Should never get here
	assert(0);
	m_state = state_t::ABORTED_DUE_TO_ERROR;
	return json_tokenizer_ret::C_ERROR;
}


//------------------------------------------------------------------------------
// (brief) Process one character inside a comment
// (param) ch32_curr Character to be processed or EOF
// (return) Returns the result of this step
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::process_char_comment(char32_t ch32_curr)
{
	switch(m_sub_state) {
		// First entering a slash-star comment
		case sub_state_t::IN_COMMENT_SLASHSTAR_DISCARD_FIRST_STAR: {
			// The following character must be a star (it has been looked ahead)
			assert(ch32_curr == '*');
			m_sub_state = sub_state_t::IN_COMMENT_SLASHSTAR_INSIDE;
			return json_tokenizer_ret::C_NEED_MORE_CHARS;
		}
		// Inside a slash-star comment; discard everything except for *; EOF is not allowed.
		case sub_state_t::IN_COMMENT_SLASHSTAR_INSIDE: {
			// No EOF admitted inside a slash-star comment
			if (ch32_curr == CH32_EOF) {
				m_state = state_t::ABORTED_DUE_TO_ERROR;
				return json_tokenizer_ret::C_ERROR;
			}

			if (ch32_curr == '*') {
				m_sub_state = sub_state_t::IN_COMMENT_SLASHSTAR_MATCHED_POTENTIAL_CLOSING_STAR;
			}
			return json_tokenizer_ret::C_NEED_MORE_CHARS;
		}
		// Got a slash: it can be a false alarm or the end of a slash-star comment
		case sub_state_t::IN_COMMENT_SLASHSTAR_MATCHED_POTENTIAL_CLOSING_STAR: {
			// No EOF admitted inside a slash-star comment
			if (ch32_curr == CH32_EOF) {
				m_state = state_t::ABORTED_DUE_TO_ERROR;
				return json_tokenizer_ret::C_ERROR;
			}

			if (ch32_curr == '/') {
				m_sub_state = sub_state_t::NORMAL;
				m_state = state_t::IDLING;
				return json_tokenizer_ret::C_SPACE;
			}
			return json_tokenizer_ret::C_NEED_MORE_CHARS;
		}

		// Inside slash-slash comments; waiting for an end of line marker
		case sub_state_t::IN_COMMENT_SLASHSLASH: {
			switch (ch32_curr) {
				case '\r':
				case '\n':
				case CH32_EOF: {
					m_state = state_t::IDLING;
					return json_tokenizer_ret::C_SPACE;
				}
			}
			return json_tokenizer_ret::C_NEED_MORE_CHARS;
		}

		// Other cases are unexpected
		default: {
			assert(0);
			return json_tokenizer_ret::C_ERROR;
		}
	}
}

//------------------------------------------------------------------------------
// (brief) Process one character of a keyword
// (param) ch32_curr Character to be processed or EOF
// (param) ch32_next Next character in the queue
// (return) Returns the result of this step
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::process_char_keyword(char32_t ch32_curr, char32_t ch32_next)
{
	// The current character is expected to be valid. This is because:
	// - the first time this method is called is because the "process_char_internal" method
	//   found a valid keyword character.
	// - the following times this method is called is because this method has verified
	//   that ch32_next is valid.
	assert(is_valid_keyword_char(ch32_curr));

	m_string.push_back(ch32_curr);
	if (is_valid_keyword_char(ch32_next)) return json_tokenizer_ret::C_NEED_MORE_CHARS;

	m_state = state_t::IDLING;

	// If we get here is because we have a valid string
	if (m_string.compare_utf8("null") == 0) return json_tokenizer_ret::C_NULL;
	if (m_string.compare_utf8("true") == 0) return json_tokenizer_ret::C_TRUE;
	if (m_string.compare_utf8("false") == 0) return json_tokenizer_ret::C_FALSE;

	// This is not standard but useful when reading user written
	// JSON files.
	return json_tokenizer_ret::C_STRING;

	#if 0
	// Otherwise, it is an invalid string
	m_state = state_t::ABORTED_DUE_TO_ERROR;
	return json_tokenizer_ret::C_ERROR;
	#endif
}

//------------------------------------------------------------------------------
// Private function used by process_char_number. It converts a char32_t
// character into one of the following consecutive numeric codes:
// other  --> 0
// '-'    --> 1
// '+'    --> 2
// '.'    --> 3
// [Ee]   --> 4
// [0-9]  --> 5
//------------------------------------------------------------------------------
inline size_t json_tokenizer_numeric_ch(char32_t ch32)
{
	switch(ch32) {
		case '-': return 1;
		case '+': return 2;
		case '.': return 3;
		case 'e': return 4;
		case 'E': return 4;
		default: if ((ch32>='0') && (ch32<='9')) return 5;
	}
	return 0;
}

//------------------------------------------------------------------------------
// (brief) Process one character of a number
// (param) ch32_curr Character to be processed or EOF
// (param) ch32_next Next character in the queue
// (return) Returns the result of this step
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_base<CHARTYPE,RAWSTRING>::process_char_number(char32_t ch32_curr, char32_t ch32_next)
{
	/*
	INTERNAL DESIGN
	===============
	Numbers do not have a dedicated terminator like strings, that are known
	to terminate on the second double quote ("). Instead, numbers terminate
	when either the string ends or there is a character that can not be part
	of a number.
	This implies that in order to terminate a number, we must read and reject
	the first character that is not part of the number.
	For this reason, we have `ch32_curr` that contains the current character
	and `ch32_next` that peeks to the next one.

	By design in this method `ch32_curr` will always be a valid character:

	- the first time, this method is invoked by `process_char_internal` in the
	  IDLING state; that method will invoke this method only if the current
		character is either a '-' or a [0-9] number, i.e. a 100% valid character.

	- the following times, when m_state is IN_NUMBER, this method will terminate
	  as soon as the `ch32_next` appears either to be unexpected. If the number
		is consistent, it will return OK, otherwise it will return an error.

	*/
	constexpr size_t STATES_COUNT=8;
	constexpr size_t EVENTS_COUNT=6;

	size_t ev_curr = json_tokenizer_numeric_ch(ch32_curr);
	size_t ev_next = json_tokenizer_numeric_ch(ch32_next);

	assert((size_t)m_numeric_parser_state < STATES_COUNT);
	assert(ev_curr < EVENTS_COUNT);
	assert(ev_next < EVENTS_COUNT);

	// This map indicates, for each state, what state to move to according
	// to the various events. The event is at the position indicated in `json_tokenizer_numeric_ch`.
	// The state machine is depicted in ../../devdocs/json_tokenizer_numeric.jpg
	static constexpr uint8_t _ = UINT8_MAX;    // Not supported
	static constexpr uint8_t X = UINT8_MAX-1;  // Terminate
	static constexpr uint8_t state_map[STATES_COUNT][EVENTS_COUNT] = {
	             //  ?  -  +  .  E  0-9
		/* STATE 0 */ {_, 1, _, _, _, 2},
		/* STATE 1 */ {_, _, _, _, _, 2},
		/* STATE 2 */ {X, _, _, 3, 5, 2},
		/* STATE 3 */ {_, _, _, _, _, 4},
		/* STATE 4 */ {X, _, _, _, 5, 4},
		/* STATE 5 */ {_, 6, 6, _, _, 7},
		/* STATE 6 */ {_, _, _, _, _, 7},
		/* STATE 7 */ {X, _, _, _, _, 7},
	};

	// Find the next state according to the current character and the current state
	m_numeric_parser_state = state_map[m_numeric_parser_state][ev_curr];

	// By design, this code checks in advance if the current transaction will
	// fail or terminate. See 'INTERNAL DESIGN' at the beginnin of this function.
	assert(m_numeric_parser_state != _);
	assert(m_numeric_parser_state != X);

	// Now we know that the next state is valid. Add this character to the string.
	m_string.push_back(ch32_curr);

	// Get the evolution from the next state.
	// With this information we know in advance if the next transition
	// will fail and we can stop here. In this way
	assert(m_numeric_parser_state <= STATES_COUNT);
	uint8_t future_state = state_map[m_numeric_parser_state][ev_next];

	// If m_numeric_parser_state is '_' it means that the following character will not be
	// supported.
	if (future_state == _) {
		m_state = state_t::ABORTED_DUE_TO_ERROR;
		return json_tokenizer_ret::C_ERROR;
	}

	// If m_numeric_parser_state is 'X' it means that it can exit immediately with success
	if (future_state == X) {
		m_state = state_t::IDLING;
		m_multinum.parse(m_string);

		if (!m_multinum.valid()) {
			m_state = state_t::ABORTED_DUE_TO_ERROR;
			return json_tokenizer_ret::C_ERROR;
		}

		return json_tokenizer_ret::C_NUMBER;
	}

	return json_tokenizer_ret::C_NEED_MORE_CHARS;
}

//------------------------------------------------------------------------------
// (brief) Extract the next token
// (return) Returns the result of this step or `C_NOTHING_MORE` if there are no more tokens
//
// Note: C_SPACEs are silently skipped
//------------------------------------------------------------------------------
template<class CHARTYPE, class RAWSTRING>
json_tokenizer_ret json_tokenizer_sourced<CHARTYPE,RAWSTRING>::fetch_token()
{
	json_tokenizer_ret ret;
	for(;;) {
		CHARTYPE ch;
		//if (m_source.tentative_read_char(ch)) {
        m_source.tentative_read_char(ch);
        if (m_source.tentative_peek_char(ch)) {
			ret = json_tokenizer_base<CHARTYPE,RAWSTRING>::internal_process_char(ch);
		}
		else {
			ret = json_tokenizer_base<CHARTYPE,RAWSTRING>::internal_process_eof();
		}
		if ((ret != json_tokenizer_ret::C_NEED_MORE_CHARS) && (ret != json_tokenizer_ret::C_SPACE)) break;
	}
	return ret;
}


} // namespace dastd
