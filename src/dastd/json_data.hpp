/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 01-MAY-2024
*
* JSON genericd data representation
**/
#pragma once
#include "defs.hpp"
#include "fmt_string.hpp"
#include "char32string.hpp"
#include "rtti.hpp"
#include "ostream_indent.hpp"
#include "json_encoder.hpp"
#include "json_tokenizer.hpp"
#include "multinum.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <map>

namespace dastd {

/// @brief Base class for JSON elements
///
/// The goal of this class and its derivatives is to have an usable
/// C++ structure representing a JSON record.
/// A `json_data` can be built by decoding a JSON string or encoded
/// into a JSON string.
class json_data {
    DASTD_RTTI_BASE(json_data)
	public:
		/// @brief Virtual destructor
		virtual ~json_data() {}

		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const = 0;

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const = 0;
};
DASTD_DEF_OSTREAM(json_data)

/// @brief Base class for terminal values
class json_data_value: public json_data {
    DASTD_RTTI_DERIVED(json_data_value, json_data)
	public:
		/// @brief Get the associated multinum, if this entry can be converted to a multinum
		///
		/// For example, string "  1234 " can be considered number 1234.
		virtual multinum get_multinum() const {return multinum();}
};

/// @brief Class for string terminal values
class json_data_value_string: public json_data_value {
    DASTD_RTTI_DERIVED(json_data_value_string, json_data_value)
	public:
		/// @brief Constructor empty
		json_data_value_string() {}

		/// @brief Constructor with initial value
		json_data_value_string(const std::u32string& value): m_value(value) {}

		/// @brief Constructor with initial value UTF-8 encoded
		json_data_value_string(const std::string& value_utf8): m_value(value_utf8) {}

		/// @brief Associated string
		char32string m_value;

		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const override {o << fmt_cq(m_value);}

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const override {o << '"'; json_encode_string(o, m_value); o << '"';}

		/// @brief Get the associated multinum, if this entry can be converted to a multinum
		///
		/// For example, string "  1234 " can be considered number 1234.
		virtual multinum get_multinum() const override {multinum m; m.parse(m_value); return m;}
};

/// @brief Class for boolean terminal values
class json_data_value_boolean: public json_data_value {
    DASTD_RTTI_DERIVED(json_data_value_boolean, json_data_value)
	public:
		/// @brief Constructor with initial value
		json_data_value_boolean(bool value=false): m_value(value) {}

		/// @brief Associated string
		bool m_value;

		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const override {o << (m_value ? "true" : "false");}

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const override {o << (m_value ? "true" : "false");}

		/// @brief Get the associated multinum, if this entry can be converted to a multinum
		///
		/// For example, string "  1234 " can be considered number 1234.
		virtual multinum get_multinum() const override {return multinum(m_value ? 1 : 0);}
};

/// @brief Class for null terminal values
class json_data_value_null: public json_data_value {
    DASTD_RTTI_DERIVED(json_data_value_null, json_data_value)
	public:
		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const override {o << "null";}

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const override {o << "null";}

		/// @brief Get the associated multinum, if this entry can be converted to a multinum
		///
		/// For example, string "  1234 " can be considered number 1234.
		virtual multinum get_multinum() const override {return multinum(0);}
};

/// @brief Class for double terminal values
class json_data_value_number: public json_data_value {
    DASTD_RTTI_DERIVED(json_data_value_number, json_data_value)
	public:
		/// @brief Constructor
		json_data_value_number(const multinum& value): m_value(value) {}

		/// @brief Constructor
		json_data_value_number() {}

		/// @brief Associated value
		multinum m_value;

		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const override {o << m_value;}

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const override {o << m_value;}

		/// @brief Get the associated multinum, if this entry can be converted to a multinum
		///
		/// For example, string "  1234 " can be considered number 1234.
		virtual multinum get_multinum() const override {return m_value;}
};

/// @brief Base class for objects (structures)
class json_data_object: public json_data {
    DASTD_RTTI_DERIVED(json_data_object, json_data)
	private:
		/// @brief Position counter; used to keep track of the order of insertion
		size_t m_insertion_counter = 0;

		/// @brief Members
		std::map<char32string, std::pair<size_t, std::shared_ptr<json_data>>> m_members;

		/// @brief Type of the iterator of the m_members map
		typedef std::map<char32string, std::pair<size_t, std::shared_ptr<json_data>>>::const_iterator const_map_iterator;

