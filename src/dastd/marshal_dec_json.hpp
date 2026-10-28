/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#pragma once
#include "marshal_dec.hpp"
#include "marshal_json.hpp"
#include "json_tokenizer.hpp"
#include "fmt_string.hpp"
#include "source.hpp"
#include "base64.hpp"
#include "ostream_string.hpp"
#include <stack>
#include <map>

namespace dastd {

/// @brief Binary little-endian marshaling decoder
///
/// Decodes binary data using JSON.
///
/// @tparam CHARTYPE     Type of the character handled; usually `char`
///                      or `char32_t` but it can be any type compatible
///                      with `CHARTYPE` of `json_tokenizer`.
///
/// @tparam DECOPRINTER  Type used to format a `string_or_vector<CHARTYPE, DECOPRINTER>`
///                      when printed with `<<` on a `std::ostream`, usually
///                      to report errors.
/// 
/// 
/// Example:
/// 
///         std::string jsonText = "...json...";
///         dastd::source_string_or_vector<char> jsonTextSource(jsonText);
///         dastd::marshal_dec_json<char> dec(jsonTextSource);
template<class CHARTYPE, class DECOPRINTER=fmt_string<CHARTYPE,fmt_string_f::C11_ESCAPED_QUOTED>>
class marshal_dec_json: public marshal_dec {
	private:
		/// @brief Stack element
		struct stack_element {
			/// @brief Type of the element on the stack
			marshal_json_element_type m_element_type;

			/// @brief Remembers the number of items already encoded for arrays and structures
			size_t m_items_count = 0;

			/// @brief Constructor
			stack_element(marshal_json_element_type element_type): m_element_type(element_type) {}

			/// @brief Constructor
			stack_element() {}
		};

		/// @brief Internal encoding stack
		std::stack<stack_element> m_stack;

		/// @brief Map mapping the decoded label id with the field name (for error reporting)
		std::map<marshal_label_id_t, dastd::char32string> m_fields_map;

		/// @brief Internal JSON decoder
		json_tokenizer_sourced<CHARTYPE> m_tokenizer;

		/// @brief Set to true if the previous decoding is to be submitted again
		bool m_resubmit_prev_token = false;

		/// @brief See @ref marshal_json_polymorphic_encoding
		marshal_json_polymorphic_encoding m_polymorphic_encoding;

		/// @brief Name of the field in case of `TYPEID_AS_STRUCT_FIELD`; see @ref marshal_json_polymorphic_encoding
		std::string m_typed_field;

		/// @brief Set to true if the decoder decoded a type structure in case of `TYPEID_AS_STRUCT_FIELD`
		bool m_is_typed = false;

		/// @brief Enables "relaxed" decoding.
		bool m_relaxed = true;

		/// @brief Request to resubmit the previous token
		///
		/// The previous token can be resubmitted only once
		void resubmit_prev_token() {assert(!m_resubmit_prev_token); m_resubmit_prev_token=true;}

		/// @brief Parse the string until a new token has been detected
		void fetch_token();

	  /// @brief Skip the entire substructure
	  ///
	  /// This function expects to be right after a "{" or "[" and skips the
	  /// entire substructure until matched the corresponding "}" or "]".
	  /// It stops at the first unbalanced "}" or "]" which is left in the buffer.
	  void skip_substructure();

	public:
		/// @brief Constructor
		/// @param input_string Contains the JSON to be decoded in UNICODE-32 form. The string must
		///                     remain available and unchanged until the parsing has terminated.
		/// @param polymorphic_encoding   See @ref marshal_json_polymorphic_encoding
		/// @param typed_field            Name of the field in case of `TYPEID_AS_STRUCT_FIELD`; see @ref marshal_json_polymorphic_encoding
		marshal_dec_json(json_tokenizer_source<CHARTYPE>& source, marshal_json_polymorphic_encoding polymorphic_encoding=marshal_json_polymorphic_encoding::TYPEID_AS_FIELD_NAME, std::string_view typed_field="$type"):
			m_tokenizer(source), m_polymorphic_encoding(polymorphic_encoding), m_typed_field(typed_field) {}

		/// @brief Enables or disables "relaxed" decoding.
		///
		/// Enabled by default. With `relaxed` decoding, commas are ignored.
		void set_relaxed(bool relaxed) {m_relaxed = relaxed;}

