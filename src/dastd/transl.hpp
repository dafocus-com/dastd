/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 30-JAN-2024
*
* This module is designed to support strings in multiple languages.
* It handles statically generated strings that each one containing the translations
* of the same sentence in multiple languages.
* The sentence is identified by its "sentence_id".
**/
#pragma once
#include "locale_id.hpp"
#include "char32string.hpp"
#include <vector>
#include <map>

namespace dastd {

/// @brief Sentence id
///
/// A sentence_id identifies a sentence that can be mapped to multiple
/// strings in different languages indicated by their locale_id.
typedef uint32_t sentence_id;

/// @brief Constant value to be used for invalid sentence IDs
constexpr sentence_id SENTENCE_ID_INVALID = 0x00000000;


/// @brief Structure used to map a sentence_id to a readable string for
///
/// Useful in tracing and debugging, maps a numeric NId to a readable string.
/// The code generators usually map the NId to a string identical to the
/// macro.
/// For example in case of:
///
///     #define GUI_hello  0x3610A686
///
/// we would have:
///
///     {0x3610A686, "GUI_hello"}
///
/// or better:
///
///     {GUI_hello, "GUI_hello"}
///
struct sentence_id_to_name {
	/// @brief Sentence id of the sentence
	sentence_id sid;

	/// @brief Textual representation of the sentence
	const char* sid_text;
};


/// @brief Single sentence text
///
/// Contains the text of one sentence in one given language.
struct sentence {
	/// @brief Sentence id of the sentence
	sentence_id sid;

	/// @brief Text of the phrase. This is an ASCIIZ string encoded in UTF-8
	const char* textUTF8;
};

/// @brief Static sentence set for one given language
///
/// Contains a set of phrases in a given language.
/// This structure is meant to be statically generated and added into a
/// `sentence_multilang_set`.
struct sentence_set {
	/// @brief Locale id; indicates the language of the sentences
	locale_id lid;

	/// @brief Number of entries in the "phrases" array
	size_t sentences_count;

	/// @brief Static array of `sentences_count` sentences sorted by `sid`
	const sentence* const sentences;

	/// @brief Given a sentence id, find the related text
	/// @param sid Sentence-id of the sentence being looked for
	/// @return Returns the pointer to the UTF-8 ASCIIZ string or nullptr if not available
	const char* find_sentence(sentence_id sid) const
	{
		const sentence* sentence_found = std::lower_bound(sentences, sentences+sentences_count, sid, [](const sentence& element, sentence_id sid) {return element.sid < sid;});
		return (sentence_found != sentences+sentences_count && sentence_found->sid == sid) ? sentence_found->textUTF8 : nullptr;
	}
};

/// @brief Static set of sentence_sets
///
/// This structure contains one or more `sentence_set` instances,
/// each one having the translation of the same sentences in various
/// languages.
struct sentence_multilang_set {
	/// @brief Pointer to the master language
	///
	/// This pointer points to the set of sentences that are used as
	/// the "master language". This language, usually English, is
	/// used to retrieve the sentences if the selected language
	/// does not have a translation for a given sentence_id.
	const sentence_set* const master_language;

	/// @brief Number of elements in the "languages" array
	size_t languages_count;

	/// @brief Array of languages sorted by their `lid`
	///
	/// NOTE: this set must include also the `master_language` that can be
	/// accessed as a regular language as well.
	const sentence_set* const * const languages;

	/// @brief Number of entries in the sentence_ids_to_names array
	size_t sentence_ids_to_names_count;

	/// @brief Array of sentence_id's with their readable names
	///
	/// This array is useful when debugging.
	const sentence_id_to_name* const sentence_ids_to_names;

	/// @brief Given a locale_id, returns the related sentence_set
	/// @return Returns the sentence_set or nullptr if not found
	const sentence_set* find_sentence_set(locale_id lid) const
	{
		auto language_found = std::lower_bound(languages, languages+languages_count, lid, [](const sentence_set* element, locale_id lid) {return element->lid < lid;});
		return (language_found != languages+languages_count && (*language_found)->lid == lid) ? (*language_found) : nullptr;
	}

	/// @brief Get a sentence text
	///
	/// @return Returns the UTF-8 ASCIIZ string in that locale; if not found, returns the master locale. If not found even there, returns nullptr
	const char* get_sentence(sentence_id sid, locale_id lid) const;

	/// @brief Get a sentence_id short descripion
	///
	/// @return Returns an ASCIIZ short string with its name for debugging purposes. Returns nullptr if not available.
	const char* get_sentence_id_name(sentence_id sid) const
	{
		if (sentence_ids_to_names == nullptr) return nullptr;
		auto name = std::lower_bound(sentence_ids_to_names, sentence_ids_to_names+sentence_ids_to_names_count, sid, [](const sentence_id_to_name& element, sentence_id sid) {return element.sid < sid;});
		return ((name != sentence_ids_to_names+sentence_ids_to_names_count) && (name->sid == sid)) ? name->sid_text : nullptr;
	}

