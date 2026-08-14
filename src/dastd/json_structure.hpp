/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 01-MAY-2024
*/
#pragma once
#include "json_tokenizer.hpp"
#include "fmt.hpp"
#include <cstdint>
#include <cassert>
#include <vector>

namespace dastd {

/// @brief Manages the structure of a valid JSON string allowing to skip subsections altogether
///
/// Validates the structure of the JSON file. This validation is only related at the correct
/// syntax and nesting, not at scheme level.
/// 
/// The structure supports strict and relaxed modes. In relaxed mode, commas are ignored altogether.
/// 
/// @anchor JsonSkip
/// ## Skipping JSON subtrees
/// This class helps in skipping a value or a subtree when the calle realizes that
/// it is not interested in that data. This happens, for example, in case of unknown
/// object fields are encountered.
/// 
/// The technique of skipping a subtree is done with the following steps:
/// 
/// 1. execute `auto marker = OBJ.get_marker()` before submitting the first token of the sequence that we
///    want to skip. The first token must be either a terminal value, an open brace `{` or an open bracket `[`.
/// 2. call the `OBJ.process_token(...)` method
/// 3. call `OBJ.is_completed(marker)`: if it returns `true`, the subtree has been skipped and the next token will be good
/// 4. go to 2
///  
class json_structure {
	public:
		/// @brief Return values
		enum class ret_t {
			OK,                                     ///< Processing ok
			RECEIVED_ERROR_TOKEN,                   ///< Relays the C_ERROR token
			NO_MORE_TOKENS_EXPECTED,                ///< The stack is empty thus the parsing of the JSON tree was completed
			UNEXPECTED_CASE,                        ///< Catastrophic error: one part of the code entered a section that should have been denied by design
			FAILED_EXPECT_NOTHING_MORE,             ///< It expected C_NOTHING_MORE because the tree is terminated
			FAILED_EXPECT_RVALUE,                   ///< It expected a r-value, i.e. a terminal value, a `[` or a `{`
			FAILED_EXPECT_COLON,                    ///< Right after a field name in an object, expected a colon `:`
			FAILED_EXPECT_STRING_OR_BRACE_CLOSE,    ///< It was inside an object and expected either a string, to begin a new field, or the closing brace `}`
			FAILED_EXPECT_COMMA_OR_BRACE_CLOSE,     ///< In strict mode, inside an object, it expected either a comma or the closing brace `}`
			FAILED_EXPECT_RVALUE_OR_BRACKET_CLOSE,  ///< Starting an array (or continuing in case of relaxed mode), it expected either the first value or a closed bracket `]` in case of an empty array
			FAILED_EXPECT_COMMA_OR_BRACKET_CLOSE,   ///< In strict mode, inside an array, it expected either a comma or a closed bracket `]`
		};

		/// @brief Type used for marking the position when skipping
		using marker_t = size_t;

		/// @brief Invalid value for markers
		static constexpr marker_t marker_invalid = 0;

	private:
		/// @brief Set to `true` if relaxed, i.e. if commas are ignored in JSON input
		bool m_relaxed = false;

		/// @brief State
		enum class state_t {
			EXPECT_NOTHING_MORE,
			STRICT_EXPECT_RVALUE,
			STRICT_EXPECT_STRING_OR_BRACE_CLOSE,
			STRICT_EXPECT_COLON,
			STRICT_EXPECT_COMMA_OR_BRACE_CLOSE,
			STRICT_EXPECT_RVALUE_OR_BRACKET_CLOSE,
			STRICT_EXPECT_COMMA_OR_BRACKET_CLOSE,
			RELAXED_EXPECT_RVALUE,
			RELAXED_EXPECT_STRING_OR_BRACE_CLOSE,
			RELAXED_EXPECT_COLON,
			RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE,
			ERROR
		};

		/// @brief Execution stack
		std::vector<state_t> m_stack;

		/// @brief Get the top entry
		/// @return Returns the top entry or `ERROR` if empty
		state_t top() const;

		/// @brief Push a new state on top of the stack
		/// @param state State to be pushed
		void push(state_t state);

		/// @brief Replace the state on top of the stack
		/// @param state State to be replaced
		void replace(state_t state);

		/// @brief Remove a state from the top of the stack
		void pop();