		/// @brief Decode a bool
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual bool decode_bool(uint32_t suggestions=0) override;

		/// @brief Decode a uint8_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual uint8_t decode_u8(uint32_t suggestions=0) override;

		/// @brief Decode a int8_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual int8_t decode_i8(uint32_t suggestions=0) override;

		/// @brief Decode a uint16_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual uint16_t decode_u16(uint32_t suggestions=0) override;

		/// @brief Decode a int16_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual int16_t decode_i16(uint32_t suggestions=0) override;

		/// @brief Decode a uint32_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual uint32_t decode_u32(uint32_t suggestions=0) override;

		/// @brief Decode a int32_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual int32_t decode_i32(uint32_t suggestions=0) override;

		/// @brief Decode a uint64_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual uint64_t decode_u64(uint32_t suggestions=0) override;

		/// @brief Decode a int64_t
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual int64_t decode_i64(uint32_t suggestions=0) override;

		/// @brief Decode a 64-bit floating point
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual double decode_f64(uint32_t suggestions=0) override;

		/// @brief Decode a 128-bit UUID
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual uuid decode_uuid(uint32_t suggestions=0) override;

		/// @brief Decode a std::string (UTF-8)
		/// @param value Receives the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void decode_string_utf8(std::string& value, uint32_t suggestions=0) override;

		/// @brief Decode a std::u32string
		/// @param value Receives the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void decode_u32string(std::u32string& value, uint32_t suggestions=0) override;

		/// @brief Start Decoding a structure
		/// @param extensible If true, the structure will be encoded so more or less fields can be expected.
		/// @param field_infos Array with the label-ids of the expected fields in their correct order and a marker telling if that field is optional.
		/// @param field_infos_count Number of elements to be expected in the field_ids array.
		///
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void decode_struct_begin(bool extensible, const marshal_label_info_t* field_infos, size_t fields_infos_count) override;

		/// @brief Terminate Decoding a structure
		///
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void decode_struct_end() override;

		/// @brief Start decoding a field within a structure
		///
		/// This must be invoked inside a `decode_struct_begin` and `decode_struct_end` pair.
		///
		/// It extracts a field from the structure and returns its label id. Depending on the
		/// encoding format, the fields can be out of order, dupe, missing or unknown.
		/// If the encoding type allows dupe or unknown fields, the `decode_struct_field_skip()`
		/// method will be available for skipping the unwanted fields.
		///
		/// See @link marshaling_structs documentation @endlink for details and examples.
		///
		/// @param optional_present This flag is set to `true` if the field is present, or `false`
		///                         if missing.
		///                         A field can be missing only if optional, i.e. encoded with `OPTIONAL_MISSING`
		///                         or `OPTIONAL_PRESENT`.
		///                         An optional field must be declared in the `field_infos` array
		///                         passed to `decode_struct_begin`.
		///
		/// @return Returns the label id of the field that has been detected or `marshal_label_id_INVALID`
		///         if no more fields are available.
		virtual marshal_label_id_t decode_struct_field_begin(bool* optional_present=nullptr) override;

		/// @brief Terminate decoding a field within a structure
		///
		/// This must be invoked inside a `decode_struct_begin` and `decode_struct_end` pair.
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void decode_struct_field_end() override;

		/// @brief Decodes and discard an unknown field
		/// @return Returns `true` if skipped, `false` if this feature is not available
		virtual bool decode_struct_field_skip() override;

		/// @brief Start decoding an array
		/// @param count Number of elements that will be decoded
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		///
		/// @return Returns the expected number of elements or `marshal_array_SIZE_UNKNOWN`
		///         if the size is unknown.
		virtual size_t decode_array_begin() override;

		/// @brief Terminate decoding an array
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		virtual void decode_array_end() override;

		/// @brief Start decoding an array element
		/// @param count Number of elements that will be decoded
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		///
		/// @return Returns `true` if the element is available or `false` if
		///         no more elements are available.
		virtual bool decode_array_element_begin() override;

		/// @brief Terminate decoding an array element
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		virtual void decode_array_element_end() override;

		/// @brief Start decoding a dictionary
		/// @param count Number of elements that will be decoded
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		///
		/// @return Returns the expected number of elements or `marshal_array_SIZE_UNKNOWN`
		///         if the size is unknown.
		virtual size_t decode_dictionary_begin() override;