	/// @brief Returns true if it manages the indicated sentence_id
	///
	/// This information is searched in the master language.
	bool has_sentence_id(sentence_id sid) const {return (master_language->find_sentence(sid) != nullptr);}
};

/// @brief Multiple languages manager class
///
/// Manages a set of multiple sentence_multilang_set objects.
/// This can be useful when some library has its own strings and
/// the application has another set.
/// These sets must have distinct sentence_ids.
///
/// This cla ss implements also the ability of returning a list of available
/// locale_ids that is the intersection of the locale_ids available
/// among all the sentence_multilang_sets.
/// For example, in one set has "en,it,fr" and the other "en,it,es", it will
/// return "en,it".
class multilang_manager {
	public:
		/// @brief Add a TDLocaleStringsSet to the group
		///
		/// @param string_set The `sentence_multilang_set` to be added. It will not
		///                  be owned nor deleted by this object. Thus, it can be
		///                  a static const object.
		void add(const sentence_multilang_set& string_set) {sentence_multilang_sets.push_back(&string_set); calc_locale_codes_intersection();}
		void add(const sentence_multilang_set* string_set) {add(*string_set);}

		/// @brief   Allow scanning the available locale ids
		/// @return  Returns a reference to the locale id vector
		const std::vector<locale_id>& get_locale_ids() const {return locale_ids;}

		/// @brief Get a sentence text
		///
		/// @return Returns the UTF-8 ASCIIZ string in that locale.
		///         If not found, returns the master locale.
		///         If not found even there, returns nullptr
		const char* get_sentence(sentence_id sid, locale_id lid) const;

		/// @brief Get a sentence text
		///
		/// @return Returns the UNICODE-32 string in that locale.
		///         If not found, returns the master locale.
		///         If not found even there, returns empty string.
		char32string get_sentence_char32(sentence_id sid, locale_id lid) const {
			const char* text = get_sentence(sid, lid);
			if (text == nullptr) return char32string();
			return char32string(text);
		}

		/// @brief Get a sentence_id short descripion
		///
		/// @return Returns an ASCIIZ short string with its name for debugging purposes. Returns nullptr if not available.
		const char* get_sentence_id_name(sentence_id sid) const;

	private:
		/// @brief Sync the localeCodes calculating the intersection of all
		/// string sets.
		void calc_locale_codes_intersection();

		/// @brief Internal list of string sets
		std::vector<const sentence_multilang_set*> sentence_multilang_sets;

		/// @brief Array of available languages
		std::vector<locale_id> locale_ids;
};

/*---------------------------------------
		 _       _ _
		(_)_ __ | (_)_ __   ___  ___
		| | '_ \| | | '_ \ / _ \/ __|
		| | | | | | | | | |  __/\__ \
		|_|_| |_|_|_|_| |_|\___||___/

---------------------------------------*/

// Get a sentence text
inline const char* sentence_multilang_set::get_sentence(sentence_id sid, locale_id lid) const
{
	const char* utf8text;

	// First try with the sentence set for the requested language
	const sentence_set* sentset = find_sentence_set(lid);
	if (sentset) {
		utf8text=sentset->find_sentence(sid);
		if (utf8text) return utf8text;
	}

	if (master_language==nullptr) return nullptr;
	return master_language->find_sentence(sid);
}

// Get a sentence text
inline const char* multilang_manager::get_sentence(sentence_id sid, locale_id lid) const
{
	for (auto sm: sentence_multilang_sets) {
		const char* text = sm->get_sentence(sid, lid);
		if (text != nullptr) return text;
	}
	return nullptr;
}

// Sync the localeCodes calculating the intersection of all
// string sets.
inline void multilang_manager::calc_locale_codes_intersection()
{
	uint64_t flag = 1;
	uint64_t all_flags = 0;

	// This map is used to mark with a flag 0..63 the `sentence_multilang_set`
	// that support the given `locale_id`. Only those with all the flags set
	// will be added to the intersection.
	std::map<locale_id, uint64_t> locale_usages;

	// Start by clearing the vector
	locale_ids.clear();

	// Loop on every `sentence_multilang_set`
	for (auto sm_set: sentence_multilang_sets) {
		all_flags |= flag;

		// Loop on every language
		for (size_t i=0; i<sm_set->languages_count; i++) {
			auto ret = locale_usages.insert({sm_set->languages[i]->lid, 0});
			ret.first->second |= flag;
		}

		flag <<= 1;
	}

	// Scan the map and add those with all flags set
	for (auto usages: locale_usages) {
		if (usages.second == all_flags) locale_ids.push_back(usages.first);
	}
}

// Get a sentence_id short descripion
inline const char* multilang_manager::get_sentence_id_name(sentence_id sid) const
{
	for (auto sm: sentence_multilang_sets) {
		const char* text = sm->get_sentence_id_name(sid);
		if (text != nullptr) return text;
	}
	return nullptr;
}

} // namespace dastd