		/// @brief Submit a tokenizer token (strict mode)
		/// @param token The token coming from the tokenizer
		/// @param is_value Set to `true` if the token is one of the terminal values
		/// @param is_string Set to `true` if the token is a string that can be used as field name in objects
		/// @return Returns one of the `ret_t` return values
		ret_t process_token_strict(json_tokenizer_ret token, bool is_value, bool is_string);

		/// @brief Submit a tokenizer token (relaxed mode)
		/// @param token The token coming from the tokenizer
		/// @param is_value Set to `true` if the token is one of the terminal values
		/// @param is_string Set to `true` if the token is a string that can be used as field name in objects
		/// @return Returns one of the `ret_t` return values
		ret_t process_token_relaxed(json_tokenizer_ret token, bool is_value, bool is_string);

	public:
		/// @brief Start a new parsing sequence
		/// @param relaxed Set to `true` if relaxed, i.e. if commas are ignored in JSON input
		void start(bool relaxed=false);

		/// @brief Submit a tokenizer token
		/// @param token The token coming from the tokenizer
		/// @return Returns one of the `ret_t` return values
		ret_t process_token(json_tokenizer_ret token);

		/// @brief Get the starting marker. See 
		/// @return Rerturns a marker to be saved for later use
		/// 
		/// See the [skipping JSON subtrees](#JsonSkip) section for details.
		marker_t get_marker() const {return m_stack.size();}

