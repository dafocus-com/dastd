/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#pragma once
#include "marshal_enc.hpp"
#include "json_encoder.hpp"
#include "char32string.hpp"
#include "base64.hpp"
#include "istream_membuf.hpp"
#include "marshal_json.hpp"
#include <stack>
#include <iomanip>
#include <iostream>
#include <limits>

namespace dastd {
/// @brief Binary little-endian marshaling encoder
///
/// Encodes binary data using the binary little-endian encoding.
/// See @link marshaling_json_format "Marshaling binary format" @endlink
/// for details on the binary encoding format.
class marshal_enc_json: public marshal_enc {
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

		/// @brief See @ref marshal_json_polymorphic_encoding
		marshal_json_polymorphic_encoding m_polymorphic_encoding;

		/// @brief Name of the field in case of `TYPEID_AS_STRUCT_FIELD`; see @ref marshal_json_polymorphic_encoding
		std::string m_typed_field;

		/// @brief Set to true when the following `encode_struct_begin` must encode also the
		///        the typed field with the type name.
		///
		/// The name of the type will be available in `m_type_id`. Used only in case of TYPEID_AS_STRUCT_FIELD.
		bool m_is_typed = false;

		/// @brief Type id of the type that is to be encoded by the following `encode_struct_begin` in case
		///        of TYPEID_AS_STRUCT_FIELD
		marshal_label m_type_id;

	public:
		/// @brief Constructor
		/// @param out Stream where the JSON data will be written
		/// @param polymorphic_encoding   See @ref marshal_json_polymorphic_encoding
		/// @param typed_field            Name of the field in case of `TYPEID_AS_STRUCT_FIELD`; see @ref marshal_json_polymorphic_encoding
		marshal_enc_json(std::ostream &out, marshal_json_polymorphic_encoding polymorphic_encoding=marshal_json_polymorphic_encoding::TYPEID_AS_FIELD_NAME, std::string_view typed_field="$type"):
			m_polymorphic_encoding(polymorphic_encoding), m_typed_field(typed_field), m_out(out) {}

		/// @brief Encode a bool
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_bool(bool value, uint32_t suggestions=0) override;

		/// @brief Encode a uint8_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_u8(uint8_t value, uint32_t suggestions=0) override;

		/// @brief Encode a int8_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_i8(int8_t value, uint32_t suggestions=0) override;

		/// @brief Encode a uint16_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_u16(uint16_t value, uint32_t suggestions=0) override;

		/// @brief Encode a int16_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_i16(int16_t value, uint32_t suggestions=0) override;

		/// @brief Encode a uint32_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_u32(uint32_t value, uint32_t suggestions=0) override;

		/// @brief Encode a int32_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_i32(int32_t value, uint32_t suggestions=0) override;

		/// @brief Encode a uint64_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_u64(uint64_t value, uint32_t suggestions=0) override;

		/// @brief Encode a int64_t
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_i64(int64_t value, uint32_t suggestions=0) override;

		/// @brief Encode a 64-bit floating point
		/// @return Return the decoded data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_f64(double value, uint32_t suggestions=0) override;

		/// @brief Encode a UUID
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_uuid(const uuid& value, uint32_t suggestions=0) override;

		/// @brief Encode a std::string (UTF-8)
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_string_utf8(const std::string& value, uint32_t suggestions=0) override;

		/// @brief Encode a std::u32string
		/// @param value Value to be encoded.
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void encode_u32string(const std::u32string& value, uint32_t suggestions=0) override;

		/// @brief Start encoding a structure
		/// @param extensible If true, the structure will be encoded so more or less fields can be expected.
		///
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void encode_struct_begin(bool extensible) override;

		/// @brief Terminate encoding a structure
		///
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void encode_struct_end() override;

		/// @brief Start encoding a field within a structure
		/// @param label Contains the name of the field and its label-id
		/// @param opt   Indicates whether the field is mandatory or optional
		///
		/// This must be invoked inside a `encode_struct_begin` and `encode_struct_end` pair.
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void encode_struct_field_begin(marshal_label label, marshal_optional_field opt=marshal_optional_field::MANDATORY) override;

		/// @brief Terminate encoding a field within a structure
		///
		/// This must be invoked inside a `encode_struct_begin` and `encode_struct_end` pair.
		/// See @link marshaling_structs documentation @endlink for details and examples.
		virtual void encode_struct_field_end() override;

		/// @brief Start encoding an array
		/// @param count Number of elements that will be encoded
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		virtual void encode_array_begin(size_t count) override;

		/// @brief Terminate encoding an array
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		virtual void encode_array_end() override;

		/// @brief Start encoding an array element
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		virtual void encode_array_element_begin() override;

		/// @brief Terminate encoding an array element
		///
		/// See @link marshaling_arrays documentation @endlink for details and examples.
		virtual void encode_array_element_end() override;

