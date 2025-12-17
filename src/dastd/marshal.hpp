/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 23-AUG-2023
*
* @brief Master include for the marshaling libraries
**/

/*!
	@page marshaling_main Marshaling subsystem
	The action of "marshaling" is the ability of converting memory representations
	of data into a streamed form and viceversa.
	Marshaling allows the exchange of data and persistence among different system and
	with different encodings.
	This library gives a consistent interface that allows supporting verbose and
	not stricly ordered encodings like JSON

	@subsection marshaling_suggestions Marshaling 'suggestion' field
	The `suggestion` parameter is designed to allow the encoder/decoder to optimize
	the encoding.

	If the given encoder/decoder pair does not support a suggestion, it will be
	safely ignored.

	Example:

	@code{.cpp}

	// Encode an uint32_t value that can be at most 24 bits long and
	// increasing from zero (i.e. most likely small, not random values).
	encoder.encode_u32(myVal, marshal_suggest_limit_bits(24)|marshal_suggest_increasing);

	@endcode

	See @ref dastd::marshal_suggest_limit_bits and @ref dastd::marshal_suggest_increasing


	@subsection marshaling_structs Marshaling structures
	Structures are sequences of named, ordered and typed fields that can be encoded
	and decoded.
	When encoding, the exact fixed sequence of fields must be given.
	When decoding, the fields may come in any order and contain missing, dupe or
	unexpected fields.

	Structures can be declared as 'extensible': this means that the encoder supports
	fields subsets. This means that given an ordered sequence of F1,F2,...,Fn fields,
	the encoder can manage shorter sequences F1,F2,...,Fm with m<n, and longer sequence,
	i.e. with additional unknown fields added at the end.

	@code{.cpp}

	// Encode a structure (true == extensible)
	encoder.encode_struct_begin(true);

	// Start encoding of field 'field1'
	// The 'dastd_marshal_label' macro will statically encode
	// the hash value.
	encoder.encode_struct_field_begin(dastd_marshal_label("field1"));

	// Encode field 'field1' body
	encoder.encode_u32(data.field1);

	// End encoding of field 'field1'
	encoder.encode_struct_field_end();


	// Start encoding of field 'field2' as optional.
	// In this case, the value is missing
	encoder.encode_struct_field_begin(dastd_marshal_label("field2"), dastd::marshal_optional_field::OPTIONAL_MISSING);

	// End encoding of field 'field2'; no content is allowed since the field has been
	// encoded as "missing"
	encoder.encode_struct_field_end();


	// Start encoding of field 'field3' as optional.
	// In this case, the value is present
	encoder.encode_struct_field_begin(dastd_marshal_label("field3"), dastd::marshal_optional_field::OPTIONAL_PRESENT);

	// Encode field 'field1' body
	encoder.encode_u32(data.field3);

	// End encoding of field 'field1'
	encoder.encode_struct_field_end();


	// ...encode other fields

	// End a structure encoding
	encoder.encode_struct_end();

	@endcode

	When decoding, the sequence of expected fields is to be declared in advance. This
	allows the decoder to maintain a consistent interface no matter if the field
	names are included in the encoded data (as it happens with JSON) or the fields
	are binary encoded in sequence without labeling them.

	@code{.cpp}

	// The `false` parameter indicates that the field is not optional, i.e. mandatory.
	constexpr marshal_label_info_t struct_fields[] = {
		marshal_label_info_calc("myField", true),
		marshal_label_info_calc("pippo", false),
	};
	constexpr size_t struct_fields_count = sizeof(struct_fields)/sizeof(struct_fields[0]);

	// Start the decoding of a structure (true == extensible).
	// The decoder is informed of the expected sequence of fields.
	decoder.decode_struct_begin(true, struct_fields, struct_fields_count);

	for(;;) {
		bool field_is_present;

		// Request the decoding of the next field. The `field_is_present` flag
		// will be set to `false` if the field is optional and marked "missing".
		marshal_label_id_t label_id = decoder.decode_struct_field_begin(&field_is_present);

		// If returns 'marshal_label_id_INVALID', it means there are no more fields.
		if (label_id == dastd::marshal_label_id_INVALID) break;

		// Decode the fields according to the `label_id`
		switch(label_id) {
			case marshal_label::const_hash("myField"): myField.decode(decoder); break;
			case marshal_label::const_hash("pippo"): pippo.decode(decoder); break;

			// In case the field is unexpected, it can be skipped.
			// Note that not all decoders support skipping unknown fields:
			// if not supported, this will throw an exception.
			default: decoder.decode_struct_field_skip();
		}

		// Terminate the decoding of the field
		decoder.decode_struct_field_end();
	}

	// End a structure decoding
	decoder.decode_struct_end();

	@endcode


	@subsection marshaling_arrays Marshaling arrays
	Arrays are sequences of a variable number of objects of the same type.

	When encoding, the number of elements is to be specified in advance:

	@code{.cpp}

	// The number of elements that are about to be encoded must be
	// declared at the beginning.
	encoder.encode_array_begin(myArray.size());
	for(i=0; i<myData.size(); i++) {
		encoder.encode_array_element_begin();
		myArray[i].encode(encoder);
		encoder.encode_array_element_end();
	}
	encoder.encode_array_end();

	@endcode

	When decoding, the decoder might hint the number of following
	items or return `dastd::marshal_array_SIZE_UNKNOWN` if the size
	is unknown. For example, JSON arrays will return that the size
	is not known.

	@code{.cpp}

	size_t noOfElements = decoder.decode_array_begin();
	if (noOfElements != dastd::marshal_array_SIZE_UNKNOWN) {
		myArray.reserve(noOfElements);
	}
	// Keep looping
	while(decoder.decode_array_element_begin()) {
		myArray.push_back().decode(decoder);
		decoder.decode_array_element_end();
	}
	// If the `noOfElements` hint was returned, it must match
	// the number of elements decoded.
	assert((noOfElements == dastd::marshal_array_SIZE_UNKNOWN) || (myArray.size() == noOfElements));

	@endcode

	@subsection marshaling_dictionaries Marshaling dictionarys
	Dictionaries are sequences of a variable number of (key,object) pairs of the same type.
	Keys are required to be strings.

	When encoding, the number of elements is to be specified in advance:

	@code{.cpp}


	// The number of elements that are about to be encoded must be
	// declared at the beginning.
	encoder.encode_dictionary_begin(myDictionary.size());
	for(i=0; i<myDictionary.size(); i++) {
		encoder.encode_dictionary_element_begin(myDictionary[i].key);
		myDictionary[i].value.encode(encoder);
		encoder.encode_dictionary_element_end();
	}
	encoder.encode_dictionary_end();

	@endcode

	When decoding, the decoder might hint the number of following
	items or return `dastd::marshal_array_SIZE_UNKNOWN` if the size
	is unknown. For example, JSON dictionarys will return that the size
	is not known.

	@code{.cpp}

	std::string key;
	size_t noOfElements = decoder.decode_dictionary_begin();
	if (noOfElements != dastd::marshal_array_SIZE_UNKNOWN) {
		// Prepare the dictionary to receive `noOfElements` elements.
		// This action is not mandatory: it can be used to optimize the allocations.
		// myDictionary.reserve(noOfElements);
	}
	// Keep looping
	while(decoder.decode_dictionary_element_begin(key)) {
		auto ret = myDictionary.emplace(
			std::piecewise_construct,
		  std::forward_as_tuple(key),
			std::forward_as_tuple()
		);
		if (ret.second) ret.first.decode(decoder);
		decoder.decode_dictionary_element_end();
	}
	// If the `noOfElements` hint was returned, it must match
	// the number of elements decoded.
	assert((noOfElements == dastd::marshal_array_SIZE_UNKNOWN) || (myDictionary.size() == noOfElements));

	@endcode


	@subsection marshaling_typeds Marshaling polymorphic objects
	Polymorphic objects are elements that have different encodings according to their type.
	Types are represented by a `dastd::marshal_label` which is stored in the
	encoded data.
	If the encoding is declared "expandable", unknown types can be skipped when decoding.
	Otherwise, unknown types will cause the abortion of the decoding. In order to allow
	skipping, some more compact encodings will have to include the information that allows
	skipping the data (for example, the length of the encoded record).

	Encoding example:
	@code{.cpp}

	// The "true" parameter means that it is expandable
	encoder.encode_typed_begin(dastd_marshal_label("TMyType"), true);
	obj.encode(encoder);
	encoder.encode_typed_end();

	@endcode

	Decoding example:
	@code{.cpp}

	// The "true" parameter means that it is expandable
	marshal_label_id_t type = decoder.decode_typed_begin(true);
	switch(type) {
		case marshal_label::const_hash("TMyType"): f=new TMyType; f->decode(decoder); break;
		// Skip unknown types. This can be done only if "expandable"
		// has been set to true.
		default: decoder.decode_typed_end_skip();
	}
	decoder.decode_typed_end();

	@endcode

	Note that this system allows encoding a null object: just indicate a
	conventional name for the type and call "encode_typed_end()" without
	encoding anything. In case of detection of null objects, it will
	return `marshal_label_id_INVALID`.

**/
#pragma once
#include "defs.hpp"
#include "exception.hpp"
#include "hash_crc32.hpp"
#include "fmt.hpp"
#include "uuid.hpp"
#include <cassert>