		/// @brief Terminate decoding a dictionary
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		virtual void decode_dictionary_end() override;

		/// @brief Start decoding a dictionary element
		/// @param key UTF-8 encoded string key that has been encoded along with the object
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		///
		/// @return Returns `true` if the element is available or `false` if
		///         no more elements are available.
		virtual bool decode_dictionary_element_begin(std::string& key) override;

		/// @brief Terminate decoding a dictionary element
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		virtual void decode_dictionary_element_end() override;

		/// @brief Start decoding a typed object
		/// @param extensible If true, the object is decoded in way that allows skipping it during decoding.
		///
		/// See @link marshaling_typeds documentation @endlink for details and examples.
		///
		/// @return Returns the label-id of the type of the object that is following
		virtual marshal_label_id_t decode_typed_begin(bool extensible) override;

		/// @brief Skip the object without decoding it
		///
		/// This method will skip the object without decoding. It can be invoked only if
		/// the `extensible` parameter is `true`. Useful when the encoding data comes from
		/// a newer version that has new object types unknown to this version.
		///
		/// See @link marshaling_typeds documentation @endlink for details and examples.
		virtual void decode_typed_end_skip() override;

		/// @brief Terminate decoding a typed object
		///
		/// See @link marshaling_typeds documentation @endlink for details and examples.
		virtual void decode_typed_end() override;

		/// @brief Attempt to translate a label_id into a text
		/// @param label_id Label id to be translated
		/// @param label_text Related text
		/// @return Returns true if the translation is available
		virtual bool get_field_name(marshal_label_id_t label_id, char32string& label_text) const override;

	protected:
		/// @brief Decode raw binary data of a known and fixed length
		/// @param buffer Receives the raw decoded data
		/// @param length Number of bytes expected
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void internal_decode_binary(void* buffer, size_t length, uint32_t suggestions=0) override;

