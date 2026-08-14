/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-AUG-2023
**/
#pragma once
#include "ostream_basic.hpp"

namespace dastd {

/// @brief Implementation of a `ostream` that appends to a std::string
///
/// This object carries a std::string and writes on it.
class ostream_string: public ostream_basic {
	public:
		/// @brief Empty constructor
		ostream_string() {}

		/// @brief Constructor with const char*
		ostream_string(const char* str): m_str(str) {}

		/// @brief Constructor with std::string
		ostream_string(std::string_view str): m_str(str) {}

		/// @brief Copy constructor
		ostream_string(const ostream_string& o): std::basic_ios<char>(), ostream_basic(), m_str(o.m_str) {}

		/// @brief Operator=
		const ostream_string& operator=(const ostream_string& o) {m_str=o.m_str; return *this;}

		/// @brief Read the string (read-only)
		const std::string& str() const {return m_str;}

		/// @brief Read the string (read-write)
		std::string& str() {return m_str;}

	protected:
		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override {m_str.push_back(c);}

		/// @brief Write multiple characters to the target stream
		///
		/// The default is to loop on "write_char". It can be redefined for better
		/// optimization.
		virtual void write_chars(const char_type* s, std::streamsize n) override {m_str.append(s, (size_t)n);}

		/// @brief Internal string
		std::string m_str;
};

/// @brief Implementation of a `ostream` that appends to an existing std::string
///
/// This object carries a std::string and writes on it.
class ostream_string_ref: public ostream_basic {
	public:
		/// @brief Constructor with std::string
		ostream_string_ref(std::string& str): m_str(str) {}

		/// @brief Read the string (read-only)
		const std::string& str() const {return m_str;}

		/// @brief Read the string (read-write)
		std::string& str() {return m_str;}

	protected:
		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override {m_str.push_back(c);}

		/// @brief Write multiple characters to the target stream
		///
		/// The default is to loop on "write_char". It can be redefined for better
		/// optimization.
		virtual void write_chars(const char_type* s, std::streamsize n) override {m_str.append(s, (size_t)n);}

		/// @brief Internal string
		std::string& m_str;
};


} // namespace dastd
