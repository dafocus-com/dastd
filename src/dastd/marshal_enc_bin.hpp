/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#pragma once
#include "marshal_enc.hpp"
#include "marshal_bin.hpp"
#include "endian_aware.hpp"
#include "utf8.hpp"
#include "float.hpp"
#include <stack>

namespace dastd {
/// @brief Binary little-endian marshaling encoder
///
/// Encodes binary data using the binary little-endian encoding.
/// See @link marshaling_bin_format "Marshaling binary format" @endlink
/// for details on the binary encoding format.
template<class STREAMPOS>
class marshal_enc_bin: public marshal_enc {
	private:
		/// @brief Stack element
		struct stack_element {
			marshal_bin_element_type m_element_type;

			/// @brief Position where the element starts
			STREAMPOS m_pos;

			/// @brief True if marked as extensible
			bool m_extensible = false;

			/// @brief Constructor
			stack_element(marshal_bin_element_type element_type, STREAMPOS pos, bool extensible=false):
				m_element_type(element_type), m_pos(pos), m_extensible(extensible) {}

			/// @brief Constructor
			stack_element() {}
		};

		/// @brief Internal encoding stack
		std::stack<stack_element> m_stack;

	public:
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
		/// @param count Number of elements that will be encoded
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

		/// @brief Write the required amount of bytes
		///
		/// Attempts to write the indicated number of bytes. If it fails, it
		/// should throw an exception or report in other way.
		///
		/// @param source The source buffer where to take the data to be written
		/// @param length The required number of bytes
		virtual void write_bytes(const void* source, size_t length) = 0;

		/// @brief Get the current position into the output stream
		/// @return Returns the current absolute position
		virtual STREAMPOS get_curr_pos() const = 0;

		/// @brief Set the current position into the output stream.
		/// @param pos Position where write_bytes must be able to write.
		///
		/// Note: the `pos` position is ether inside an area that has been previously
		/// written by this object or at the end of the stream (to append new data).
		/// The `pos` value is always a value returned by `get_curr_pos()`.
		virtual void set_curr_pos(STREAMPOS pos) = 0;

		/// @brief Calculate the difference in bytes between two STREAMPOS
		/// @param p1 Lowest position value
		/// @param p2 Highest position value
		/// @return Returns the difference in bytes
		virtual size_t pos_diff(STREAMPOS p1, STREAMPOS p2) const = 0;

	private:
		/// @brief Encode a size indicaotr
		void encode_size_indicator(size_t size) {encode_u32((uint32_t)size);}
};

/// @brief Binary little-endian marshaling encoder
///
/// Encodes binary data using the binary little-endian encoding.
/// Operates by writing on a std::ostream.
class marshal_enc_bin_ostream: public marshal_enc_bin<std::streampos> {
	public:
		/// @brief Constructor
		/// @param output Binary output stream; make sure it does no ASCII transaltions.
		marshal_enc_bin_ostream(std::ostream& output): m_output(output) {}

		/// @brief Write the required amount of bytes
		///
		/// Attempts to write the indicated number of bytes. If it fails, it
		/// should throw an exception or report in other way.
		///
		/// @param source The source buffer where to take the data to be written
		/// @param length The required number of bytes
		/// @throw dastd::exception_marshal
		virtual void write_bytes(const void* source, size_t length) override;

		/// @brief Get the current position into the output stream
		/// @return Returns the current absolute position
		/// @throw dastd::exception_marshal
		virtual std::streampos get_curr_pos() const override;

		/// @brief Set the current position into the output stream.
		/// @param pos Position where write_bytes must be able to write.
		///
		/// Note: the `pos` position is ether inside an area that has been previously
		/// written by this object or at the end of the stream (to append new data).
		/// The `pos` value is always a value returned by `get_curr_pos()`.
		/// @throw dastd::exception_marshal
		virtual void set_curr_pos(std::streampos pos) override;

		/// @brief Calculate the difference in bytes between two STREAMPOS
		/// @param p1 Lowest position value
		/// @param p2 Highest position value
		/// @return Returns the difference in bytes
		virtual size_t pos_diff(std::streampos p1, std::streampos p2) const override {return (size_t)(p2-p1);}

	protected:
		/// @brief Binary output stream
		std::ostream& m_output;
};

// Encode a bool
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_bool(bool value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t v = (value ? 1 : 0);
	write_bytes((const char*)&v, 1);
}

// Encode a uint8_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_u8(uint8_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	write_bytes((const char*)&value, 1);
}

