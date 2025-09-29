/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include "utf8.hpp"
#include "flooder_ch32.hpp"
#include "sink_ch32.hpp"
#include <cstring>
#ifdef DASTD_JNI
#include <jni.h>
#endif

namespace dastd {

/// @brief 32-bit unicode string
class char32string: public std::u32string, public flooder_ch32, public sink_ch32 {

	public:
		/// @brief Empty constructor
		char32string() {}

		/// @brief Copy constructor
		char32string(const std::u32string_view& o): std::u32string(o) {}

		/// @brief Copy constructor
		char32string(const std::u32string& o): std::u32string(o) {}

		/// @brief Copy constructor
		//char32string(const char32string& o): std::u32string(o) {}

		/// @brief Constructor from a const char32_t* array
		char32string(const char32_t* str): std::basic_string<char32_t>(str) {}

		/// @brief Constructor from a const char32_t* array
		char32string(const char32_t* str, size_t len): std::basic_string<char32_t>(str, len) {}

		/// @brief Constructor from a const char* UTF-8 array
		char32string(const char* utf8) {append_from_utf8(utf8);}

		/// @brief Constructor from a string UTF-8
		char32string(const std::string& utf8) {append_from_utf8(utf8);}

		/// @brief Constructor from a string UTF-8
		char32string(const std::string_view& utf8) {append_from_utf8(utf8);}

		/// @brief Set from another flooder_ch32
		char32string(const flooder_ch32& c32src) {c32src.write_to_sink(*this);}

		/// @brief Set from another flooder_ch32
		char32string(const flooder_ch32* c32src) {c32src->write_to_sink(*this);}

		/// @brief Assignment operator
		/// @param o Source string
		/// @return Returns itself
		char32string& operator=(const std::u32string_view& o) {std::u32string::operator=(o); return *this;}

		/// @brief Assignment operator
		/// @param o Source string
		/// @return Returns itself
		char32string& operator=(const char32_t* o) {std::u32string::operator=(o); return *this;}

		/// @brief Assignment operator
		/// @param o Source string
		/// @return Returns itself
		char32string& operator=(const char32string& o) {std::u32string::operator=(o); return *this;}

		///  @brief Append from UTF-8.
		///
		///  @param utf8 An ASCII string containing UTF-8 encoded characters.
		///  @param len  Length in bytes of the string
		///
		///  @return Returns the number of characters used from "utf8".
		///  If this number does not include the entire string, it means that an
		///  error occurred at that position.
		size_t append_from_utf8(const char* utf8, size_t len);

		///  @brief Append from UTF-8.
		///
		///  @param utf8 An ASCII string containing UTF-8 encoded characters.
		///
		///  @return Returns the number of characters used from "utf8".
		///  If this number does not include the entire string, it means that an
		///  error occurred at that position.
		size_t append_from_utf8(const std::string_view& utf8) {return append_from_utf8(utf8.data(), utf8.length());}

		///  @brief Append from UTF-8.
		///
		///  @param utf8 A zero-terminated ASCII string containing UTF-8 encoded characters.
		///
		///  @return Returns the number of characters used from "utf8".
		///  If this number does not include the entire string, it means that an
		///  error occurred at that position.
		size_t append_from_utf8(const char* utf8) {return append_from_utf8(utf8, strlen(utf8));}

		///  @brief Set the string from UTF-8.
		///
		///  @param utf8 A zero-terminated ASCII string containing UTF-8 encoded characters.
		/// @param len  Maximum allowed length in bytes of the string
		///
		///  @return Returns the number of characters used from "utf8".
		///  If this number does not include the entire string, it means that an
		///  error occurred at that position.
		size_t set_from_utf8(const char* utf8, size_t len=SIZE_MAX) {clear(); return append_from_utf8(utf8, len);}

		///  @brief Set the string from UTF-8.
		///
		///  @param utf8 A ASCII string containing UTF-8 encoded characters.
		///
		///  @return Returns the number of characters used from "utf8".
		///  If this number does not include the entire string, it means that an
		///  error occurred at that position.
		size_t set_from_utf8(const std::string& utf8) {clear(); return append_from_utf8(utf8);}

