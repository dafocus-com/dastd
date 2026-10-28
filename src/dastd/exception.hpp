/*
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
* @brief Exceptions
**/
#pragma once
#include "defs.hpp"
#include "ostream_string.hpp"
#include <exception>
#include <sstream>

namespace dastd {

/// @brief DASTD exception base class
class exception: public std::exception {
	public:
		/// @brief Empty constructor
		exception() {}

		/// @brief Constructor with string
		exception(std::string_view error_text): m_error_text(error_text) {}

		/// @brief Copy constructor
		exception(const exception& o): m_error_text(o.m_error_text) {}

		/// @brief Operator=
		exception& operator=(const exception& o) {std::exception::operator=(o); m_error_text=o.m_error_text; return *this;}

		/// @brief Returns the explanatory string.
		/// @return Pointer to a null-terminated string with explanatory information.
		///         The pointer is guaranteed to be valid at least until the exception
		///         object from which it is obtained is destroyed, or until a non-const
		///         member function on the exception object is called.
		virtual const char* what() const noexcept {return m_error_text.str().c_str();}

		/// @brief Error message
		ostream_string m_error_text;
};

/// @brief Throw an 'exception' allowing direct texting
///
/// Example:
///
///    DASTD_THROW(exception, "This is error " << errorCode << " etc.")
///
//#define DASTD_THROW(exc,text) {exc e; e.m_error_text << text; throw e;}
#define DASTD_THROW(exc,text) throw exc(DSTR(text))

/// @brief Define an exception type derived from dastd::exception
#define DASTD_DEF_EXCEPTION(exc) class exc: public ::dastd::exception{\
	public:\
		exc() {} \
		exc(std::string_view error_text): ::dastd::exception(error_text) {}\
}; 

/// @brief Define an exception type derived from 'base'
#define DASTD_DEF_EXCEPTION2(exc,base) class exc: public base{};


} // namespace dastd

/// @brief Implementation of the << operator on std::exceptions
/// @param o Output stream
/// @param e Excpetion to be printed
/// @return Returns the output stream for chaining
inline std::ostream& operator<<(std::ostream& o, const std::exception& e) {o << e.what(); return o;}
