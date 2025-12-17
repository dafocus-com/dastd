/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#pragma once
#include "marshal_dec.hpp"
#include "marshal_bin.hpp"
#include "source_string_or_vector.hpp"
#include <stack>

namespace dastd {

	/// @brief Binary little-endian marshaling decoder
	///
	/// Decodes binary data using the binary little-endian encoding.
	/// See @link marshaling_bin_format "Marshaling binary format" @endlink
	/// for details on the binary encoding format.
	class marshal_dec_bin: public marshal_dec {
		private:
			/// @brief Stack element
			struct stack_element {
				/// @brief Type of the element on the stack
				marshal_bin_element_type m_element_type;

				/// @brief True if extensible
				bool m_extensible = false;

				/// @brief Offset where the element is expected to terminate.
				///
				/// This is the position of the first byte that is not part of
				/// the current object. It is calculated only if "m_extensible" is
				/// set to true.
				size_t m_end_offset = 0;

				/// @brief Used by structures; contains the list of expected fields
				const marshal_label_info_t* m_field_infos = nullptr;

				/// @brief Used by structures; indicates the position the number of elements
				/// in the `m_field_infos` array.
				/// In case of arrays, it contains the number of elements.
				size_t m_fields_count = 0;

				/// @brief Used by structures; indicates the position of the next structure
				/// field.
				/// In case of arrays, it contains the current of elements.
				size_t m_field_pos = 0;

				/// @brief Constructor
				stack_element(marshal_bin_element_type element_type):
					m_element_type(element_type) {}

				/// @brief Constructor
				stack_element(marshal_bin_element_type element_type, size_t fields_count):
					m_element_type(element_type), m_fields_count(fields_count) {}

				/// @brief Constructor
				stack_element(marshal_bin_element_type element_type, bool extensible, size_t end_offset):
					m_element_type(element_type), m_extensible(extensible), m_end_offset(end_offset) {}

				/// @brief Constructor
				stack_element(marshal_bin_element_type element_type, bool extensible, size_t end_offset, const marshal_label_info_t* field_infos, size_t fields_count):
					m_element_type(element_type), m_extensible(extensible), m_end_offset(end_offset),
					m_field_infos(field_infos), m_fields_count(fields_count) {}

				/// @brief Constructor
				stack_element() {}
			};

			/// @brief Internal encoding stack
			std::stack<stack_element> m_stack;

			/// @brief Bytes read so far
			size_t m_offset = 0;

		public:
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
			virtual marshal_label_id_t decode_struct_field_begin(bool* optional_present = nullptr) override;

			/// @brief Terminate decoding a field within a structure
			///
			/// This must be invoked inside a `decode_struct_begin` and `decode_struct_end` pair.
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void decode_struct_field_end() override;

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

			/// @brief Read the required amount of bytes
			///
			/// Attempts to read the indicated number of bytes. If it fails, it
			/// should throw an exception or report in other way.
			///
			/// @param target The target buffer where to write the data
			/// @param length The required number of bytes
			virtual void read_bytes_impl(void* target, size_t length) = 0;

			/// @brief Skip the required amount of bytes
			///
			/// Attempts to skip the indicated number of bytes. If it fails, it
			/// should throw an exception or report in other way.
			/// Default implementation is trivial (reads a byte at the time).
			///
			/// @param length The required number of bytes
			virtual void skip_bytes_impl(size_t length) {
				for(; length>0; length--) {uint8_t c; read_bytes_impl(&c, 1);}
			}

		private:
			/// @brief Read the required amount of bytes
			///
			/// Attempts to read the indicated number of bytes. If it fails, it
			/// should throw an exception or report in other way.
			///
			/// @param target The target buffer where to write the data
			/// @param length The required number of bytes
			void read_bytes(void* target, size_t length) {
				read_bytes_impl(target, length);
				m_offset += length;
			}

			/// @brief Skip the required amount of bytes
			///
			/// Attempts to skipthe indicated number of bytes. If it fails, it
			/// should throw an exception or report in other way.
			///
			/// @param length The required number of bytes
			void skip_bytes(size_t length) {
				skip_bytes_impl(length);
				m_offset += length;
			}

			/// @brief Decode a size indicaotr
			size_t decode_size_indicator() {return (size_t)decode_u32();}
	};

	/// @brief Binary little-endian marshaling decoder
	///
	/// Decodes binary data using the binary little-endian encoding.
	/// Operates by reading a std::istream.
	class marshal_dec_bin_istream: public marshal_dec_bin {
		public:
			/// @brief Constructor
			/// @param output Binary output stream; make sure it does no ASCII transaltions.
			marshal_dec_bin_istream(std::istream& input): m_input(input) {}

			/// @brief Read the required amount of bytes
			///
			/// Attempts to read the indicated number of bytes. If it fails, it
			/// should throw an exception or report in other way.
			///
			/// @param target The target buffer where to write the data
			/// @param length The required number of bytes
			virtual void read_bytes_impl(void* target, size_t length) override;

		protected:
			/// @brief Binary output stream
			std::istream& m_input;
	};

	/// @brief Binary little-endian marshaling decoder
	///
	/// Decodes binary data using the binary little-endian encoding.
	/// Operates by reading a dastd::source.
	template<concept_integral_8bit CHARTYPE>
	class marshal_dec_bin_source: public marshal_dec_bin {
	public:
			/// @brief Constructor
			/// @param output Binary output stream; make sure it does no ASCII transaltions.
			marshal_dec_bin_source(dastd::source<CHARTYPE>& input) : m_input(input) {}

			/// @brief Read the required amount of bytes
			///
			/// Attempts to read the indicated number of bytes. If it fails, it
			/// should throw an exception or report in other way.
			///
			/// @param target The target buffer where to write the data
			/// @param length The required number of bytes
			virtual void read_bytes_impl(void* target, size_t length) override;

	protected:
			/// @brief Binary output stream
			dastd::source<CHARTYPE>& m_input;
	};

	/// @brief Binary little-endian marshaling decoder
	///
	/// Decodes binary data using the binary little-endian encoding.
	/// Operates by reading a std::string.
	class marshal_dec_bin_string: public marshal_dec_bin_source<char> {
	public:
			/// @brief Constructor
			/// @param output Binary output stream; make sure it does no ASCII transaltions.
			marshal_dec_bin_string(const std::string& input) : marshal_dec_bin_source<char>(m_input), m_input(input) {}

	protected:
			/// @brief Binary output stream
			dastd::source_string_or_vector<char> m_input;
	};

} // namespace dastd
#define INCLUDE_dastd_marshal_dec_bin_inline
#include "marshal_dec_bin__inline.hpp"
#undef INCLUDE_dastd_marshal_dec_bin_inline
