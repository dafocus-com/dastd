/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
**/
#pragma once
#include "marshal.hpp"

namespace dastd {

	/// @brief Describes a marshal_label_id_t with additional information
	///
	/// This type is used to describe a label to the decoder when it is about
	/// to decode the fields of a structure. It contains a `marshal_label_id_t`
	/// in it lower 32-bits, while the upper 32-bits contain a bit telling
	/// whether the field is expected to be optional.
	using marshal_label_info_t = uint64_t;

	#if DASTD_CPP_VER >= 20
	constexpr uint64_t marshal_label_info_optional_flag = 0x100000000UL;

	/// Calculates the marshal_label_info_t given a label_id and optional flag
	consteval marshal_label_info_t marshal_label_info_calc(marshal_label_id_t label_id, bool optional=false) {
		return (optional ? marshal_label_info_optional_flag : 0x0UL) | (marshal_label_info_t)(label_id);
	}

	/// Calculates the marshal_label_info_t given a label_id and optional flag
	consteval marshal_label_info_t marshal_label_info_calc(const char label_text[], bool optional=false) {
		return (optional ? marshal_label_info_optional_flag : 0x0UL) | (marshal_label_info_t)(marshal_label::const_hash(label_text));
	}

	/// Returns true if the field is optional
	inline bool marshal_label_info_is_optional(marshal_label_info_t label_info) {return (label_info & marshal_label_info_optional_flag) == marshal_label_info_optional_flag;}

	/// Returns true if the field_id from a marshal_label_info_t
	inline marshal_label_id_t marshal_label_info_to_id(marshal_label_info_t label_info) {return (marshal_label_id_t)(label_info & 0xFFFFFFFFULL);}
	#endif


	/// @brief Base class for the marshaling decoder
	///
	/// This class provides an interface that allows marshaling basic
	/// types. The derived classes will provide with specific
	/// streaming methods.
	class marshal_dec: public marshal {
		public:
			/// @brief Template for decoding integral types
			/// @tparam TYPE Integral type
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			template<marshal_integral_types TYPE>
			TYPE decode(uint32_t suggestions=0);

			/// @brief Decode a bool
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual bool decode_bool(uint32_t suggestions=0) = 0;

			/// @brief Decode a uint8_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual uint8_t decode_u8(uint32_t suggestions=0) = 0;

			/// @brief Decode a int8_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual int8_t decode_i8(uint32_t suggestions=0) = 0;

			/// @brief Decode a uint16_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual uint16_t decode_u16(uint32_t suggestions=0) = 0;

			/// @brief Decode a int16_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual int16_t decode_i16(uint32_t suggestions=0) = 0;

			/// @brief Decode a uint32_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual uint32_t decode_u32(uint32_t suggestions=0) = 0;

			/// @brief Decode a int32_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual int32_t decode_i32(uint32_t suggestions=0) = 0;

			/// @brief Decode a uint64_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual uint64_t decode_u64(uint32_t suggestions=0) = 0;

			/// @brief Decode a int64_t
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual int64_t decode_i64(uint32_t suggestions=0) = 0;

			/// @brief Decode a 64-bit floating point
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual double decode_f64(uint32_t suggestions=0) = 0;

			/// @brief Decode a 128-bit UUID
			/// @return Return the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual uuid decode_uuid(uint32_t suggestions=0) = 0;

			/// @brief Decode a std::string (UTF-8)
			/// @param value Receives the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void decode_string_utf8(std::string& value, uint32_t suggestions=0) = 0;

			/// @brief Decode a std::u32string
			/// @param value Receives the decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void decode_u32string(std::u32string& value, uint32_t suggestions=0) = 0;

			/// @brief Decode raw binary data of a known and fixed length
			/// @param buffer Receives the raw decoded data
			/// @param length Number of bytes expected
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			void decode_binary(void* buffer, size_t length, uint32_t suggestions=0) {internal_decode_binary(buffer, length, suggestions);}