		/// @brief Return this string encoded in UTF-8
		/// @return Returns the UTF-8 string
		std::string get_utf8() const;

		/// @brief Append this string encoded in UTF-8 to `utf8`
		/// @param utf8 Gets this string encoded in UTF-8 appended
		void get_utf8(std::string& utf8) const;

		/// @brief Return the length of this string in UTF-8 characters
		/// @return Returns the length
		size_t get_utf8_length() const;

		/// @brief Writes the string into "output" using the UTF-8 encoding.
		///
		/// Writes the string into "output" using the UTF-8 encoding. It will write
		/// a terminating zero.
		///
		/// @param output          Target buffer
		/// @param max_output_length Maximum number of bytes that can be written to `output`
		///
		/// @return Returns the number of characters it should have
		/// printed. This number includes the byte required for the zero terminator.
		size_t write_utf8_with_ending_zero(char* output, size_t max_output_length) const;

		/// @brief Compare this string with an UTF-8 string
		/// @param utf8 The UTF-8 encoded string this string is to be compared to
		/// @return Return <0 if this<utf8;
		int compare_utf8(const std::string_view& utf8) const {return compare(char32string(utf8));}

		///  @brief Write on the indicate sink_ch32
		virtual void write_to_sink(sink_ch32& sink) const override;

	protected:
		/// @brief Put one char32_t character in the sink
		///
		/// Implementation of sink_ch32 method.
		///
		/// @param code_point The character to be written
		virtual void internal_sink_write_char32(char32_t code_point) override {push_back(code_point);}

		/// @brief Write an array of char32_t characters in the sink
		///
		/// This method is provided for optimizations. Default implementation
		/// is to repeatedly call `put`.
		/// Implementation of sink_ch32 method.
		///
		/// @param c32data The characters to be written
		/// @param c32len The number of characters to be written
		virtual void internal_sink_write_char32_array(const char32_t* c32data, size_t c32len) override {reserve(c32len); sink_ch32::internal_sink_write_char32_array(c32data, c32len);}

	public:
		#ifdef DASTD_JNI
		/*--------------------------------------------------------------------------
			CONSTRUCTOR FROM A JNI "jstring"
		--------------------------------------------------------------------------*/
		char32string(JNIEnv* env, jstring source) {set_fromJNI(env, source);}

		/*--------------------------------------------------------------------------
			Append the given JNI "jstring" to this string.
		--------------------------------------------------------------------------*/
		void append_fromJNI(JNIEnv* env, jstring source);

		/*--------------------------------------------------------------------------
			Append the given JNI "jstring" to this string.
		--------------------------------------------------------------------------*/
		void set_fromJNI(JNIEnv* env, jstring source) {clear(); append_fromJNI(env, source);}

		/*--------------------------------------------------------------------------
			Returns a new "jstring"
		--------------------------------------------------------------------------*/
		jstring getJNI(JNIEnv* env) const;
		#endif
};

inline sink_ch32& operator<<(sink_ch32& sink, const char32string& d) {sink.sink_write(d); return sink;}


} // namespace dastd

