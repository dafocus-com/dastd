/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 01-MAY-2024
*
* JSON generic data representation
**/
#pragma once
#include "json_data.hpp"

namespace dastd {

enum class json_data_decoder_error_t {
	ERROR_OBJECT_EXPECTED_COMMA,      ///< decoding object, expected a comma");
	ERROR_OBJECT_EXPECTED_FIELD_NAME, ///< decoding object, expected a field name");
	ERROR_OBJECT_EXPECTED_COLON,      ///< decoding object, expected the colon ':' after the field name");;
	ERROR_ARRAY_EXPECTED_COMMA,       ///< decoding array, expected a comma");;
	ERROR_UNKNOWN_ELEMENT,            ///< unknown JSON element");
};

inline std::ostream& operator<<(std::ostream& o, json_data_decoder_error_t err) {
	using T = json_data_decoder_error_t;
	switch(err) {
		case T::ERROR_OBJECT_EXPECTED_COMMA:      o << "ecoding object, expected a comma"; break;
		case T::ERROR_OBJECT_EXPECTED_FIELD_NAME: o << "ecoding object, expected a field name"; break;
		case T::ERROR_OBJECT_EXPECTED_COLON:      o << "ecoding object, expected the colon ':' after the field name"; break;
		case T::ERROR_ARRAY_EXPECTED_COMMA:       o << "ecoding array, expected a comma"; break;
		case T::ERROR_UNKNOWN_ELEMENT:            o << "unknown JSON element"; break;
		default: o << "INVALID(" << (unsigned)err << ")";
	}
	return o;
}

/// @brief Implements a decoder that creates a `json_data` root instance
/// @tparam CHARTYPE 
template<class CHARTYPE>
class json_data_decoder {
	protected:
		/// @brief Tokenizer object
		json_tokenizer_sourced<CHARTYPE> m_tokenizer;

		/// @brief True if relaxed (i.e. ignores commas)
		bool m_relaxed;

		/// @brief Extract the next token
		/// @return Returns the result of this step or `C_NOTHING_MORE` if there are no more tokens
		///
		/// Note: C_SPACEs are silently skipped; C_COMMAs are silently skipped if `m_relaxed == true`
		json_tokenizer_ret fetch_token() {
			json_tokenizer_ret ret = m_tokenizer .fetch_token();
			if (m_relaxed) while (ret == json_tokenizer_ret::C_COMMA) ret = fetch_token();
			return ret;
		}


	public:
		/// @brief Constructor
		/// @param source Source that provides with the data
		json_data_decoder(json_tokenizer_source<CHARTYPE>& source, bool relaxed=true): m_tokenizer(source), m_relaxed(relaxed) {}

		/// @brief Destructor
		virtual ~json_data_decoder() {}

		/// @brief Executes the decoding of an unknonw new json entity
		/// @param fetch_new_token Set to `true` if the code has to start by fetching the new token or `false` if it has to use the currently fetched token
		/// @return Returns the instance of the root element or `nullptr` in case of failure
		std::shared_ptr<json_data> decode_unknown(bool fetch_new_token);

	public:
		/// @brief Reports an error
		/// @param err The error code
		virtual void report_error(json_data_decoder_error_t err);