		/// @brief Decode raw binary data of a variable length
		/// @param value Receives the raw decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void internal_decode_varsize_binary(std::string& value, uint32_t suggestions=0) override;
};


// Attempt to translate a label_id into a text
template<class CHARTYPE, class DECOPRINTER>
inline bool marshal_dec_json<CHARTYPE, DECOPRINTER>::get_field_name(marshal_label_id_t label_id, char32string& label_text) const
{
	auto iter = m_fields_map.find(label_id);
	if (iter != m_fields_map.end()) {
		label_text = iter->second;
		return true;
	}
	return false;
}

// Parse the string until a new token has been detected
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::fetch_token()
{
	if (m_resubmit_prev_token) {m_resubmit_prev_token=false; return;}
	for (;;) {
		m_tokenizer.fetch_token();
		// If relaxed, commas are ignored
		if (!m_relaxed || (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COMMA)) break;
	}
}

// Decode a bool
template<class CHARTYPE, class DECOPRINTER>
inline bool marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_bool(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	switch(m_tokenizer.get_last_process_ret()) {
		case json_tokenizer_ret::C_TRUE: return true;
		case json_tokenizer_ret::C_FALSE: return false;
		default: DASTD_THROW(exception_marshal, "marshal_dec_json::decode_bool: expected 'true' or 'false', got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	}
}

// Decode a int8_t
template<class CHARTYPE, class DECOPRINTER>
inline int8_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_i8(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<int8_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_i8: expected i8, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a uint8_t
template<class CHARTYPE, class DECOPRINTER>
inline uint8_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_u8(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<uint8_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_u8: expected u8, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a int16_t
template<class CHARTYPE, class DECOPRINTER>
inline int16_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_i16(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<int16_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_i16: expected i16, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a uint16_t
template<class CHARTYPE, class DECOPRINTER>
inline uint16_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_u16(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<uint16_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_u16: expected u16, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a int32_t
template<class CHARTYPE, class DECOPRINTER>
inline int32_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_i32(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<int32_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_i32: expected i32, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a uint32_t
template<class CHARTYPE, class DECOPRINTER>
inline uint32_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_u32(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<uint32_t>();
	if (!pair.second){
		DASTD_THROW(exception_marshal, "marshal_dec_json::decode_u32: expected u32, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	}
	return pair.first;
}

// Decode a int64_t
template<class CHARTYPE, class DECOPRINTER>
inline int64_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_i64(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<int64_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_i64: expected i64, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a uint64_t
template<class CHARTYPE, class DECOPRINTER>
inline uint64_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_u64(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<uint64_t>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_u64: expected u64, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a 64-bit floating point
template<class CHARTYPE, class DECOPRINTER>
inline double marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_f64(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	auto pair = m_tokenizer.get_multinum().template get<double>();
	if (!pair.second) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_f64: expected f64, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	return pair.first;
}

// Decode a 128-bit UUID
template<class CHARTYPE, class DECOPRINTER>
inline uuid marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_uuid(uint32_t suggestions)
{
	std::string value;
	decode_string_utf8(value, suggestions);
	uuid v_uuid;
    if (!value.empty()) {
	    if (!v_uuid.parse(value)) {
		    DASTD_THROW(exception_marshal, "marshal_dec_json::decode_uuid: expected UUID, got " << DECOPRINTER(m_tokenizer.get_raw_token()));
	    }
    }
	return v_uuid;
}

// Decode a std::string (UTF-8)
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_string_utf8(std::string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_string_utf8: expected a string, got result " << m_tokenizer.get_last_process_ret());
	value = m_tokenizer.get_string().get_utf8();
}

// Decode raw binary data of a known and fixed length
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::internal_decode_binary(void* buffer, size_t length, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_binary: expected a string, got result " << m_tokenizer.get_last_process_ret());
	std::istringstream in(m_tokenizer.get_string().get_utf8());
	std::ostringstream out;
	if (!base64_decode(in, out)) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_binary: invalid base-64 sequence");
	if (out.str().size() != length) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_binary: expected a base-64 binary sequence of " << length << " bytes, decoded " << out.str().size());
	memcpy(buffer, out.str().c_str(), length);
}

// Decode raw binary data of a variable length
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::internal_decode_varsize_binary(std::string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) DASTD_THROW(exception_marshal, "marshal_dec_json::internal_decode_varsize_binary: expected a string, got result " << m_tokenizer.get_last_process_ret());
	std::istringstream in(m_tokenizer.get_string().get_utf8());
	ostream_string_ref out(value);
	if (!base64_decode(in, out)) DASTD_THROW(exception_marshal, "marshal_dec_json::internal_decode_varsize_binary: invalid base-64 sequence");
}

// Decode a std::u32string
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_u32string(std::u32string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_u32string: expected a string, got result " << m_tokenizer.get_last_process_ret());
	value = m_tokenizer.get_string();
}

// Start Decoding a structure
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_struct_begin(bool extensible, const marshal_label_info_t* field_infos, size_t fields_infos_count)
{
	DASTD_NOWARN_UNUSED(extensible);
	DASTD_NOWARN_UNUSED(field_infos);
	DASTD_NOWARN_UNUSED(fields_infos_count);

	// If `m_is_typed` is set, it means that the structure has been already opened and partially
	// parsed by the `decode_typed_begin` call to retrieve the internal field that contains
	// the type name.
	if (!m_is_typed) {
		if (!m_stack.empty()) {
			if (m_stack.top().m_element_type == marshal_json_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_struct_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
			if (m_stack.top().m_element_type == marshal_json_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_struct_begin inside an ARRAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
			if (m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_struct_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		}

		fetch_token();
		if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACE_OPEN) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_struct_begin: expected '{' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

		// marshal_json_element_type element_type, bool extensible, size_t element_offset, size_t element_size, const marshal_label_id_t* field_ids, size_t fields_count
		m_stack.emplace(marshal_json_element_type::STRUCT);
	}
	else m_is_typed=false;
}

// Terminate Decoding a structure
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_struct_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_struct_end without being inside a STRUCT (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_struct_end without being inside a STRUCT but inside a " << cur_stack.m_element_type);

	// Skip the remaining substructure (with unexpected fields)
	skip_substructure();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACE_CLOSE) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_struct_end: expected '}' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	m_stack.pop();
}

// Start decoding a field within a structure
template<class CHARTYPE, class DECOPRINTER>
marshal_label_id_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_struct_field_begin(bool* optional_present)
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_begin without being inside a struct (stack empty)");
	if (m_stack.top().m_element_type != marshal_json_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_begin without being inside a STRUCT but inside a " << m_stack.top().m_element_type);

	stack_element& cur_stack = m_stack.top();

	// Start decoding. It could be a '}', which means that no more fields are following
	fetch_token();
	if (m_tokenizer.get_last_process_ret() == json_tokenizer_ret::C_BRACE_CLOSE) {
		resubmit_prev_token();
		return marshal_label_id_INVALID;
	}

	// If this is not the first field, there must be a comma
	if (!m_relaxed) {
		if (cur_stack.m_items_count > 0) {
			if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COMMA) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_struct_field_begin: expected ',' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
			fetch_token();
		}
	}
	cur_stack.m_items_count++;

	// The current element must be the field name
	if ((m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) || (m_tokenizer.get_string().length() == 0)) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_struct_field_begin: expected field name but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
	marshal_label_id_t label_id = marshal_label::hash(m_tokenizer.get_string().get_utf8());

	// Save the label text in case it will be needed
	if (!m_fields_map.contains(label_id)) {
		m_fields_map.insert({label_id, m_tokenizer.get_string()});
	}

	// The following field must be a ":"
	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COLON) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_struct_field_begin: expected ':' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	m_stack.emplace(marshal_json_element_type::FIELD);

	// If the field is optional, look ahead to catch a "null"
	if (optional_present) {
		fetch_token();
		if (m_tokenizer.get_last_process_ret() == json_tokenizer_ret::C_NULL) {
			(*optional_present) = false;
		}
		else {
			(*optional_present) = true;
			resubmit_prev_token();
		}
	}

	return label_id;
}

// Terminate decoding a field within a structure
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_struct_field_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end without being inside a FIELD (stack empty)");
	if (m_stack.top().m_element_type != marshal_json_element_type::FIELD) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end without being inside a FIELD but inside a " << m_stack.top().m_element_type);

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::FIELD) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end without being inside a FIELD but inside a " << cur_stack.m_element_type);

	m_stack.pop();
}

// Terminate decoding a field within a structure
template<class CHARTYPE, class DECOPRINTER>
inline bool marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_struct_field_skip()
{
	fetch_token();
	switch(m_tokenizer.get_last_process_ret()) {
		case json_tokenizer_ret::C_BRACKET_OPEN: 
		case json_tokenizer_ret::C_BRACE_OPEN: skip_substructure(); break;
		default:;
	}
	
	return true;
}

// Start decoding an array
template<class CHARTYPE, class DECOPRINTER>
inline size_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_array_begin()
{
	assert(!m_is_typed);
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_json_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_array_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_json_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_array_begin inside an ARRAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_array_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACKET_OPEN) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_array_begin: expected '[' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	m_stack.emplace(marshal_json_element_type::ARRAY);

	return dastd::marshal_array_SIZE_UNKNOWN;
}

// Terminate decoding an array
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_array_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_array_end without being inside a ARRAY (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_array_end without being inside a ARRAY but inside a " << cur_stack.m_element_type);

	m_stack.pop();
}

// Start decoding an array element
template<class CHARTYPE, class DECOPRINTER>
inline bool marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_array_element_begin()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_array_element_begin without being inside a ARRAY (stack empty)");

	stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_array_element_begin without being inside a ARRAY but inside a " << cur_stack.m_element_type);

	// Start decoding. It could be a ']', which means that no more fields are following
	fetch_token();
	if (m_tokenizer.get_last_process_ret() == json_tokenizer_ret::C_BRACKET_CLOSE) {
		return false;
	}

	// If this is not the first field, there must be a comma
	if (!m_relaxed) {
		if (cur_stack.m_items_count > 0) {
			if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COMMA) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_array_element_begin: expected ',' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
		}
		else {
			resubmit_prev_token();
		}
	}
	else {
		resubmit_prev_token();
	}
	cur_stack.m_items_count++;

	m_stack.emplace(marshal_json_element_type::ARRAY_ELEMENT);
	return true;
}

// Terminate decoding an array element
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_array_element_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_array_element_end without being inside a ARRAY_ELEMENT (stack empty)");
	if (m_stack.top().m_element_type != marshal_json_element_type::ARRAY_ELEMENT) DASTD_THROW(exception_marshal, "Invoked decode_array_element_end without being inside a ARRAY_ELEMENT but inside a " << m_stack.top().m_element_type);
	m_stack.pop();
}

// Start decoding a dictionary
template<class CHARTYPE, class DECOPRINTER>
size_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_dictionary_begin()
{
	assert(!m_is_typed);
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_json_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_json_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_begin inside an AR RAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACE_OPEN) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_dictionary_begin: expected '{' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	m_stack.emplace(marshal_json_element_type::DICTIONARY);

	return dastd::marshal_array_SIZE_UNKNOWN;
}

// Terminate decoding a dictionary
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_dictionary_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_end without being inside a DICTIONARY (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_end without being inside a DICTIONARY but inside a " << cur_stack.m_element_type);

	m_stack.pop();
}

// Start decoding a dictionary element
template<class CHARTYPE, class DECOPRINTER>
inline bool marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_dictionary_element_begin(std::string& key)
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_begin without being inside a DICTIONARY (stack empty)");

	stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_begin without being inside a DICTIONARY but inside a " << cur_stack.m_element_type);

	// Start decoding. It could be a '}', which means that no more fields are following
	fetch_token();
	if (m_tokenizer.get_last_process_ret() == json_tokenizer_ret::C_BRACE_CLOSE) {
		return false;
	}

	// If this is not the first field, there must be a comma
	if (!m_relaxed) {
		if (cur_stack.m_items_count > 0) {
			if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COMMA) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_dictionary_element_begin: expected ',' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
			fetch_token();
		}
	}
	cur_stack.m_items_count++;

	// The current element must be the field key
	if ((m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) || (m_tokenizer.get_string().length() == 0)) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_dictionary_field_begin: expected key string but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
	key = m_tokenizer.get_string().get_utf8();

	// The following field must be a ":"
	fetch_token();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COLON) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_dictionary_field_begin: expected ':' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	m_stack.emplace(marshal_json_element_type::DICTIONARY_ELEMENT);
	return true;
}