// Encode a int8_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_i8(int8_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	write_bytes((const char*)&value, 1);
}

// Encode a uint16_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_u16(uint16_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[sizeof(value)];
	native_to_little_endian(value, encoded);
	write_bytes(encoded, sizeof(value));
}

// Encode a int16_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_i16(int16_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[sizeof(value)];
	native_to_little_endian(value, encoded);
	write_bytes(encoded, sizeof(value));
}

// Encode a uint32_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_u32(uint32_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[sizeof(value)];
	native_to_little_endian(value, encoded);
	write_bytes(encoded, sizeof(value));
}

// Encode a int32_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_i32(int32_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[sizeof(value)];
	native_to_little_endian(value, encoded);
	write_bytes(encoded, sizeof(value));
}

// Encode a uint64_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_u64(uint64_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[sizeof(value)];
	native_to_little_endian(value, encoded);
	write_bytes(encoded, sizeof(value));
}

// Encode a int64_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_i64(int64_t value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[sizeof(value)];
	native_to_little_endian(value, encoded);
	write_bytes(encoded, sizeof(value));
}

// Encode a int64_t
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_f64(double value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	uint8_t encoded[8];
	native_to_little_endian(pack_f64(value), encoded);
	write_bytes(encoded, 8);
}


// Encode a UUID
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_uuid(const uuid& value, uint32_t suggestions)
{
	uuid::array_t buffer;
	value.get_binary(buffer);
	encode_binary(buffer, uuid::UUID_LEN_IN_BYTES, suggestions);
}

// Encode a std::string (UTF-8)
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_string_utf8(const std::string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	encode_u32((uint32_t)value.size(), marshal_suggest_increasing);
	write_bytes(value.c_str(), value.size());
}

// Encode a std::u32string
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_u32string(const std::u32string& value, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	size_t len = calc_utf8_length(value.c_str(), value.length());
	encode_u32((uint32_t)len, marshal_suggest_increasing);

	size_t i;
	char buf[UTF8_CHAR_MAX_LEN];
	for (i=0; i<value.length(); i++) {
		len = write_utf8_asciiz(buf, value[i]);
		write_bytes(buf, len);
	}
}

// (brief) Encode fixed-size, known in advance, raw binary data
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::internal_encode_binary(const void* data, size_t length, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	write_bytes(data, length);
}

// (brief) Encode variably sized, raw binary data
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::internal_encode_varsize_binary(const void* data, size_t length, uint32_t suggestions)
{
	DASTD_NOWARN_UNUSED(suggestions);
	assert(length <= std::numeric_limits<uint32_t>::max());
	encode_u32((uint32_t)length, marshal_suggest_increasing);
	write_bytes(data, length);
}

// Start encoding a structure
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_struct_begin(bool extensible)
{
	// Invoked encode_struct_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_bin_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_bin_element_type::ARRAY)));

	m_stack.emplace(marshal_bin_element_type::STRUCT, get_curr_pos(), extensible);

	// Prepare the space for the size indicator
	if (extensible) encode_size_indicator(0);
}

// Terminate encoding a structure
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_struct_end()
{
	// Invoked encode_struct_end without being inside a STRUCT (stack empty)
	assert(!m_stack.empty());
	assert (m_stack.top().m_element_type == marshal_bin_element_type::STRUCT);

	if (m_stack.top().m_extensible) {
		STREAMPOS currpos = get_curr_pos();
		size_t size = pos_diff(m_stack.top().m_pos, currpos);
		set_curr_pos(m_stack.top().m_pos);
		encode_size_indicator(size-4);
		set_curr_pos(currpos);
	}
	m_stack.pop();
}

// Start encoding a field within a structure
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_struct_field_begin(marshal_label label, marshal_optional_field opt)
{
	DASTD_NOWARN_UNUSED(label);
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::STRUCT);
	switch(opt) {
		case marshal_optional_field::MANDATORY: m_stack.emplace(marshal_bin_element_type::FIELD, get_curr_pos(), false); break;
		case marshal_optional_field::OPTIONAL_MISSING: {
			encode_bool(false);
			m_stack.emplace(marshal_bin_element_type::FIELD_MISSING, get_curr_pos(), false);
			break;
		}
		case marshal_optional_field::OPTIONAL_PRESENT: {
			encode_bool(true);
			m_stack.emplace(marshal_bin_element_type::FIELD, get_curr_pos(), false);
			break;
		}
	}

}