		/// @brief Check if the subtree has been closed.
		/// @return Rerturns true if the subtree has been closed
		/// 
		/// See the [skipping JSON subtrees](#JsonSkip) section for details.
		bool is_completed(marker_t marker) const {return m_stack.size() <= marker;}
};

inline std::ostream& operator<<(std::ostream& o, json_structure::ret_t ret) 
{
	using R = json_structure::ret_t;
	switch(ret) {
		case R::OK																	 : o << "OK"; break;
		case R::RECEIVED_ERROR_TOKEN								 : o << "RECEIVED_ERROR_TOKEN"; break;
		case R::NO_MORE_TOKENS_EXPECTED							 : o << "NO_MORE_TOKENS_EXPECTED"; break;
		case R::UNEXPECTED_CASE											 : o << "UNEXPECTED_CASE"; break;
		case R::FAILED_EXPECT_NOTHING_MORE					 : o << "FAILED_EXPECT_NOTHING_MORE"; break;
		case R::FAILED_EXPECT_RVALUE								 : o << "FAILED_EXPECT_RVALUE"; break;
		case R::FAILED_EXPECT_COLON									 : o << "FAILED_EXPECT_COLON"; break;
		case R::FAILED_EXPECT_STRING_OR_BRACE_CLOSE	 : o << "FAILED_EXPECT_STRING_OR_BRACE_CLOSE"; break;
		case R::FAILED_EXPECT_COMMA_OR_BRACE_CLOSE	 : o << "FAILED_EXPECT_COMMA_OR_BRACE_CLOSE"; break;
		case R::FAILED_EXPECT_RVALUE_OR_BRACKET_CLOSE: o << "FAILED_EXPECT_RVALUE_OR_BRACKET_CLOSE"; break;
		case R::FAILED_EXPECT_COMMA_OR_BRACKET_CLOSE : o << "FAILED_EXPECT_COMMA_OR_BRACKET_CLOSE"; break;
		default: o << "UNKNOWN-0x" << fmt((unsigned)ret, 16, 8);
	}
	return o;
}

// Start a new parsing sequence
inline void json_structure::start(bool relaxed)
{
	m_relaxed = relaxed;
	m_stack.clear();

	using S = state_t;

	push(S::EXPECT_NOTHING_MORE);
	if (m_relaxed) {
		push(S::RELAXED_EXPECT_RVALUE);
	}
	else {
		push(S::STRICT_EXPECT_RVALUE);
	}
}

// Submit a tokenizer token
inline json_structure::ret_t json_structure::process_token(json_tokenizer_ret token)
{
	bool is_value = false;
	bool is_string = false;
	using T = json_tokenizer_ret;

	// In case of empty tokens, never complain
	if ((token == T::C_NEED_MORE_CHARS) || (token == T::C_SPACE)) return ret_t::OK;

	// Invoked with empty stack: too many entries
	if (m_stack.empty()) {
		return ret_t::NO_MORE_TOKENS_EXPECTED;
	}

	// Do the initial analysis of the token
	switch(token) {
		// A parsing error has occured; the string parsed so far is in 'm_raw_token'
		// In this case, we return error as well
		case T::C_ERROR: return ret_t::RECEIVED_ERROR_TOKEN;
	
		// In the input data there is nothing else
		case T::C_NOTHING_MORE: break;

		// Matched '{'
		case T::C_BRACE_OPEN: break;
		// Matched '}'
		case T::C_BRACE_CLOSE: break;
		// Matched '['
		case T::C_BRACKET_OPEN: break;
		// Matched ']'
		case T::C_BRACKET_CLOSE: break;

		// Matched ','
		// Discard if relaxed
		case T::C_COMMA: {
			if (m_relaxed) return ret_t::OK;
			break;
		}

		// Matched ':'
		case T::C_COLON: break;
		// Matched 'true'
		case T::C_TRUE: is_value=true; break;
		// Matched 'false'
		case T::C_FALSE: is_value=true; break;
		// Matched 'null'
		case T::C_NULL: is_value=true; break;
		// Matched a string
		case T::C_STRING: is_value=true; is_string=true; break;

		// Matched a number, `get_multinum()` to retrieve the value
		case T::C_NUMBER: is_value=true; break;
		// Matched a string that contains a valid number
		// Returned only if flag `NUMBERS_IN_STRINGS` is set.
		case T::C_STRING_AND_NUMBER: is_value=true; is_string=true; break;

		// Unexpected case: always a failure
		default: assert(0); return ret_t::UNEXPECTED_CASE;
	}

	size_t prev = m_stack.size();

	ret_t ret;
	if (m_relaxed) ret = process_token_relaxed(token, is_value, is_string);
	else ret = process_token_strict(token, is_value, is_string);

	std::cout << "[DEBUG] Stack: " << fmt(prev, 10, 3) << " -> " << fmt(m_stack.size(), 10, 3) << " Token: " << token << std::endl;

	return ret;
}

/// @brief Get the top entry
/// @return Returns the top entry or `ERROR` if empty
inline json_structure::state_t json_structure::top() const
{
	if (m_stack.empty()) return json_structure::state_t::ERROR;
	return m_stack.back();
}

/// @brief Push a new state on top of the stack
/// @param state State to be pushed
inline void json_structure::push(state_t state)
{
	m_stack.push_back(state);
}

/// @brief Replace the state on top of the stack
/// @param state State to be replaced
inline void json_structure::replace(state_t state)
{
	assert(!m_stack.empty());
	m_stack.back() = state;
}

/// @brief Remove a state from the top of the stack
inline void json_structure::pop()
{
	assert(!m_stack.empty());
	m_stack.pop_back();
}


// Submit a tokenizer token (strict mode)
inline json_structure::ret_t json_structure::process_token_strict(json_tokenizer_ret token, bool is_value, bool is_string)
{
	using S = state_t;
	using T = json_tokenizer_ret;
	S state = top();

	switch(state) {
		case S::EXPECT_NOTHING_MORE: {
			if (token == T::C_NOTHING_MORE) pop();
			else return ret_t::FAILED_EXPECT_NOTHING_MORE;
			break;
		}

		case S::STRICT_EXPECT_RVALUE: {
			if (is_value) pop();
			else if (token == T::C_BRACE_OPEN) replace(S::STRICT_EXPECT_STRING_OR_BRACE_CLOSE);
			else if (token == T::C_BRACKET_OPEN) replace(S::STRICT_EXPECT_RVALUE_OR_BRACKET_CLOSE);
			else return ret_t::FAILED_EXPECT_RVALUE;
			break;
		}

		case S::STRICT_EXPECT_STRING_OR_BRACE_CLOSE: {
			if (is_string) replace(S::STRICT_EXPECT_COLON);
			else if (token == T::C_BRACE_CLOSE) pop();
			else return ret_t::FAILED_EXPECT_STRING_OR_BRACE_CLOSE;
			break;
		}

		case S::STRICT_EXPECT_COLON: {
			if (token == T::C_COLON) {replace(S::STRICT_EXPECT_COMMA_OR_BRACE_CLOSE); push(S::STRICT_EXPECT_RVALUE);}
			else return ret_t::FAILED_EXPECT_COLON;
			break;
		}

		case S::STRICT_EXPECT_COMMA_OR_BRACE_CLOSE: {
			if (token == T::C_COMMA) replace(S::STRICT_EXPECT_STRING_OR_BRACE_CLOSE);
			else if (token == T::C_BRACE_CLOSE) pop();
			else return ret_t::FAILED_EXPECT_COMMA_OR_BRACE_CLOSE;
			break;
		}

		case S::STRICT_EXPECT_RVALUE_OR_BRACKET_CLOSE: {
			if (is_value) replace(S::STRICT_EXPECT_COMMA_OR_BRACKET_CLOSE);
			else if (token == T::C_BRACE_OPEN) {replace(S::STRICT_EXPECT_COMMA_OR_BRACKET_CLOSE); push(S::STRICT_EXPECT_STRING_OR_BRACE_CLOSE);}
			else if (token == T::C_BRACKET_OPEN) {replace(S::STRICT_EXPECT_COMMA_OR_BRACKET_CLOSE); push(S::STRICT_EXPECT_RVALUE_OR_BRACKET_CLOSE);}
			else if (token == T::C_BRACKET_CLOSE) pop();
			else return ret_t::FAILED_EXPECT_RVALUE_OR_BRACKET_CLOSE;
			break;
		}

		case S::STRICT_EXPECT_COMMA_OR_BRACKET_CLOSE: {
			if (token == T::C_COMMA) replace(S::STRICT_EXPECT_RVALUE_OR_BRACKET_CLOSE);
			else if (token == T::C_BRACKET_CLOSE) pop();
			else return ret_t::FAILED_EXPECT_COMMA_OR_BRACKET_CLOSE;
			break;
		}

		// State unexpected by design
		default: assert(0); return ret_t::UNEXPECTED_CASE;
	}

	return ret_t::OK;
}

// Submit a tokenizer token (relaxed mode)
inline json_structure::ret_t json_structure::process_token_relaxed(json_tokenizer_ret token, bool is_value, bool is_string)
{
	using S = state_t;
	using T = json_tokenizer_ret;
	S state = top();
	
	switch(state) {
		case S::EXPECT_NOTHING_MORE: {
			if (token == T::C_NOTHING_MORE) pop();
			else return ret_t::FAILED_EXPECT_NOTHING_MORE;
			break;
		}

		case S::RELAXED_EXPECT_RVALUE: {
			if (is_value) pop();
			else if (token == T::C_BRACE_OPEN) replace(S::RELAXED_EXPECT_STRING_OR_BRACE_CLOSE);
			else if (token == T::C_BRACKET_OPEN) replace(S::RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE);
			else return ret_t::FAILED_EXPECT_RVALUE;
			break;
		}

		case S::RELAXED_EXPECT_STRING_OR_BRACE_CLOSE: {
			if (is_string) replace(S::RELAXED_EXPECT_COLON);
			else if (token == T::C_BRACE_CLOSE) pop();
			else return ret_t::FAILED_EXPECT_COMMA_OR_BRACE_CLOSE;
			break;
		}

		case S::RELAXED_EXPECT_COLON: {
			if (token == T::C_COLON) {replace(S::RELAXED_EXPECT_STRING_OR_BRACE_CLOSE); push(S::RELAXED_EXPECT_RVALUE);}
			else return ret_t::FAILED_EXPECT_COLON;
			break;
		}

		case S::RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE: {
			if (is_value) replace(S::RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE);
			else if (token == T::C_BRACE_OPEN) {replace(S::RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE); push(S::RELAXED_EXPECT_STRING_OR_BRACE_CLOSE);}
			else if (token == T::C_BRACKET_OPEN) {replace(S::RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE); push(S::RELAXED_EXPECT_RVALUE_OR_BRACKET_CLOSE);}
			else if (token == T::C_BRACKET_CLOSE) pop();
			else return ret_t::FAILED_EXPECT_RVALUE_OR_BRACKET_CLOSE;
			break;
		}

		// State unexpected by design
		default: assert(0); return ret_t::UNEXPECTED_CASE;
	}
	
	return ret_t::OK;
}

} // namespace dastd
