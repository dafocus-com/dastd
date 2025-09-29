/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-JUN-2025
**/
#pragma once
#include "ostream_basic.hpp"
#include <vector>

namespace dastd {

/// @brief Implementation of a `ostream` that appends to a std::vector<TYPE>
///
/// This object carries a std::string and writes on it.
template <class CHARTYPE>
class ostream_vector: public ostream_basic {
	private:
		std::vector<CHARTYPE>& m_vector;

	public:
		ostream_vector(std::vector<CHARTYPE>& vector): m_vector(vector) {}

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override {m_vector.push_back((CHARTYPE)c);}
};

} // namespace dastd
