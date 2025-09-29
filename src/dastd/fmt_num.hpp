/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 16-SEP-2024
*
**/
#pragma once
#include "defs.hpp"
#include "fmt32.hpp"
#include <ostream>

namespace dastd {

/// @brief Formatting class that writes the data type as a number even if char or uint8_t
/// @tparam NUMTYPE
///
/// This class prints the number as a decimal number even in case it is a char or uint8_t.
/// The regular operator<< prints them as ASCII characters.
/// It works with floating point values as well.
template<class NUMTYPE>
class fmt_num {
	private:
		NUMTYPE m_data;
	public:
		fmt_num(NUMTYPE data): m_data(data) {}
		void print(std::ostream& o) const {
			if constexpr (std::is_integral_v<NUMTYPE> && std::is_unsigned_v<NUMTYPE> && sizeof(NUMTYPE)<=sizeof(unsigned)) {
				o << (unsigned)m_data;
			}
			else if constexpr (std::is_integral_v<NUMTYPE> && std::is_signed_v<NUMTYPE> && sizeof(NUMTYPE)<=sizeof(int)) {
				o << (int)m_data;
			}
			else {
				o << m_data;
			}
		}
};

// Stream on a std::ostream
template<class NUMTYPE>
std::ostream& operator<< (std::ostream& o, const fmt_num<NUMTYPE>& n) {n.print(o); return o;}

// Stream on a sink_ch32
template<class NUMTYPE>
sink_ch32& operator<<(sink_ch32& sink, const fmt_num<NUMTYPE>& s) {fmt32utf8<const fmt_num<NUMTYPE>&> fm(s); fm.write_to_sink(sink); return sink;}


} // namespace dastd