	private:
		std::shared_ptr<json_data> decode_object();
		std::shared_ptr<json_data> decode_array();
};

//------------------------------------------------------------------------------
// Reports an error
//------------------------------------------------------------------------------
template<class CHARTYPE>
inline void json_data_decoder<CHARTYPE>::report_error(json_data_decoder_error_t err)
{
	std::cout << "Error: " << err << std::endl;
}

//------------------------------------------------------------------------------
// Executes the decoding of an object
//------------------------------------------------------------------------------
template<class CHARTYPE>
inline std::shared_ptr<json_data> json_data_decoder<CHARTYPE>::decode_object()
{
	bool first_time = true;
	json_tokenizer_ret tokenizer_ret;
	std::u32string field_name;

	std::shared_ptr<json_data_object> obj = json_data_object::make();

	for (;;) {
		// Read a token.
		// The first time, it must be either a field name (string) or a closed brace
		// The other times, it must be either a comma or a closed brace
		tokenizer_ret = fetch_token();

		if (tokenizer_ret == json_tokenizer_ret::C_BRACE_CLOSE) return obj;
		if (first_time) {
			first_time = false;
		}
		else {
			if (m_relaxed) {
				// Relaxed version: if the comma is missing, nevermind
				while (tokenizer_ret == json_tokenizer_ret::C_COMMA) tokenizer_ret = fetch_token();
			}
			else {
				if (tokenizer_ret != json_tokenizer_ret::C_COMMA) {report_error(json_data_decoder_error_t::ERROR_OBJECT_EXPECTED_COMMA); return nullptr;}
				tokenizer_ret = fetch_token();
			}
		}

		if (tokenizer_ret != json_tokenizer_ret::C_STRING) {report_error(json_data_decoder_error_t::ERROR_OBJECT_EXPECTED_FIELD_NAME); return nullptr;}

		field_name = m_tokenizer.get_string();

		// Read a token. It must be a colon
		tokenizer_ret = fetch_token();
		if (tokenizer_ret != json_tokenizer_ret::C_COLON) {report_error(json_data_decoder_error_t::ERROR_OBJECT_EXPECTED_COLON); return nullptr;}

		// Read the object field
		std::shared_ptr<json_data> field_body = decode_unknown(true);
		if (!field_body) return nullptr;

		// Add the field
		obj->add(field_name, field_body);
	}
}

//------------------------------------------------------------------------------
// Executes the decoding of an array
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decoder<CHARTYPE>::decode_array()
{
	bool first_time = true;
	json_tokenizer_ret tokenizer_ret;
	std::shared_ptr<json_data_array> obj = json_data_array::make();

	for (;;) {
		// Read a token. It must be either a comma or a closed bracket
		tokenizer_ret = fetch_token();

		if (tokenizer_ret == json_tokenizer_ret::C_BRACKET_CLOSE) return obj;
		if (first_time) {
			first_time = false;
		}
		else {
			if (m_relaxed) {
				// Relaxed version: if the comma is missing, nevermind
				if (tokenizer_ret == json_tokenizer_ret::C_COMMA) tokenizer_ret = fetch_token();
			}
			else {
				if (tokenizer_ret != json_tokenizer_ret::C_COMMA) {report_error(json_data_decoder_error_t::ERROR_ARRAY_EXPECTED_COMMA); return nullptr;}
				tokenizer_ret = fetch_token();
			}
		}
		// Read the object field
		std::shared_ptr<json_data> field_body = decode_unknown(false);
		if (!field_body) return nullptr;

		// Add the field
		obj->add(field_body);
	}
}

//------------------------------------------------------------------------------
// Executes the decoding of an unknonw new json entity
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decoder<CHARTYPE>::decode_unknown(bool fetch_new_token)
{
	json_tokenizer_ret tokenizer_ret;
	if (fetch_new_token) tokenizer_ret = fetch_token();
	else tokenizer_ret = m_tokenizer.get_last_process_ret();

	switch(tokenizer_ret) {
		// Start a new object
		case json_tokenizer_ret::C_BRACE_OPEN: return decode_object();

		// Start a new array
		case json_tokenizer_ret::C_BRACKET_OPEN: return decode_array();

		// Terminal: true
	  case json_tokenizer_ret::C_TRUE: return json_data_value_boolean::make(true);

		// Terminal: false
	  case json_tokenizer_ret::C_FALSE: return json_data_value_boolean::make(false);

		// Terminal: null
	  case json_tokenizer_ret::C_NULL: return json_data_value_null::make();

		// Terminal: string
	  case json_tokenizer_ret::C_STRING: return json_data_value_string::make(m_tokenizer.get_string());

		// Terminal: number
	  case json_tokenizer_ret::C_NUMBER: return json_data_value_number::make(m_tokenizer.get_multinum());

		// Terminal: string that can be consisdered a number
	  case json_tokenizer_ret::C_STRING_AND_NUMBER: return json_data_value_string::make(m_tokenizer.get_string());

		// Other cases: failure
		default: {report_error(json_data_decoder_error_t::ERROR_UNKNOWN_ELEMENT); return nullptr;}
	}
}


} // namespace dastd