	public:
		/// @brief Add a member created externally
		template<class DATA_TYPE>
		std::shared_ptr<DATA_TYPE> add(const std::u32string& name, std::shared_ptr<DATA_TYPE> body);

		/// @brief Create and add a member
		template<class DATA_TYPE, typename... Targs>
		std::shared_ptr<DATA_TYPE> add_new(const std::u32string& name, Targs... Fargs);

		/// @brief Create and add a member
		template<class DATA_TYPE, typename... Targs>
		std::shared_ptr<DATA_TYPE> add_new(const std::string& name_utf8, Targs... Fargs) {return add_new<DATA_TYPE>(char32string(name_utf8), Fargs...);}

		/// @brief Fill a vector of pairs {key,value} ordered by insertion order
		void get_members(std::vector<std::pair<std::u32string, std::shared_ptr<json_data>>>& members) const;

		/// @brief Find a member
		/// @return Returns the member or `nullptr` if not found
		///
		/// This method returns a valid entry only if found and derived from `DATA_TYPE`.
		///
		/// Example:
		///
		///     auto myField = obj.find<json_data_value_string>("myField");
		///     if (myField) cout << "Found: " << *myField << endl;
		///     else cout << "Not found" << endl;
		///
		template<class DATA_TYPE>
		std::shared_ptr<DATA_TYPE> find(const std::u32string& name) const;

		/// @brief Find a member
		template<class DATA_TYPE>
		std::shared_ptr<DATA_TYPE> find(const std::string& name_utf8) const {return find<DATA_TYPE>(char32string(name_utf8));}

		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const override;

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const override;
};

/// @brief Base class for arrays
class json_data_array: public json_data {
    DASTD_RTTI_DERIVED(json_data_array, json_data)
	public:
		/// @brief Values
		std::vector<std::shared_ptr<json_data>> m_values;

		/// @brief Add a member created externally
		template<class DATA_TYPE>
		std::shared_ptr<DATA_TYPE> add(std::shared_ptr<DATA_TYPE> value) {m_values.push_back(value); return value;}

		/// @brief Create and add a member
		template<class DATA_TYPE, typename... Targs>
		std::shared_ptr<DATA_TYPE> add_new(Targs... Fargs) {std::shared_ptr<DATA_TYPE> value = std::make_unique<DATA_TYPE>(Fargs...); m_values.push_back(value); return value;}

		/// @brief Print the content without terminating with a newline
		virtual void print(std::ostream& o) const override;

