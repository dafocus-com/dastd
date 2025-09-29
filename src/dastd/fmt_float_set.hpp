/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 20-MAR-2024
**/
#pragma once
#include "defs.hpp"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <cstdio>
namespace dastd {

/// @brief Analyzes multiple floating point 'double' values to find a common reasonable formatting
///
/// The goal of this formatter is to configure the number of intergers and decimals so a set
/// of floating point values can be printed aligned, with a meaningful number of decimals but
/// without wasting too much space.
///
/// Once created an instance of `fmt_float_set`, submit all the values calling `merge`.
/// Then the single value can be formatted by calling `format`.
class fmt_float_set {
	private:
		/// @brief Number of digits at the right of the decimal point
		unsigned m_decimals = 0;

		/// @brief Number of digits at the left of the decimal point
		unsigned m_integers = 0;

		/// @brief True if it has negative values
		bool m_negative = false;

	public:
		/// @brief Analyze a value in the set
		/// @param limit_decimals If set, it will not allow decimals to more than that
		void merge(double n, unsigned limit_decimals=((unsigned)-1));

		/// @brief Write the number on the stream according to calculated formatting
		std::string format(double n) const;

		/// @brief Width occupied by the format
		unsigned width() const {return m_integers+m_decimals+(m_decimals>0 ? 1 : 0);}
};


//------------------------------------------------------------------------------
// (brief) Analyze a value in the set
//------------------------------------------------------------------------------
inline void fmt_float_set::merge(double n, unsigned limit_decimals)
{
	unsigned req_decimals = 0;
	unsigned req_integers = 0;

	if (n == 0) return;
	if (n < 0) {m_negative = true; n = std::abs(n);}
	if (n < 0.01) req_decimals = 6;
	else if (n < 1) req_decimals = 3;
	else if (n < 10) {
		if (((uint64_t)(n * 100) % 100) != 0) req_decimals = 2;
	}
	else if (n < 100) {
		if (((uint64_t)(n * 10) % 10) != 0) req_decimals = 1;
	}

	char buf[64];
	snprintf(buf, sizeof(buf), "%.0f", n);
	buf[sizeof(buf)-1] = '\0';
	req_integers = (unsigned)strlen(buf);

	if (req_decimals > limit_decimals) req_decimals = limit_decimals;

	if (m_negative) req_integers++;
	if (req_decimals > m_decimals) m_decimals = req_decimals;
	if (req_integers > m_integers) m_integers = req_integers;
}

//------------------------------------------------------------------------------
// (brief) Write the number on the stream according to calculated formatting
//------------------------------------------------------------------------------
inline std::string fmt_float_set::format(double n) const
{
	char buf[64];
	char fmt[64];
	snprintf(fmt, sizeof(fmt), "%%%u.%uf", (unsigned)width(), m_decimals);
	//std::cout << "[DEBUG] fmt=[" << fmt << "]" << std::endl;
	snprintf(buf, sizeof(buf), fmt, n);
	buf[sizeof(buf)-1] = '\0';
	return buf;
}

} // namespace dastd