// Terminate encoding a field within a structure
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_struct_field_end()
{
	assert(!m_stack.empty());
	assert((m_stack.top().m_element_type == marshal_bin_element_type::FIELD) || (m_stack.top().m_element_type == marshal_bin_element_type::FIELD_MISSING));
	m_stack.pop();
}

// Start encoding an array
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_array_begin(size_t count)
{
	// Invoked encode_struct_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_bin_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_bin_element_type::ARRAY)));
	encode_size_indicator(count);
	m_stack.emplace(marshal_bin_element_type::ARRAY, get_curr_pos(), false);
}

// Terminate encoding an array
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_array_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::ARRAY);
	m_stack.pop();
}

// Start encoding an array element
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_array_element_begin()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::ARRAY);
	m_stack.emplace(marshal_bin_element_type::ARRAY_ELEMENT, get_curr_pos(), false);
}

// Terminate encoding an array element
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_array_element_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::ARRAY_ELEMENT);
	m_stack.pop();
}

// Start encoding an dictionary
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_dictionary_begin(size_t count)
{
	// Invoked encode_struct_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, DICTIONARY_ELEMENT TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_bin_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_bin_element_type::DICTIONARY)));
	encode_size_indicator(count);
	m_stack.emplace(marshal_bin_element_type::DICTIONARY, get_curr_pos(), false);
}

// Terminate encoding an dictionary
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_dictionary_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY);
	m_stack.pop();
}

// Start encoding an dictionary element
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_dictionary_element_begin(const std::string& key)
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY);
	m_stack.emplace(marshal_bin_element_type::DICTIONARY_ELEMENT, get_curr_pos(), false);
	encode_string_utf8(key);
}

// Terminate encoding an dictionary element
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_dictionary_element_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::DICTIONARY_ELEMENT);
	m_stack.pop();
}

// Start encoding a typed object
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_typed_begin(marshal_label label, bool extensible)
{
	// Invoked encode_struct_begin inside a STRUCT, ARRAY or DICTIONARY; it should be at root or inside a STRUCT_ELEMENT, ARRAY_ELEMENT, DICTRIONARY_ELEMENT or TYPED
	assert(m_stack.empty() || ((m_stack.top().m_element_type != marshal_bin_element_type::STRUCT) && (m_stack.top().m_element_type != marshal_bin_element_type::ARRAY)));

	// Encode the type identifier
	encode_u32(label.m_label_id);

	// Add the element on the stack with the position where the size indicator would
	// be if enabled.
	m_stack.emplace(marshal_bin_element_type::TYPED, get_curr_pos(), extensible);

	// If required, prepare the space for the size indicator
	if (extensible) encode_size_indicator(0);
}

// Terminate encoding a typed object
template<class STREAMPOS>
inline void marshal_enc_bin<STREAMPOS>::encode_typed_end()
{
	assert(!m_stack.empty());
	assert(m_stack.top().m_element_type == marshal_bin_element_type::TYPED);
	if (m_stack.top().m_extensible) {
		STREAMPOS currpos = get_curr_pos();
		size_t size = pos_diff(m_stack.top().m_pos, currpos);
		set_curr_pos(m_stack.top().m_pos);
		encode_size_indicator(size-4);
		set_curr_pos(currpos);
	}
	m_stack.pop();
}

// Write the required amount of bytes
inline void marshal_enc_bin_ostream::write_bytes(const void* source, size_t length)
{
	m_output.write((const char*)source, length);
	if (m_output.bad() || m_output.fail()) {
		DASTD_THROW(exception_marshal, "marshal_enc_bin_ostream::write_bytes failed writing " << length << " bytes");
	}
}

// Get the current position into the output stream
inline std::streampos marshal_enc_bin_ostream::get_curr_pos() const
{
	std::streampos pos = m_output.tellp();
	if (m_output.bad() || m_output.fail()) {
		DASTD_THROW(exception_marshal, "marshal_enc_bin_ostream::set_curr_pos");
	}
	return pos;
}

// Set the current position into the output stream.
inline void marshal_enc_bin_ostream::set_curr_pos(std::streampos pos)
{
	m_output.seekp(pos);
	if (m_output.bad() || m_output.fail()) {
		DASTD_THROW(exception_marshal, "marshal_enc_bin_ostream::set_curr_pos");
	}
}

} // namespace dastd