			/// @brief Decode raw binary data of a known and fixed length into a std::string
			/// @param value Receives the raw decoded data
			/// @param length Number of bytes expected
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			void decode_binary(std::string& value, size_t length, uint32_t suggestions=0);

			/// @brief Decode raw binary data of a variable length into a std::string
			/// @param value Receives the raw decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			void decode_varsize_binary(std::string& value, uint32_t suggestions=0);

			/// @brief Start Decoding a structure
			/// @param extensible If true, the structure will be encoded so more or less fields can be expected.
			/// @param field_infos Array with the label-ids of the expected fields in their correct order and a marker telling if that field is optional.
			/// @param field_infos_count Number of elements to be expected in the field_ids array.
			///
			/// See @link marshaling_structs documentation @endlink for details and examples.
			/// 
			/// The `field_infos` part is to be statically defined. Example:
			/// 
			/// 			static constexpr char k_marshal_attr_name_timestamp[] = "timestamp";
			/// 			static constexpr char k_marshal_attr_name_frame_num[] = "frame_num";
			/// 			static constexpr char k_marshal_attr_name_center_freq[] = "center_freq";
			/// 			
			/// 			static constexpr dastd::marshal_label_info_t k_struct_fields[] = {
			/// 				dastd::marshal_label_info_calc(k_marshal_attr_name_timestamp, false),
			/// 				dastd::marshal_label_info_calc(k_marshal_attr_name_frame_num, false),
			/// 				dastd::marshal_label_info_calc(k_marshal_attr_name_center_freq, false),
			/// 			};
			/// 			
			/// 			static constexpr size_t k_struct_fields_count = sizeof(k_struct_fields)/sizeof(k_struct_fields[0]);
			/// 
			virtual void decode_struct_begin(bool extensible, const marshal_label_info_t* field_infos, size_t fields_infos_count) = 0;

			/// @brief Terminate Decoding a structure
			///
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void decode_struct_end() = 0;

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
			virtual marshal_label_id_t decode_struct_field_begin(bool* optional_present=nullptr) = 0;

			/// @brief Terminate decoding a field within a structure
			///
			/// This must be invoked inside a `decode_struct_begin` and `decode_struct_end` pair.
			/// See @link marshaling_structs documentation @endlink for details and examples.
			virtual void decode_struct_field_end() = 0;

			/// @brief Decodes and discard an unknown field
			/// @return Returns `true` if skipped, `false` if this feature is not available
			virtual bool decode_struct_field_skip() {return false;}

			/// @brief Start decoding an array
			/// @param count Number of elements that will be decoded
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			///
			/// @return Returns the expected number of elements or `marshal_array_SIZE_UNKNOWN`
			///         if the size is unknown.
			virtual size_t decode_array_begin() = 0;

			/// @brief Terminate decoding an array
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			virtual void decode_array_end() = 0;

			/// @brief Start decoding an array element
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			///
			/// @return Returns `true` if the element is available or `false` if
			///         no more elements are available.
			virtual bool decode_array_element_begin() = 0;

			/// @brief Terminate decoding an array element
			///
			/// See @link marshaling_arrays documentation @endlink for details and examples.
			virtual void decode_array_element_end() = 0;

			/// @brief Start decoding a dictionary
			/// @param count Number of elements that will be decoded
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			///
			/// @return Returns the expected number of elements or `marshal_array_SIZE_UNKNOWN`
			///         if the size is unknown.
			virtual size_t decode_dictionary_begin() = 0;

			/// @brief Terminate decoding a dictionary
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			virtual void decode_dictionary_end() = 0;

			/// @brief Start decoding a dictionary element
			/// @param key UTF-8 encoded string key that has been encoded along with the object
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			///
			/// @return Returns `true` if the element is available or `false` if
			///         no more elements are available.
			virtual bool decode_dictionary_element_begin(std::string& key) = 0;

			/// @brief Terminate decoding a dictionary element
			///
			/// See @link marshaling_dictionaries documentation @endlink for details and examples.
			virtual void decode_dictionary_element_end() = 0;

