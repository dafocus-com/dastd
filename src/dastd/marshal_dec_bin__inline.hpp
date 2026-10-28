/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#if defined INCLUDE_dastd_marshal_dec_bin_inline && !defined dastd_marshal_dec_bin_inline
#define dastd_marshal_dec_bin_inline

#include "marshal_dec_bin.hpp"
#include "endian_aware.hpp"
#include "utf8.hpp"
#include "float.hpp"

namespace dastd {

// Decode a uint8_t
inline bool marshal_dec_bin::decode_bool(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t value;
	read_bytes(&value, sizeof(value));
	return (value != 0);
}

// Decode a uint8_t
inline uint8_t marshal_dec_bin::decode_u8(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a int8_t
inline int8_t marshal_dec_bin::decode_i8(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	int8_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a uint16_t
inline uint16_t marshal_dec_bin::decode_u16(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint16_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a int16_t
inline int16_t marshal_dec_bin::decode_i16(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	int16_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a uint32_t
inline uint32_t marshal_dec_bin::decode_u32(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint32_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a int32_t
inline int32_t marshal_dec_bin::decode_i32(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	int32_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a uint64_t
inline uint64_t marshal_dec_bin::decode_u64(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint64_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

// Decode a int64_t
inline int64_t marshal_dec_bin::decode_i64(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	int64_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return value;
}

//  Decode a 64-bit floating point
inline double marshal_dec_bin::decode_f64(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	int64_t value;
	uint8_t encoded[sizeof(value)];
	read_bytes(encoded, sizeof(value));
	little_endian_to_native(encoded, value);
	return unpack_f64(value);
}

/// @brief Decode a 128-bit UUID
/// @return Return the decoded data
/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
inline uuid marshal_dec_bin::decode_uuid(uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uuid::array_t buffer;
	decode_binary(buffer, uuid::UUID_LEN_IN_BYTES, suggestions);
	return uuid(buffer);
}


// Decode a std::string (UTF-8)
inline void marshal_dec_bin::decode_string_utf8(std::string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint32_t length;
	length = decode_u32(marshal_suggest_increasing);

	// Resizing the string and writing directly into its buffer (value.data())
	// is legitimate since, starting with C++11, the strings use contiguous memory.
	value.resize(length);
	read_bytes(value.data(), length);
}

// Decode a std::u32string
inline void marshal_dec_bin::decode_u32string(std::u32string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);

	uint32_t length;
	length = decode_u32(marshal_suggest_increasing);
	value.clear();

	// The "length" parameter is the number of bytes. Since the the string
	// is UTF-8 encoded, the value of "length" can be greater than the
	// actual char32_t string. However, it is a safe value to prepare.
	value.reserve(length);

	// Read the string decoding UTF-8
	uint8_t tmp_buf[UTF8_CHAR_MAX_LEN+1];
	char32_t ch32;
	uint8_t ch;
	size_t i;
	for (i=0; i<length; i++) {
		read_bytes(&ch, 1);
		size_t extra_chars = count_utf8_following_chars(ch);

		if (extra_chars == 0) {
			value.push_back((char32_t)ch);
		}
		else {
			// If the length does not include enough characters, throw an exception
			// It means that the UTF-8 string is badly formed.
			if (extra_chars > length-i) {
				DASTD_THROW(exception_marshal, "marshal_dec_bin::decode_u32string expected " << extra_chars << " characters to complete UTF-8, but only " << (length-i) << " were available according to length");
			}
			memset(tmp_buf, 0, sizeof(tmp_buf));
			tmp_buf[0] = ch;
			read_bytes(tmp_buf+1, extra_chars);
			//tmp_buf[extra_chars+1] = '\0';
			DASTD_DEBUG(size_t debug_size = )read_utf8_asciiz(tmp_buf, ch32);
			assert(debug_size == ((size_t)(extra_chars+1)));
			value.push_back(ch32);

			i += extra_chars+1;
		}
	}
}

// Start Decoding a structure
inline void marshal_dec_bin::decode_struct_begin(bool extensible, const marshal_label_info_t* field_infos, size_t fields_infos_count)
{
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_bin_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_struct_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_struct_begin inside an ARRAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_struct_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	size_t length = 0;
	if (extensible) length = decode_size_indicator();

	// marshal_bin_element_type element_type, bool extensible, size_t element_offset, size_t element_size, const marshal_label_id_t* field_ids, size_t fields_count
	m_stack.emplace(marshal_bin_element_type::STRUCT, extensible, m_offset+length, field_infos, fields_infos_count);
}

// Terminate Decoding a structure
inline void marshal_dec_bin::decode_struct_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_struct_end without being inside a STRUCT (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_struct_end without being inside a STRUCT but inside a " << cur_stack.m_element_type);

	// If the structure is extensible, we must skip the remaining data
	if (cur_stack.m_extensible) {
		if (m_offset < cur_stack.m_end_offset) skip_bytes(cur_stack.m_end_offset - m_offset);
		else if (m_offset > cur_stack.m_end_offset) DASTD_THROW(exception_marshal, "Too many bytes read invoking decode_struct_end; end expected at " << cur_stack.m_end_offset << " but current offset is " << m_offset);
	}

	m_stack.pop();
}

// Start decoding a field within a structure
inline marshal_label_id_t marshal_dec_bin::decode_struct_field_begin(bool* optional_present)
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_begin without being inside a struct (stack empty)");
	if (m_stack.top().m_element_type != marshal_bin_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_begin without being inside a STRUCT but inside a " << m_stack.top().m_element_type);

	stack_element& cur_stack = m_stack.top();

	// Make sure we are no reading too many fields
	if (cur_stack.m_field_pos >= cur_stack.m_fields_count) {
		//DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end too many times");
		return marshal_label_id_INVALID;
	}

	// If the structure is extensible and we ran out of data, it
	// means that the code is expecting more fields than the data provides.
	// This might mean that the data has been produced with an older version
	// of the code.
	if (cur_stack.m_extensible) {
		if (cur_stack.m_end_offset == m_offset) return marshal_label_id_INVALID;
		if (cur_stack.m_end_offset < m_offset) DASTD_THROW(exception_marshal, "Invoking decode_struct_field_end, m_end_offset (" << cur_stack.m_end_offset << ") is less than m_offset (" << m_offset << ")");
	}

	marshal_label_info_t label_info = cur_stack.m_field_infos[cur_stack.m_field_pos];
	marshal_label_id_t label_id = marshal_label_info_to_id(label_info);
	bool is_optional = marshal_label_info_is_optional(label_info);
	cur_stack.m_field_pos++;

	m_stack.emplace(marshal_bin_element_type::FIELD);

	// If marked as optional, decode the boolean telling whether the field is present or not
	if (is_optional) {
		assert(optional_present != nullptr);
		(*optional_present) = decode_bool();
	}
	else if (optional_present) (*optional_present)=true;

	return label_id;
}

// Terminate decoding a field within a structure
inline void marshal_dec_bin::decode_struct_field_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end without being inside a FIELD (stack empty)");
	if (m_stack.top().m_element_type != marshal_bin_element_type::FIELD) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end without being inside a FIELD but inside a " << m_stack.top().m_element_type);

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::FIELD) DASTD_THROW(exception_marshal, "Invoked decode_struct_field_end without being inside a FIELD but inside a " << cur_stack.m_element_type);

	m_stack.pop();
}

// Start decoding an array
inline size_t marshal_dec_bin::decode_array_begin()
{
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_bin_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_array_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_array_begin inside an ARRAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_array_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	// Get the number of elements
	size_t no_of_elements = decode_size_indicator();

	m_stack.emplace(marshal_bin_element_type::ARRAY, no_of_elements);

	return 0;
}

// Terminate decoding an array
inline void marshal_dec_bin::decode_array_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_array_end without being inside a ARRAY (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_array_end without being inside a ARRAY but inside a " << cur_stack.m_element_type);
	if (cur_stack.m_field_pos != cur_stack.m_fields_count) DASTD_THROW(exception_marshal, "Invoked decode_array_end with " << cur_stack.m_field_pos << " fields exttracted out of " << cur_stack.m_fields_count);

	m_stack.pop();
}

// Start decoding an array element
inline bool marshal_dec_bin::decode_array_element_begin()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_array_element_begin without being inside a ARRAY (stack empty)");

	stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_array_element_begin without being inside a ARRAY but inside a " << cur_stack.m_element_type);
	if (cur_stack.m_field_pos >= cur_stack.m_fields_count) return false;
	cur_stack.m_field_pos++;
	m_stack.emplace(marshal_bin_element_type::ARRAY_ELEMENT);
	return true;
}

// Terminate decoding an array element
inline void marshal_dec_bin::decode_array_element_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_array_element_end without being inside a ARRAY_ELEMENT (stack empty)");
	if (m_stack.top().m_element_type != marshal_bin_element_type::ARRAY_ELEMENT) DASTD_THROW(exception_marshal, "Invoked decode_array_element_end without being inside a ARRAY_ELEMENT but inside a " << m_stack.top().m_element_type);
	m_stack.pop();
}

// Start decoding an dictionary
inline size_t marshal_dec_bin::decode_dictionary_begin()
{
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_bin_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_begin inside an AR RAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	// Get the number of elements
	size_t no_of_elements = decode_size_indicator();

	m_stack.emplace(marshal_bin_element_type::DICTIONARY, no_of_elements);

	return 0;
}

// Terminate decoding an dictionary
inline void marshal_dec_bin::decode_dictionary_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_end without being inside a DICTIONARY (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_end without being inside a DICTIONARY but inside a " << cur_stack.m_element_type);
	if (cur_stack.m_field_pos != cur_stack.m_fields_count) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_end with " << cur_stack.m_field_pos << " fields exttracted out of " << cur_stack.m_fields_count);

	m_stack.pop();
}

// Start decoding an dictionary element
inline bool marshal_dec_bin::decode_dictionary_element_begin(std::string& key)
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_begin without being inside a DICTIONARY (stack empty)");

	stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_begin without being inside a DICTIONARY but inside a " << cur_stack.m_element_type);
	if (cur_stack.m_field_pos >= cur_stack.m_fields_count) return false;
	cur_stack.m_field_pos++;
	decode_string_utf8(key);
	m_stack.emplace(marshal_bin_element_type::DICTIONARY_ELEMENT);
	return true;
}

// Terminate decoding an dictionary element
inline void marshal_dec_bin::decode_dictionary_element_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_end without being inside a DICTIONARY_ELEMENT (stack empty)");
	if (m_stack.top().m_element_type != marshal_bin_element_type::DICTIONARY_ELEMENT) DASTD_THROW(exception_marshal, "Invoked decode_dictionary_element_end without being inside a DICTIONARY_ELEMENT but inside a " << m_stack.top().m_element_type);
	m_stack.pop();
}


// Start decoding a typed object
inline marshal_label_id_t marshal_dec_bin::decode_typed_begin(bool extensible)
{
	if (!m_stack.empty()) {
		if (m_stack.top().m_element_type == marshal_bin_element_type::STRUCT) DASTD_THROW(exception_marshal, "Invoked decode_typed_begin inside a STRUCT; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::ARRAY) DASTD_THROW(exception_marshal, "Invoked decode_typed_begin inside an ARRAY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
		if (m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY) DASTD_THROW(exception_marshal, "Invoked decode_typed_begin inside a DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED");
	}

	marshal_label_id_t type_id = decode_u32();
	size_t length = 0;

	if (extensible) length = decode_size_indicator();

	// marshal_bin_element_type element_type, bool extensible, size_t element_offset, size_t element_size, const marshal_label_id_t* field_ids, size_t fields_count
	m_stack.emplace(marshal_bin_element_type::TYPED, extensible, m_offset+length);

	return type_id;
}

// Skip the object without decoding it
inline void marshal_dec_bin::decode_typed_end_skip()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_typed_end_skip without being inside a TYPED (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::TYPED) DASTD_THROW(exception_marshal, "Invoked decode_typed_end_skip without being inside a TYPED but inside a " << cur_stack.m_element_type);
	if (!cur_stack.m_extensible) DASTD_THROW(exception_marshal, "Invoked decode_typed_end_skip on a TYPED not declared as 'extensible'");

	if (m_offset < cur_stack.m_end_offset) skip_bytes(cur_stack.m_end_offset - m_offset);
	else if (m_offset > cur_stack.m_end_offset) DASTD_THROW(exception_marshal, "Too many bytes read invoking decode_struct_end; end expected at " << cur_stack.m_end_offset << " but current offset is " << m_offset);

	m_stack.pop();
}

// Terminate decoding a typed object
inline void marshal_dec_bin::decode_typed_end()
{
	if (m_stack.empty()) DASTD_THROW(exception_marshal, "Invoked decode_typed_end without being inside a TYPED (stack empty)");

	const stack_element& cur_stack = m_stack.top();
	if (cur_stack.m_element_type != marshal_bin_element_type::TYPED) DASTD_THROW(exception_marshal, "Invoked decode_typed_end without being inside a TYPED but inside a " << cur_stack.m_element_type);

	if (cur_stack.m_extensible) {
		if (cur_stack.m_end_offset != m_offset) DASTD_THROW(exception_marshal, "In decode_typed_end, invalid offset; expected " << cur_stack.m_end_offset << ", got " << m_offset);
	}

	m_stack.pop();
}

// Decode raw binary data of a known and fixed length
inline void marshal_dec_bin::internal_decode_binary(void* buffer, size_t length, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	read_bytes(buffer, length);
}

// Decode raw binary data of a variable length
inline void marshal_dec_bin::internal_decode_varsize_binary(std::string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	size_t length = decode_u32(marshal_suggest_increasing);
	value.resize(length);
	read_bytes(value.data(), length);
}


// Read the required amount of bytes
inline void marshal_dec_bin_istream::read_bytes_impl(void* target, size_t length)
{
	m_input.read((char*)target, length);
	if (m_input.gcount() != (std::streamsize)length) {
		DASTD_THROW(exception_marshal, "marshal_dec_bin_istream::read_bytes_impl failed reading " << length << " bytes; read only " << m_input.gcount());
	}
}


// Read the required amount of bytes
template<concept_integral_8bit CHARTYPE>
void marshal_dec_bin_source<CHARTYPE>::read_bytes_impl(void* target, size_t length)
{
		size_t bytesRead = m_input.tentative_read((CHARTYPE*)target, length);
		if (bytesRead != length) {
				DASTD_THROW(exception_marshal, "marshal_dec_bin_source::read_bytes_impl failed reading " << length << " bytes; read only " << bytesRead);
		}
}

} // namespace dastd

#endif
