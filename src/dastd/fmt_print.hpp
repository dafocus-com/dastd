/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 25-OCT-2025
**/
#pragma once
#include "defs.hpp"
#include "ostream_indent.hpp"
#include <string>

namespace dastd {

enum class fmt_print_f {
	PRINT_PACKED,
	PRINT_INDENTED
};

class fmt_print_t {
	private:
		/// Select the way the data is printed
		fmt_print_f m_print_mode;

		/// @brief Indent string
		std::string m_indent = "\t";

	public:
		/// @brief Constructor
		/// @param print_mode Select the way the data is printed
		fmt_print_t(fmt_print_f print_mode, std::string indent = "\t"): m_print_mode(print_mode), m_indent(indent) {}

		/// @brief Get the print mode
		/// @return Returns the print mode
		fmt_print_f get_print_mode() const {return m_print_mode;}

		/// @brief Get the indent string to be used for indenting
		/// @return 
		const std::string& get_indent() const {return m_indent;}

		/// @brief Configure an @ostream_indent object
		void configure(dastd::ostream_indent& stream) const {stream.set_indent_string(m_indent);}

		bool operator==(fmt_print_f print_mode) const {return m_print_mode == print_mode;}
		bool operator!=(fmt_print_f print_mode) const {return m_print_mode != print_mode;}
};

template<class PRINTABLE>
class fmt_print {
	private:
		const PRINTABLE& m_printable;
		fmt_print_t m_fmt;

	public:
		/// @brief Constructor
		/// @param printable Target printable
		/// @param fmt Format to be applied
		fmt_print(const PRINTABLE& printable, const fmt_print_t& fmt): m_printable(printable), m_fmt(fmt) {}

        /// @brief Print the data
        /// @param o Output stream
        void print(std::ostream& o) const {m_printable.print(o, m_fmt);}
};

template<class PRINTABLE>
inline std::ostream& operator<<(std::ostream& o, const fmt_print<PRINTABLE>& fp) {fp.print(o); return o;}

} // namespace dastd