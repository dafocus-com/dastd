/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 11-AUG-2023
**/
#if defined INCLUDE_dastd_sink_ch32_class && !defined dastd_sink_ch32_class
#define dastd_sink_ch32_class

namespace dastd {
	/// @brief Base class for objects that are able to consume UNICODE char32_t characters
	///
	/// Base class for objects that are able to consume UNICODE char32_t characters for example
	/// for printing, conversions and so on.
	class sink_ch32 {
		public:
			/// @brief Destructor
			virtual ~sink_ch32() {}

			/// @brief Put one char32_t character in the sink
			///
			/// @param code_point The character to be written
			void sink_write(char32_t code_point) {internal_sink_write_char32(code_point);}

			/// @brief Write an array of char32_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char32_t)`.
			///
			/// @param c32data The characters to be written
			/// @param c32len The number of characters to be written
			void sink_write(const char32_t* c32data, size_t c32len) {internal_sink_write_char32_array(c32data, c32len);}

			/// @brief Write a zero terminated array of char32_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char32_t)`.
			///
			/// @param c32data The characters to be written, zero terminated
			void sink_write(const char32_t* c32data) {internal_sink_write_char32_arrayz(c32data);}

			/// @brief Write an array of char16_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char16_t)`.
			///
			/// @param c16data The characters to be written
			/// @param c16len The number of characters to be written
			void sink_write(const char16_t* c16data, size_t c16len) {size_t i; for(i=0; i<c16len; i++) sink_write((char32_t)c16data[i]);}

			/// @brief Write a zero terminated array of char16_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char16_t)`.
			///
			/// @param c16data The characters to be written, zero terminated
			void sink_write(const char16_t* c16data) {size_t i; for(i=0; c16data[i] != 0; i++) sink_write((char32_t)c16data[i]);}

			/// @brief Write a zero terminated array of UTF-8 characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write`.
			/// In case of error (invalid UTF-8 character) the string is truncated.
			///
			/// @param utf8 Zero terminated UTF-8 string.
			void sink_write(const char* utf8);

			/// @brief Write an array of UTF-8 characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write`.
			/// In case of error (invalid UTF-8 character) the string is truncated.
			///
			/// @param utf8     UTF-8 string.
			/// @param utf8len  Length of the `utf8` string.
			void sink_write(const char* utf8, size_t utf8len);

			/// @brief Write a UTF-8 encoded std::string in the sink
			///
			/// @param str Zero terminated UTF-8 string.
			void sink_write(std::string_view str) {sink_write(str.data(), str.size());}

			/// @brief Write a UTF-32 encoded std::string in the sink
			///
			/// @param u32str UTF-32 string.
			void sink_write(std::u32string_view u32str) {sink_write(u32str.data(), u32str.length());}

			/// @brief Write a UTF-8 encoded std::string in the sink
			///
			/// @param str Zero terminated UTF-8 string.
			void sink_write(const std::string& str) {sink_write(str.data(), str.size());}

			/// @brief Write a UTF-32 encoded std::string in the sink
			///
			/// @param u32str UTF-32 string.
			void sink_write(const std::u32string& u32str) {sink_write(u32str.data(), u32str.length());}

		protected:
			/// @brief Put one char32_t character in the sink
			///
			/// @param code_point The character to be written
			virtual void internal_sink_write_char32(char32_t code_point) = 0;

			/// @brief Write an array of char32_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char32_t)`.
			///
			/// @param c32data The characters to be written
			/// @param c32len The number of characters to be written
			virtual void internal_sink_write_char32_array(const char32_t* c32data, size_t c32len) {size_t i; for(i=0; i<c32len; i++) sink_write(c32data[i]);}

			/// @brief Write a zero terminated array of char32_t characters in the sink
			///
			/// This method is provided for optimizations. Default implementation
			/// is to repeatedly call `sink_write(char32_t)`.
			///
			/// @param c32data The characters to be written, zero terminated
			virtual void internal_sink_write_char32_arrayz(const char32_t* c32data) {size_t i; for(i=0; c32data[i] != 0; i++) sink_write(c32data[i]);}
	};
} // namespace dastd
#endif