#ifdef DASTD_WASM
extern "C" {
	/// @brief Function used by WASM to write to a string.
	///
	/// Prepares a preallocated buffer of len characters in the given string.
	/// Since the `stringToUTF32` function writes the terminating zero, that is not
	/// used in char32string, we must allocate a longer string and then truncate it
	/// with `_char32string_truncate_zero`.
	///
	/// Usage in javascript:
	///
	///   function writeStringToCpp(jsString, cppStringPtr)
	///   {
	///     let lenUTF32 = lengthBytesUTF32(jsString);
	///     stringToUTF32(jsString, _char32string_prepare_buffer(cppStringPtr, lenUTF32/4), lenUTF32+4);
	///     _char32string_truncate_zero(cppStringPtr);
	///   }
	///
	/// @param str The string, allocated with `char32string_new`
	/// @param len The number of expected characters
	/// @return Returns the raw pointer where the string can be written
	inline void* EMSCRIPTEN_KEEPALIVE char32string_prepare_buffer(dastd::char32string* str, size_t len) {str->resize(len+1); return str->data();}

	/// @brief Removes the final character from the string
	inline void EMSCRIPTEN_KEEPALIVE char32string_truncate_zero(dastd::char32string* str) {if (str->size() > 0) str->resize(str->size()-1);}

	/// @brief Function used by WASM to get the number of characters in the string.
	/// @param str The target string
	/// @return Returns the number of characters
	inline size_t EMSCRIPTEN_KEEPALIVE char32string_size(const dastd::char32string* str) {return str->size();}

	/// @brief Function used by WASM to get the pointer to internal buffer in order to read the string
	/// @param str The target string
	/// @return Returns the raw pointer
	inline const void* EMSCRIPTEN_KEEPALIVE char32string_get_internal_buffer(const dastd::char32string* str) {return str->data();}

	/// @brief Function used by WASM to create a new instance of a char32string
	/// @return Returns a new instance of a char32string
	inline dastd::char32string* EMSCRIPTEN_KEEPALIVE char32string_new() {return new dastd::char32string;}

	/// @brief Function used by WASM to delete and instance of a char32string that has created with char32string_new
	inline void EMSCRIPTEN_KEEPALIVE char32string_delete(dastd::char32string* str) {delete str;}

	/// @brief Function used by WASM to write to a std::string.
	///
	/// Prepares a preallocated buffer of len characters in the given string.
	/// Since the `stringToUTF8` function writes the terminating zero, that is not
	/// used in string, we must allocate a longer string and then truncate it
	/// with `_string_truncate_zero`.
	///
	/// Usage in javascript:
	///
	///   function writeStringToCpp(jsString, cppStringPtr)
	///   {
	///     let lenUTF8 = lengthBytesUTF8(jsString);
	///     stringToUTF8(jsString, _string_prepare_buffer(cppStringPtr, lenUTF8), lenUTF8+1);
	///     _string_truncate_zero(cppStringPtr);
	///   }
	///
	/// @param str The string, allocated with `string_new`
	/// @param len The number of expected characters
	/// @return Returns the raw pointer where the string can be written
	inline void* EMSCRIPTEN_KEEPALIVE string_prepare_buffer(std::string* str, size_t len) {str->resize(len+1); return str->data();}

	/// @brief Removes the final character from the string
	inline void EMSCRIPTEN_KEEPALIVE string_truncate_zero(std::string* str) {if (str->size() > 0) str->resize(str->size()-1);}

	/// @brief Function used by WASM to get the number of characters in the string.
	/// @param str The target string
	/// @return Returns the number of characters
	inline size_t EMSCRIPTEN_KEEPALIVE string_size(const std::string* str) {return str->size();}

	/// @brief Function used by WASM to get the pointer to internal buffer in order to read the string
	/// @param str The target string
	/// @return Returns the raw pointer
	inline const void* EMSCRIPTEN_KEEPALIVE string_get_internal_buffer(const std::string* str) {return str->data();}

	/// @brief Function used by WASM to create a new instance of a string
	/// @return Returns a new instance of a string
	inline std::string* EMSCRIPTEN_KEEPALIVE string_new() {return new std::string;}

	/// @brief Function used by WASM to delete and instance of a string that has created with string_new
	inline void EMSCRIPTEN_KEEPALIVE string_delete(std::string* str) {delete str;}

}
#endif


//    ___ _   _ _     ___ _   _ _____
//   |_ _| \ | | |   |_ _| \ | | ____|
//    | ||  \| | |    | ||  \| |  _|
//    | || |\  | |___ | || |\  | |___
//   |___|_| \_|_____|___|_| \_|_____|
//
#include "char32string.hpp"
#include "sink_ch32.hpp"
#include "utf8.hpp"
#include "fmt.hpp"
#include "ostream_string.hpp"