		/// @brief Encode this object in JSON format onto the indicate stream
		virtual void encode_json(std::ostream& o) const override;
};

/// @brief Reads a sequence of characters from `source` and returns a `json_data`
/// @tparam CHARTYPE Type of the character of the stream
/// @param source    Source stream containing the encoded JSON
/// @param errorText Stream that will receive an error description in caso of error
/// @param flags     See the "FLAGS" section in `json_tokenizer.hpp`
/// @return          Return a nullptr in case of decoding errors
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode(source_with_peek<CHARTYPE>& source, std::ostream& errorText, uint32_t flags=0);

//------------------------------------------------------------------------------
// (brief) Print the content without terminating with a newline
//------------------------------------------------------------------------------
inline void json_data_object::print(std::ostream& o) const
{
	bool first = true;
	o << "{";
	ostream_indent ind(o);
	std::vector<std::pair<std::u32string, std::shared_ptr<json_data>>> members;
	get_members(members);
	for(auto& member: members) {
		if (first) first=false;
		else o << ",";
		ind << std::endl << fmt32_cq(member.first) << ": " << *member.second.get();
	}
	o << std::endl << "}";
}

//------------------------------------------------------------------------------
// (brief) Encode this object in JSON format onto the indicate stream
//------------------------------------------------------------------------------
inline void json_data_object::encode_json(std::ostream& o) const
{
	bool first = true;
	o << "{";
	std::vector<std::pair<std::u32string, std::shared_ptr<json_data>>> members;
	get_members(members);
	for(auto& member: members) {
		if (first) first=false;
		else o << ",";
		o << '"';
		json_encode_string(o, member.first);
		o << "\":";
		member.second->encode_json(o);
	}
	o << "}";
}

//------------------------------------------------------------------------------
// (brief) Add a member
//------------------------------------------------------------------------------
template<class DATA_TYPE>
std::shared_ptr<DATA_TYPE> json_data_object::add(const std::u32string& name, std::shared_ptr<DATA_TYPE> value)
{
	m_members.insert({name, {m_insertion_counter, value}});
	m_insertion_counter++;
	return value;
}

//------------------------------------------------------------------------------
// (brief) Create and add a member
//------------------------------------------------------------------------------
template<class DATA_TYPE, typename... Targs>
std::shared_ptr<DATA_TYPE> json_data_object::add_new(const std::u32string& name, Targs... Fargs)
{
	std::shared_ptr<DATA_TYPE> value = std::make_unique<DATA_TYPE>(Fargs...);
	return add(name, value);
}

//------------------------------------------------------------------------------
// (brief) Find a member
//------------------------------------------------------------------------------
template<class DATA_TYPE>
std::shared_ptr<DATA_TYPE> json_data_object::find(const std::u32string& name) const
{
	auto iter = m_members.find(name);
	if (iter == m_members.end()) return nullptr;
	if (iter->second.second->derives_from(DATA_TYPE::s_class_id)) return std::static_pointer_cast<DATA_TYPE>(iter->second.second);
	return nullptr;
}

//------------------------------------------------------------------------------
// (brief) Fill a vector of pairs {key,value} ordered by insertion order
//------------------------------------------------------------------------------
inline void json_data_object::get_members(std::vector<std::pair<std::u32string, std::shared_ptr<json_data>>>& members) const
{
	members.clear();
	std::vector< const_map_iterator > members_iterators;
	members_iterators.reserve(m_members.size());

	for(auto iter=m_members.begin(); iter!=m_members.end(); iter++) {
		members_iterators.push_back(iter);
	}

	std::sort(members_iterators.begin(), members_iterators.end(), [](const_map_iterator i1, const_map_iterator i2)->bool {
		return i1->second.first < i2->second.first;
	});

	for(auto& members_iterator: members_iterators) {
		members.push_back({members_iterator->first, members_iterator->second.second});
	}
}

//------------------------------------------------------------------------------
// (brief) Print the content without terminating with a newline
//------------------------------------------------------------------------------
inline void json_data_array::print(std::ostream& o) const
{
	o << "[";
	ostream_indent ind(o);
	bool first=true;
	for(auto& value: m_values) {
		if (first) first=false;
		else ind << ",";
		ind << std::endl << *value.get();
	}
	o << std::endl << "]";
}

//------------------------------------------------------------------------------
// (brief) Encode this object in JSON format onto the indicate stream
//------------------------------------------------------------------------------
inline void json_data_array::encode_json(std::ostream& o) const
{
	o << "[";
	bool first=true;
	for(auto& value: m_values) {
		if (first) first=false;
		else o << ",";
		value->encode_json(o);
	}
	o << "]";
}

//------------------------------------------------------------------------------
// Executes the decoding of an unknonw new json entity
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode_unknown(json_tokenizer_sourced<CHARTYPE>& tokenizer, std::ostream& errorText, bool fetch_new_token);


//------------------------------------------------------------------------------
// Report an error
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode_report_error(const json_tokenizer_sourced<CHARTYPE>& tokenizer, std::ostream& errorText, const std::string& errorDesc)
{
	errorText << "While processing token " << fmt_cq(tokenizer.get_raw_token()) << ", got error: " << errorDesc;
	return nullptr;
}

//------------------------------------------------------------------------------
// Executes the decoding of an object
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode_object(json_tokenizer_sourced<CHARTYPE>& tokenizer, std::ostream& errorText)
{
	bool first_time = true;
	json_tokenizer_ret tokenizer_ret;
	std::u32string field_name;

	std::shared_ptr<json_data_object> obj = std::make_unique<json_data_object>();

	for (;;) {
		// Read a token.
		// The first time, it must be either a field name (string) or a closed brace
		// The other times, it must be either a comma or a closed brace
		tokenizer_ret = tokenizer.fetch_token();

		if (tokenizer_ret == json_tokenizer_ret::C_BRACE_CLOSE) return obj;
		if (first_time) {
			first_time = false;
		}
		else {
			#if 0
			if (tokenizer_ret != json_tokenizer_ret::C_COMMA) return json_data_decode_report_error(tokenizer, errorText, "decoding object, expected a comma");
			tokenizer_ret = tokenizer.fetch_token();
			#else
			// Relaxed version: if the comma is missing, nevermind
			if (tokenizer_ret == json_tokenizer_ret::C_COMMA) tokenizer_ret = tokenizer.fetch_token();
			#endif
		}

		if (tokenizer_ret != json_tokenizer_ret::C_STRING) return json_data_decode_report_error(tokenizer, errorText, "decoding object, expected a field name");

		field_name = tokenizer.get_string();

		// Read a token. It must be a colon
		tokenizer_ret = tokenizer.fetch_token();
		if (tokenizer_ret != json_tokenizer_ret::C_COLON) return json_data_decode_report_error(tokenizer, errorText, "decoding object, expected the colon ':' after the field name");;

		// Read the object field
		std::shared_ptr<json_data> field_body = json_data_decode_unknown<CHARTYPE>(tokenizer, errorText, true);
		if (!field_body) return nullptr;

		// Add the field
		obj->add(field_name, field_body);
	}
}

//------------------------------------------------------------------------------
// Executes the decoding of an array
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode_array(json_tokenizer_sourced<CHARTYPE>& tokenizer, std::ostream& errorText)
{
	bool first_time = true;
	json_tokenizer_ret tokenizer_ret;
	std::shared_ptr<json_data_array> obj = std::make_unique<json_data_array>();

	for (;;) {
		// Read a token. It must be either a comma or a closed bracket
		tokenizer_ret = tokenizer.fetch_token();

		if (tokenizer_ret == json_tokenizer_ret::C_BRACKET_CLOSE) return obj;
		if (first_time) {
			first_time = false;
		}
		else {
			#if 0
			if (tokenizer_ret != json_tokenizer_ret::C_COMMA) return json_data_decode_report_error(tokenizer, errorText, "decoding array, expected a comma");;
			tokenizer_ret = tokenizer.fetch_token();
			#else
			// Relaxed version: if the comma is missing, nevermind
			if (tokenizer_ret == json_tokenizer_ret::C_COMMA) tokenizer_ret = tokenizer.fetch_token();
			#endif
		}
		// Read the object field
		std::shared_ptr<json_data> field_body = json_data_decode_unknown<CHARTYPE>(tokenizer, errorText, false);
		if (!field_body) return nullptr;

		// Add the field
		obj->add(field_body);
	}
}

//------------------------------------------------------------------------------
// Executes the decoding of an unknonw new json entity
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode_unknown(json_tokenizer_sourced<CHARTYPE>& tokenizer, std::ostream& errorText, bool fetch_new_token)
{
	json_tokenizer_ret tokenizer_ret;
	if (fetch_new_token) tokenizer_ret = tokenizer.fetch_token();
	else tokenizer_ret = tokenizer.get_last_process_ret();

	switch(tokenizer_ret) {
		// Start a new object
		case json_tokenizer_ret::C_BRACE_OPEN: return json_data_decode_object(tokenizer, errorText);

		// Start a new array
		case json_tokenizer_ret::C_BRACKET_OPEN: return json_data_decode_array(tokenizer, errorText);

		// Terminal: true
	  case json_tokenizer_ret::C_TRUE: return std::make_unique<json_data_value_boolean>(true);

		// Terminal: false
	  case json_tokenizer_ret::C_FALSE: return std::make_unique<json_data_value_boolean>(false);

		// Terminal: null
	  case json_tokenizer_ret::C_NULL: return std::make_unique<json_data_value_null>();

		// Terminal: string
	  case json_tokenizer_ret::C_STRING: return std::make_unique<json_data_value_string>(tokenizer.get_string());

		// Terminal: number
	  case json_tokenizer_ret::C_NUMBER: return std::make_unique<json_data_value_number>(tokenizer.get_multinum());

		// Terminal: string that can be consisdered a number
	  case json_tokenizer_ret::C_STRING_AND_NUMBER: return std::make_unique<json_data_value_string>(tokenizer.get_string());

		// Other cases: failure
		default: return json_data_decode_report_error(tokenizer, errorText, "unknown JSON element");;
	}
}

//------------------------------------------------------------------------------
// Reads a sequence of characters from `source` and returns a `json_data`
//------------------------------------------------------------------------------
template<class CHARTYPE>
std::shared_ptr<json_data> json_data_decode(source_with_peek<CHARTYPE>& source, std::ostream& errorText, uint32_t flags)
{
	json_tokenizer_sourced<CHARTYPE> tokenizer(source, flags);
	return json_data_decode_unknown(tokenizer, errorText, true);
}



} // namespace dastd