			/// @brief Start decoding a typed object
			/// @param extensible If true, the object is decoded in way that allows skipping it during decoding.
			///
			/// See @link marshaling_typeds documentation @endlink for details and examples.
			///
			/// @return Returns the label-id of the type of the object that is following
			virtual marshal_label_id_t decode_typed_begin(bool extensible) = 0;

			/// @brief Skip the object without decoding it
			///
			/// This method will skip the object without decoding. It can be invoked only if
			/// the `extensible` parameter is `true`. Useful when the encoding data comes from
			/// a newer version that has new object types unknown to this version.
			///
			/// See @link marshaling_typeds documentation @endlink for details and examples.
			virtual void decode_typed_end_skip() = 0;

			/// @brief Terminate decoding a typed object
			///
			/// See @link marshaling_typeds documentation @endlink for details and examples.
			virtual void decode_typed_end() = 0;

			/// @brief Attempt to translate a label_id into a text
			/// @param label_id Label id to be translated
			/// @param label_text Related text
			/// @return Returns true if the translation is available
			virtual bool get_field_name(marshal_label_id_t label_id, char32string& label_text) const {DASTD_NOWARN_UNUSED(label_id); DASTD_NOWARN_UNUSED(label_text); return false;}

		protected:
			/// @brief Decode raw binary data of a known and fixed length
			/// @param buffer Receives the raw decoded data
			/// @param length Number of bytes expected
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void internal_decode_binary(void* buffer, size_t length, uint32_t suggestions=0) = 0;

			/// @brief Decode raw binary data of a variable length
			/// @param value Receives the raw decoded data
			/// @param suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
			virtual void internal_decode_varsize_binary(std::string& value, uint32_t suggestions=0) = 0;
	};

	// (brief) Decode raw binary data of a known and fixed length
	// (param) value Receives the raw decoded data
	// (param) suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
	inline void marshal_dec::decode_binary(std::string& value, size_t length, uint32_t suggestions)
	{
		// Resizing the string and writing directly into its buffer (value.data())
		// is legitimate since, starting with C++11, the strings use contiguous memory.
		value.resize(length);
		internal_decode_binary(value.data(), length, suggestions);
	}

	// (brief) Decode raw binary data of a variable length into a std::string
	// (param) value Receives the raw decoded data
	// (param) suggestions Encoding suggestions; see @link marshaling_suggestions documentation @endlink.
	inline void marshal_dec::decode_varsize_binary(std::string& value, uint32_t suggestions)
	{
		value.clear();
		internal_decode_varsize_binary(value, suggestions);
	}


	// Template for decoding integral types
	template<marshal_integral_types TYPE>
	TYPE marshal_dec::decode(uint32_t suggestions)
	{
		if constexpr (std::is_same_v<TYPE, bool>) {
			return static_cast<TYPE>(decode_bool(suggestions));
		}
		else if constexpr (std::is_signed_v<TYPE>) {
			if constexpr (sizeof(TYPE) == 1) return static_cast<TYPE>(decode_i8(suggestions));
			else if constexpr (sizeof(TYPE) == 2) return static_cast<TYPE>(decode_i16(suggestions));
			else if constexpr (sizeof(TYPE) == 4) return static_cast<TYPE>(decode_i32(suggestions));
			else if constexpr (sizeof(TYPE) == 8) return static_cast<TYPE>(decode_i64(suggestions));
			// Not working on g++-11
			//else constexpr static_assert(false, "UNSUPPORTED TYPE");
		}
		else {
			if constexpr (sizeof(TYPE) == 1) return static_cast<TYPE>(decode_u8(suggestions));
			else if constexpr (sizeof(TYPE) == 2) return static_cast<TYPE>(decode_u16(suggestions));
			else if constexpr (sizeof(TYPE) == 4) return static_cast<TYPE>(decode_u32(suggestions));
			else if constexpr (sizeof(TYPE) == 8) return static_cast<TYPE>(decode_u64(suggestions));
			// Not working on g++-11
			//else constexpr static_assert(false, "UNSUPPORTED TYPE");
		}
	}

} // namespace dastd