namespace dastd {

// Append from UTF-8.
inline size_t char32string::append_from_utf8 (const char* utf8, size_t len)
{
	char32_t code_point;

	size_t i;
	size_t ret=0;
	for (i = 0; i<len; i+=ret) {
		ret = read_utf8_asciiz(utf8+i, code_point);
		if (ret == 0 || code_point == CHAR32_INVALID) break;
		push_back(code_point);
	}
	return i;
}

// Write to a sink_ch32
inline void char32string::write_to_sink(sink_ch32& sink) const
{
	sink.sink_write(c_str(), length());
}

// Return this string encoded in UTF-8
inline std::string char32string::get_utf8() const
{
	#if 1
	std::string str;
	get_utf8(str);
	return str;
	#else
	size_t len = calc_utf8_length(c_str(), length());
	ostream_string out;
	out.str().reserve(len+2);
	dastd::write_utf8(out, c_str(), length());
	return out.str();
	#endif
}

// Append this string encoded in UTF-8 to `utf8`
inline void char32string::get_utf8(std::string& utf8) const
{
	size_t len = calc_utf8_length(c_str(), length());
	size_t offset = utf8.size();
	utf8.resize(utf8.size()+len);
	size_t bytes_written;
	size_t chars_written;
	write_utf8_to_char_buffer(utf8.data()+offset, len, bytes_written, chars_written, data(), length());
	assert(bytes_written == len);
}

// Return the length of this string in UTF-8 characters
inline size_t char32string::get_utf8_length() const
{
	return calc_utf8_length(c_str(), length());
}



//------------------------------------------------------------------------------
// (brief) Writes the string into "output" using the UTF-8 encoding.
//
// Writes the string into "output" using the UTF-8 encoding. It will write
// a terminating zero.
//
// (param) output          Target buffer
// (param) max_output_length Maximum number of bytes that can be written to `output`
// (param) withEndingZero  If true, it terminates the string with '\0'
//
// (return) Returns the number of characters it should have
// printed. This number includes the byte required for the zero terminator.
//------------------------------------------------------------------------------
inline size_t char32string::write_utf8_with_ending_zero(char* output, size_t max_output_length) const
{
		if (max_output_length == 0) {
				return get_utf8_length() + 1;
		}
	assert(max_output_length>0);
		max_output_length--;
	size_t bytes_written;
	size_t chars_written;
		size_t required_bytes = write_utf8_to_char_buffer(output, max_output_length, bytes_written, chars_written, c_str(), length());
	assert(bytes_written <= max_output_length);
	output[bytes_written] = '\0';
	return required_bytes+1;
}


#ifdef DASTD_JNI
#include "flooder_ch32_conststr.hpp"

/// Returns a new "jstring"
inline jstring char32string::getJNI(JNIEnv* env) const
{
	#define SHORTBUF 48
	char short_buffer[SHORTBUF];
	char* buffer;
	size_t buffer_len = get_utf8_length()+1;

	buffer = (buffer_len <= SHORTBUF ? short_buffer : new char[buffer_len]);

	/* Out of memory situation */
	if (buffer == NULL) {
		strcpy (short_buffer, "OUT OF MEMORY");
		buffer_len = strlen(short_buffer);
		buffer = short_buffer;
	}
	else {
		write_utf8_with_ending_zero(buffer, buffer_len);
	}
	jstring ret = env->NewStringUTF(buffer);

	if (buffer != short_buffer) {
		delete[](buffer);
	}
	return ret;
}

/*--------------------------------------------------------------------------
	Append the given JNI "jstring" to this string.
--------------------------------------------------------------------------*/
inline void char32string::append_fromJNI(JNIEnv* env, jstring source)
{
	uint32_t len = (uint32_t)env->GetStringLength(source);
	const jchar* java_string_chars = env->GetStringChars(source, NULL);

	dastd::flooder_ch32_conststr<char16_t> unicode_source((const char16_t*) java_string_chars, len);
	unicode_source.write_to_sink(*this);

	env->ReleaseStringChars(source, java_string_chars);
}

#endif

}