// Terminate decoding a dictionary element
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_dictionary_element_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_end without being inside a DICTIONARY_ELEMENT (stack empty)");
	if (m_stack.top().m_element_type != marshal_json_element_type::DICTIONARY_ELEMENT) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_end without being inside a DICTIONARY_ELEMENT but inside a " << m_stack.top().m_element_type);
	m_stack.pop();
}


// Start decoding a typed object
template<class CHARTYPE, class DECOPRINTER>
marshal_label_id_t marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_typed_begin(bool extensible)
{
	DASTD_NOWARN_UNUSED(extensible);
	assert(!m_is_typed);
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_json_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_typed_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_json_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_typed_begin inside an ARRAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_typed_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	// Start decoding. It could be a '{' or "null"
	// This decoding is in common for both `TYPEID_AS_FIELD_NAME` and `TYPEID_AS_STRUCT_FIELD`
	fetch_token();
	if (m_tokenizer.get_last_process_ret() == json_tokenizer_ret::C_NULL) {
		// marshal_json_element_type element_type, bool extensible, size_t element_offset, size_t element_size, const marshal_label_id_t* field_ids, size_t fields_count
		m_stack.emplace(marshal_json_element_type::TYPED);
		return marshal_label_id_INVALID;
	}
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACE_OPEN) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected first '{' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	marshal_label_id_t type_id = marshal_label_id_INVALID;

	switch(m_polymorphic_encoding) {
		//-------------------------------------------------------------------------
		// TYPEID_AS_FIELD_NAME
		//-------------------------------------------------------------------------
		case marshal_json_polymorphic_encoding::TYPEID_AS_FIELD_NAME: {
			// At this point there must be the type name
			fetch_token();
			if ((m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) || (m_tokenizer.get_string().length() == 0)) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected type name but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
			
			type_id = marshal_label::hash(m_tokenizer.get_string().get_utf8());

			// The following field must be a ":"
			fetch_token();
			if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COLON) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected ':' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

			// Emplace the typed entry on the stack
			m_stack.emplace(marshal_json_element_type::TYPED);
			break;
		}

		//-------------------------------------------------------------------------
		// TYPEID_AS_STRUCT_FIELD
		//-------------------------------------------------------------------------
		case marshal_json_polymorphic_encoding::TYPEID_AS_STRUCT_FIELD: {
			// At this point there must be the field named `m_typed_field`
			fetch_token();
			if ((m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) || (m_tokenizer.get_string().length() == 0)) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected field name  " << fmt_cq(m_typed_field) << " but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
			if (m_tokenizer.get_string().get_utf8() != m_typed_field) {
				DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected field named " << fmt_cq(m_typed_field) << " but got " << fmt_cq(m_tokenizer.get_string()));
			}

			// The following field must be a ":"
			fetch_token();
			if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_COLON) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected ':' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

			// Now there must be a string with the type name
			fetch_token();
			if ((m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_STRING) || (m_tokenizer.get_string().length() == 0)) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_begin: expected type name but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
			type_id = marshal_label::hash(m_tokenizer.get_string().get_utf8());

			// Signal that we are processing a typed entry
			m_is_typed = true;

			// Emplace the typed entry on the stack
			m_stack.emplace(marshal_json_element_type::TYPED);

			// Implicitely begin the struct as well
			m_stack.emplace(marshal_json_element_type::STRUCT);
			assert(m_stack.top().m_items_count == 0);
			m_stack.top().m_items_count = 1;

			break;
		}
	}

	return type_id;
}