namespace dastd {
	/// @brief Marshaling option for integrals
	///
	/// Tells the encoder/decoder that the numeric value will be
	/// limited within the indicated number of bits.
	/// For example, a uint32_t value could be limited to 28 bits.
	///
	/// @param no_of_bits Number of bits; max is 63; specifing 0, means unlimited
	/// @return Returns a suggesion value that can be or-ed with other "marshal_suggest".
	constexpr uint32_t marshal_suggest_limit_bits(uint32_t no_of_bits) {assert(no_of_bits < 64); return no_of_bits;}

	/// @brief Suggest the integral is increasing
	///
	/// This suggestion can be used if the number is a value that
	/// starts from zero and it is increased. This makes more likely
	/// to find low values than high values, allowing an encoding
	/// that privileges smaller number of bits.
	constexpr uint32_t marshal_suggest_increasing = 0b01000000;

	/// @brief Base class for the marshaling encoder/decoder
	class marshal {
		public:
			/// @brief Virtual destructor
			virtual ~marshal() {}
	};

	/// @brief Label-id hash type; usually the CRC-32 of the string
	using marshal_label_id_t = uint32_t;

	/// @brief Invalid label id value
	constexpr marshal_label_id_t marshal_label_id_INVALID = 0;

	/// @brief Value returned when array size is unknown
	constexpr size_t marshal_array_SIZE_UNKNOWN = ((size_t)-1);