		/// @brief Start encoding a dictionary
		/// @param count Number of elements that will be encoded
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		virtual void encode_dictionary_begin(size_t count) override;

		/// @brief Terminate encoding a dictionary
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		virtual void encode_dictionary_end() override;

		/// @brief Start encoding a dictionary element
		/// @param key UTF-8 encoded string key that will be encoded along with the object
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		virtual void encode_dictionary_element_begin(const std::string& key) override;

		/// @brief Terminate encoding a dictionary element
		///
		/// See @link marshaling_dictionaries documentation @endlink for details and examples.
		virtual void encode_dictionary_element_end() override;

		/// @brief Start encoding a typed object
		/// @param label Contains the name of the type and its label-id
		/// @param extensible If true, the object is encoded in way that allows skipping it during decoding.
		///
		/// See @link marshaling_typeds documentation @endlink for details and examples.
		virtual void encode_typed_begin(marshal_label label, bool extensible) override;

		/// @brief Terminate encoding a typed object
		///
		/// See @link marshaling_typeds documentation @endlink for details and examples.
		virtual void encode_typed_end() override;

	protected:
		/// @brief Encode fixed-size, known in advance, raw binary data
		/// @param data Raw data
		/// @param length Length of the raw data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		virtual void internal_encode_binary(const void* data, size_t length, uint32_t suggestions=0) override;

		/// @brief Encode variably sized, raw binary data
		/// @param data Raw data
		/// @param length Length of the raw data
		/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
		/// @note The encoder must encode the data in way that allows the decoder do deduce its length at runtime.
		///       This includes a length field, some kind of terminator in the encoding or anything else suitable.
		virtual void internal_encode_varsize_binary(const void* data, size_t length, uint32_t suggestions=0) override;

		/// @brief Output stream
		std::ostream &m_out;
};

// Encode a bool
inline void marshal_enc_json::encode_bool(bool value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << (value ? "true" : "false");
}

// Encode a uint8_t
inline void marshal_enc_json::encode_u8(uint8_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << (unsigned)value;
}

// Encode a int8_t
inline void marshal_enc_json::encode_i8(int8_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << (int)value;
}

// Encode a uint16_t
inline void marshal_enc_json::encode_u16(uint16_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << value;
}

// Encode a int16_t
inline void marshal_enc_json::encode_i16(int16_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << value;
}

// Encode a uint32_t
inline void marshal_enc_json::encode_u32(uint32_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << value;
}

// Encode a int32_t
inline void marshal_enc_json::encode_i32(int32_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << value;
}

// Encode a uint64_t
inline void marshal_enc_json::encode_u64(uint64_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << value;
}

// Encode a int64_t
inline void marshal_enc_json::encode_i64(int64_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << value;
}

// Encode a 64-bit floating point
inline void marshal_enc_json::encode_f64(double value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
}

// Encode a UUID
inline void marshal_enc_json::encode_uuid(const uuid& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << '"' << value << '"';
}

// Encode a std::string (UTF-8)
inline void marshal_enc_json::encode_string_utf8(const std::string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << '"';
	if (!json_encode_string_from_UTF8(m_out, value)) {
		DASTD_THROW(exception_marshal, "marshal_enc_json::encode_string_utf8 Error decoding UTF-8 string");
	}
	m_out << '"';
}

// Encode a std::u32string
inline void marshal_enc_json::encode_u32string(const std::u32string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	m_out << '"';
	json_encode_string(m_out, value);
	m_out << '"';
}

// (brief) Encode fixed-size, known in advance, raw binary data
inline void marshal_enc_json::internal_encode_binary(const void* data, size_t length, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	istream_membuf inp(data, length);
	m_out << '"';
	base64_encode(inp, m_out);
	m_out << '"';
}

// (brief) Encode variably sized, raw binary data
inline void marshal_enc_json::internal_encode_varsize_binary(const void* data, size_t length, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(!m_is_typed);
	istream_membuf inp(data, length);
	m_out << '"';
	base64_encode(inp, m_out);
	m_out << '"';
}

// Start encoding a structure
inline void marshal_enc_json::encode_struct_begin(bool extensible)
{
	DASTD_NOWARN_UNUSED(extensible);
	// Invoked encode_struct_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_json_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_json_element_type::ARRAY) && (m_stack.top().m_element_type != marshal_json_element_type::DICTIONARY)));

	m_stack.emplace(marshal_json_element_type::STRUCT);
	m_out << '{';

	if (m_is_typed) {
		m_is_typed = false;
		assert(m_stack.top().m_items_count == 0);
		m_stack.top().m_items_count++;
		encode_string_utf8(m_typed_field);
	  m_out << ':';
		encode_string_utf8(m_type_id.m_label_text);
	}
}

// Terminate encoding a structure
inline void marshal_enc_json::encode_struct_end()
{
	// Invoked encode_struct_end without being inside a STRUCT (stack empty)
	assert(!m_stack.empty());
	assert (m_stack.top().m_element_type == marshal_json_element_type::STRUCT);
	assert(!m_is_typed);
	m_out << '}';
	m_stack.pop();
}

// Start encoding a field within a structure
inline void marshal_enc_json::encode_struct_field_begin(marshal_label label, marshal_optional_field opt)
{
	assert(!m_is_typed);
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::STRUCT);
	if (m_stack.top().m_items_count > 0) {
		m_out << ',';
	}
	m_stack.top().m_items_count++;
	m_stack.emplace(marshal_json_element_type::FIELD);