// Skip the object without decoding it
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_typed_end_skip()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_typed_end_skip without being inside a TYPED (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::TYPED) DASTD_THROW(exception_marshal, "Invoked decode_typed_end_skip without being inside a TYPED but inside a " << cur_stack.m_element_type);

	// Skip everything until the closed brace
	skip_substructure();
	if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACE_OPEN) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_end_skip: expected last '{' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));

	m_stack.pop();
}

// Terminate decoding a typed object
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::decode_typed_end()
{
	assert(!m_is_typed);
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_typed_end without being inside a TYPED (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_json_element_type::TYPED) DASTD_THROW(exception_marshal, "Invoked decode_typed_end without being inside a TYPED but inside a " << cur_stack.m_element_type);

	switch(m_polymorphic_encoding) {
		//-------------------------------------------------------------------------
		// TYPEID_AS_FIELD_NAME
		//-------------------------------------------------------------------------
		case marshal_json_polymorphic_encoding::TYPEID_AS_FIELD_NAME: {
			// The following field must be a "}" that closes the object
			fetch_token();
			if (m_tokenizer.get_last_process_ret() != json_tokenizer_ret::C_BRACE_CLOSE) DASTD_THROW(exception_marshal, "marshal_dec_json::decode_typed_end: expected final '}' but got result " << m_tokenizer.get_last_process_ret() << " " << DECOPRINTER(m_tokenizer.get_raw_token()));
			break;
		}
		//-------------------------------------------------------------------------
		// TYPEID_AS_FIELD_NAME
		//-------------------------------------------------------------------------
		case marshal_json_polymorphic_encoding::TYPEID_AS_STRUCT_FIELD: {
			break;
		}
	}

	m_stack.pop();
}

