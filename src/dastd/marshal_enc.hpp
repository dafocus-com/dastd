/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#pragma once
#include "marshal.hpp"

namespace dastd {
	/// @brief Used to indicate whether an field is mandatory
	enum class marshal_optional_field {
		MANDATORY,
		OPTIONAL_PRESENT,
		OPTIONAL_MISSING
	};


	/// @brief Base class for the marshaling encoder
	///
	/// This class provides an interface that allows marshaling basic
	/// types. The derived classes will provide with specific
	/// streaming methods.
	class marshal_enc: public marshal {
		public:
			/// @brief Template for encoding integral types
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			template<marshal_integral_types TYPE>
			void encode(TYPE value, uint32_t suggestions=0);

			/// @brief Encode a bool
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_bool(bool value, uint32_t suggestions=0) = 0;

			/// @brief Encode a uint8_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_u8(uint8_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a int8_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_i8(int8_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a uint16_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_u16(uint16_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a int16_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_i16(int16_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a uint32_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_u32(uint32_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a int32_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_i32(int32_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a uint64_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_u64(uint64_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a int64_t
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_i64(int64_t value, uint32_t suggestions=0) = 0;

			/// @brief Encode a 64-bit floating point
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_f64(double value, uint32_t suggestions=0) = 0;

			/// @brief Encode a std::string (UTF-8)
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_string_utf8(const std::string& value, uint32_t suggestions=0) = 0;

			/// @brief Encode a std::u32string
			/// @param value Value to be encoded.
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void encode_u32string(const std::u32string& value, uint32_t suggestions=0) = 0;

			/// @brief Encode fixed-size, known in advance, raw binary data
			/// @param data Raw data
			/// @param length Length of the raw data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			/// @note If the binary data is of variable length, its length must be explicitely encoded
			void encode_binary(const void* data, size_t length, uint32_t suggestions=0) {internal_encode_binary(data, length, suggestions);}

			/// @brief Encode fixed-size, known in advance, raw binary data from a std::string
			/// @param value Raw data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			/// @note If the binary data is of variable length, its length must be explicitely encoded
			void encode_binary(const std::string& value, uint32_t suggestions=0) {internal_encode_binary(value.c_str(), value.length(), suggestions);}

			/// @brief Encode variably sized, raw binary data
			/// @param data Raw data
			/// @param length Length of the raw data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			/// @note Supports data of variable length; information about the length is automatically encoded by the encoder.
			void encode_varsize_binary(const void* data, size_t length, uint32_t suggestions=0) {internal_encode_varsize_binary(data, length, suggestions);}

			/// @brief Encode variably sized, raw binary data from a std::string
			/// @param value Raw data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			/// @note Supports data of variable length; information about the length is automatically encoded by the encoder.
			void encode_varsize_binary(const std::string& value, uint32_t suggestions=0) {internal_encode_varsize_binary(value.c_str(), value.length(), suggestions);}

			/// @brief Start encoding a structure
			/// @param extensible If true, the structure will be encoded so more or less fields can be expected.
			///
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void encode_struct_begin(bool extensible) = 0;

			/// @brief Terminate encoding a structure
			///
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void encode_struct_end() = 0;

			/// @brief Start encoding a field within a structure
			/// @param label Contains the name of the field and its label-id
			/// @param opt   Indicates whether the field is mandatory or optional
			///
			/// This must be invoked inside a `encode_struct_begin` and `encode_struct_end` pair.
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void encode_struct_field_begin(marshal_label label, marshal_optional_field opt=marshal_optional_field::MANDATORY) = 0;

			/// @brief Terminate encoding a field within a structure
			///
			/// This must be invoked inside a `encode_struct_begin` and `encode_struct_end` pair.
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void encode_struct_field_end() = 0;

			/// @brief Start encoding an array
			/// @param count Number of elements that will be encoded
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			virtual void encode_array_begin(size_t count) = 0;

			/// @brief Terminate encoding an array
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			virtual void encode_array_end() = 0;

			/// @brief Start encoding an array element
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			virtual void encode_array_element_begin() = 0;

			/// @brief Terminate encoding an array element
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			virtual void encode_array_element_end() = 0;

			/// @brief Start encoding a dictionary
			/// @param count Number of elements that will be encoded
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			virtual void encode_dictionary_begin(size_t count) = 0;

			/// @brief Terminate encoding a dictionary
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			virtual void encode_dictionary_end() = 0;

			/// @brief Start encoding a dictionary element
			/// @param key UTF-8 encoded string key that will be encoded along with the object
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			virtual void encode_dictionary_element_begin(const std::string& key) = 0;

			/// @brief Terminate encoding a dictionary element
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			virtual void encode_dictionary_element_end() = 0;

			/// @brief Start encoding a typed object
			/// @param label Contains the name of the type and its label-id
			/// @param extensible If true, the object is encoded in way that allows skipping it during decoding.
			///
			/// See @link marshaling_typeds documentation @endlink for details and examples.
			virtual void encode_typed_begin(marshal_label label, bool extensible) = 0;

			/// @brief Terminate encoding a typed object
			///
			/// See @link marshaling_typeds documentation @endlink for details and examples.
			virtual void encode_typed_end() = 0;

		protected:
			/// @brief Encode fixed-size, known in advance, raw binary data
			/// @param data Raw data
			/// @param length Length of the raw data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void internal_encode_binary(const void* data, size_t length, uint32_t suggestions=0) = 0;

			/// @brief Encode variably sized, raw binary data
			/// @param data Raw data
			/// @param length Length of the raw data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			/// @note The encoder must encode the data in way that allows the decoder do deduce its length at runtime.
			///       This includes a length field, some kind of terminator in the encoding or anything else suitable.
			virtual void internal_encode_varsize_binary(const void* data, size_t length, uint32_t suggestions=0) = 0;
	};

	// Template for encoding integral types
	template<marshal_integral_types TYPE>
	void marshal_enc::encode(TYPE value, uint32_t suggestions)
	{
		if constexpr (std::is_same_v<TYPE, bool>) {
			encode_bool(static_cast<bool>(value), suggestions);
		}
		else if constexpr (std::is_signed_v<TYPE>) {
			if constexpr (sizeof(TYPE) == 1) encode_i8(static_cast<int8_t>(value), suggestions);
			else if constexpr (sizeof(TYPE) == 2) encode_i16(static_cast<int16_t>(value), suggestions);
			else if constexpr (sizeof(TYPE) == 4) encode_i32(static_cast<int32_t>(value), suggestions);
			else if constexpr (sizeof(TYPE) == 8) encode_i64(static_cast<int64_t>(value), suggestions);
			// Not working on g++-11
			//else static_assert(false, "UNSUPPORTED TYPE");
		}
		else {
			if constexpr (sizeof(TYPE) == 1) encode_u8(static_cast<uint8_t>(value), suggestions);
			else if constexpr (sizeof(TYPE) == 2) encode_u16(static_cast<uint16_t>(value), suggestions);
			else if constexpr (sizeof(TYPE) == 4) encode_u32(static_cast<uint32_t>(value), suggestions);
			else if constexpr (sizeof(TYPE) == 8) encode_u64(static_cast<uint64_t>(value), suggestions);
			// Not working on g++-11
			//else static_assert(false, "UNSUPPORTED TYPE");
		}
	}

} // namespace dastd