	encode_string_utf8(label.m_label_text);
	m_out << ':';
	if (opt == marshal_optional_field::OPTIONAL_MISSING) m_out << "null";
}

// Terminate encoding a field within a structure
inline void marshal_enc_json::encode_struct_field_end()
{
	assert(!m_is_typed);
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::FIELD);
	m_stack.pop();
}

// Start encoding an array
inline void marshal_enc_json::encode_array_begin(size_t count)
{
	DASTD_NOWARN_UNUSED(count);
	assert(!m_is_typed);
	// Invoked encode_struct_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_json_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_json_element_type::ARRAY) && (m_stack.top().m_element_type != marshal_json_element_type::DICTIONARY)));
	m_stack.emplace(marshal_json_element_type::ARRAY);
	m_out << '[';
}

// Terminate encoding an array
inline void marshal_enc_json::encode_array_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::ARRAY);
	assert(!m_is_typed);
	m_stack.pop();
	m_out << ']';
}

// Start encoding an array element
inline void marshal_enc_json::encode_array_element_begin()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::ARRAY);
	assert(!m_is_typed);
	if (m_stack.top().m_items_count > 0) {
		m_out << ',';
	}
	m_stack.top().m_items_count++;
	m_stack.emplace(marshal_json_element_type::ARRAY_ELEMENT);
}

// Terminate encoding an array element
inline void marshal_enc_json::encode_array_element_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::ARRAY_ELEMENT);
	assert(!m_is_typed);
	m_stack.pop();
}

// Start encoding a dictionary
inline void marshal_enc_json::encode_dictionary_begin(size_t count)
{
	DASTD_NOWARN_UNUSED(count);
	assert(!m_is_typed);
	// Invoked encode_struct_begin inside a STRUCT o DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, DICTIONARY_ELEMENT TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_json_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_json_element_type::DICTIONARY) && (m_stack.top().m_element_type != marshal_json_element_type::DICTIONARY)));
	m_stack.emplace(marshal_json_element_type::DICTIONARY);
	m_out << '{';
}

// Terminate encoding a dictionary
inline void marshal_enc_json::encode_dictionary_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY);
	assert(!m_is_typed);
	m_stack.pop();
	m_out << '}';
}

// Start encoding a dictionary element
inline void marshal_enc_json::encode_dictionary_element_begin(const std::string& key)
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY);
	assert(!m_is_typed);
	if (m_stack.top().m_items_count > 0) {
		m_out << ',';
	}
	encode_string_utf8(key);
	m_out << ':';
	m_stack.top().m_items_count++;
	m_stack.emplace(marshal_json_element_type::DICTIONARY_ELEMENT);
}

// Terminate encoding a dictionary element
inline void marshal_enc_json::encode_dictionary_element_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::DICTIONARY_ELEMENT);
	assert(!m_is_typed);
	m_stack.pop();
}

// Start encoding a typed object
inline void marshal_enc_json::encode_typed_begin(marshal_label label, bool extensible)
{
	DASTD_NOWARN_UNUSED(extensible);

	// Invoked encode_typed_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_json_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_json_element_type::ARRAY) && (m_stack.top().m_element_type != marshal_json_element_type::DICTIONARY)));
	assert(!m_is_typed);

	switch(m_polymorphic_encoding) {
		case marshal_json_polymorphic_encoding::TYPEID_AS_FIELD_NAME: {
			m_out << '{';
			// Encode the type identifier
			encode_string_utf8(label.m_label_text);
			m_out << ':';
			break;
		}
		case marshal_json_polymorphic_encoding::TYPEID_AS_STRUCT_FIELD: {
			m_is_typed = true;
			m_type_id = label;
			break;
		}
	}

	// Add the element on the stack with the position where the size indicator would
	// be if enabled.
	m_stack.emplace(marshal_json_element_type::TYPED);
}

// Terminate encoding a typed object
inline void marshal_enc_json::encode_typed_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_json_element_type::TYPED);
	assert(!m_is_typed);
	switch(m_polymorphic_encoding) {
		case marshal_json_polymorphic_encoding::TYPEID_AS_FIELD_NAME: {
		  m_out << '}';
			break;
		}
		case marshal_json_polymorphic_encoding::TYPEID_AS_STRUCT_FIELD: {
			break;
		}
	}
	m_stack.pop();
}

} // namespace dastd