//------------------------------------------------------------------------------
// (brief) Skip the entire substructure
//
// This function expects to be right after a "{" or "[" and skips the
// entire substructure until matched the corresponding "}" or "]".
// It stops at the first unbalanced "}" or "]" which is left in the buffer.
//------------------------------------------------------------------------------
template<class CHARTYPE, class DECOPRINTER>
inline void marshal_dec_json<CHARTYPE, DECOPRINTER>::skip_substructure()
{
	std::stack<char> stk;
	bool do_loop = true;
	while(do_loop) {
		fetch_token();

		switch(m_tokenizer.get_last_process_ret()) {
			case json_tokenizer_ret::C_BRACKET_OPEN: stk.emplace('['); break;
			case json_tokenizer_ret::C_BRACE_OPEN: stk.emplace('{'); break;

			case json_tokenizer_ret::C_BRACKET_CLOSE: {
				if (stk.empty() || (stk.top() != '[')) do_loop=false;
				else stk.pop();
				break;
			}
			case json_tokenizer_ret::C_BRACE_CLOSE: {
				if (stk.empty() || (stk.top() != '{')) do_loop=false;
				else stk.pop();
				break;
			}
			case json_tokenizer_ret::C_NOTHING_MORE:
			case json_tokenizer_ret::C_ERROR: do_loop=false; break;
			default:;
		}
	}
}

} // namespace dastd

