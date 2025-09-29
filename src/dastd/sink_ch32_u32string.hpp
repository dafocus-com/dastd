/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 29-SEP-2023
**/
#pragma once
#include "sink_ch32.hpp"

namespace dastd {
	/// @brief sink_ch32 that write the char32_t characters into a std::u32string
	class sink_ch32_u32string: public sink_ch32 {
		public:
			/// @brief Constructor
			sink_ch32_u32string(std::u32string& str): m_str(str) {}

		protected:
			/// @brief Put one char32_t character in the sink
			///
			/// @param code_point The character to be written
			virtual void internal_sink_write_char32(char32_t code_point) override {m_str.append(1, code_point);}

			/// @brief Write an array of char32_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char32_t)`.
			///
			/// @param c32data The characters to be written
			/// @param c32len The number of characters to be written
			virtual void internal_sink_write_char32_array(const char32_t* c32data, size_t c32len) override {m_str.append(c32data, c32len);}

			/// @brief Write a zero terminated array of char32_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char32_t)`.
			///
			/// @param c32data The characters to be written, zero terminated
			virtual void internal_sink_write_char32_arrayz(const char32_t* c32data) override {m_str.append(c32data);}

		private:
			/// @brief Stream that will receive the data converted into the 8-bit `m_format`
			std::u32string& m_str;
	};

} // namespace dastd