	/// @brief Class used to represent a name in the encoding
	///
	/// Names are 8-bit ASCIIZ strings that can be encode as
	/// strings or with a numeric hash value.
	struct marshal_label {
		/// @brief Empty constructor
		marshal_label(): m_label_text(nullptr), m_label_id(marshal_label_id_INVALID) {}

		/// @brief Constructor with both text and hash
		marshal_label(const char* label_text, marshal_label_id_t label_id): m_label_text(label_text), m_label_id(label_id) {assert(label_id == hash(label_text));}

		/// @brief Constructor with both text and hash
		marshal_label(const char* label_text): m_label_text(label_text) {calc_hash();}

		/// @brief Pointer to a constant ASCIIZ string containing the label text
		const char* m_label_text;

		/// @brief Hash of the label text calculated with the "hash" function below
		marshal_label_id_t m_label_id;

		/// @brief Returns true if valid
		bool valid() const {return (m_label_text != nullptr) && (m_label_id != marshal_label_id_INVALID);}

		/// @brief Print the label
		void print(std::ostream& o) const {if (valid()) o << "0x" << fmt(m_label_id, 16, 8, true) << "-[" << m_label_text << "]"; else o << "0x00000000 INVALID";}

		/// @brief Function that calculates the hash
		static marshal_label_id_t hash(const char* label_text) {return (label_text==nullptr ? marshal_label_id_INVALID : (marshal_label_id_t)crc32(label_text));}

		/// @brief Function that calculates the hash
		static marshal_label_id_t hash(const std::string& label_text) {return (marshal_label_id_t)crc32(label_text);}

#if DASTD_CPP_VER >= 20
		/// @brief Function that calculates the hash working on const values
		///
		/// It can be used for static assigments:
		///
		///     static constexpr marshal_label_id_t myFieldId = dastd::marshal_label::const_hash("myField");
		///     static_assert(myFieldId == 0x07363313);
		///
		static consteval marshal_label_id_t const_hash(const char label_text[]) {return (marshal_label_id_t)const_crc32(label_text);}
#endif

		/// @brief Calculate and save the hash according to m_label_text
		void calc_hash() {m_label_id = hash(m_label_text);}
	};

	DASTD_DEF_OSTREAM(marshal_label);


	/// @exception dastd::exception_marshal
	/// @brief Base exception for marshal execution
	DASTD_DEF_EXCEPTION(exception_marshal)


	template <class T>
	concept marshal_integral_types = std::is_integral_v<T> || std::is_enum_v<T>;
}

/// @brief Creates a statically calculated marshal_label
#define dastd_marshal_label(label_text) dastd::marshal_label(label_text, dastd::marshal_label::const_hash(label_text))

