/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 11-AUG-2023
**/
#if defined INCLUDE_dastd_sink_ch32_inline && !defined dastd_sink_ch32_inline
#define dastd_sink_ch32_inline

namespace dastd {
	#define char32sink_DECLOP(type) inline sink_ch32& operator<<(sink_ch32& sink, type d) {sink << fmt32utf8<type>(d); return sink;}

	inline sink_ch32& operator<<(sink_ch32& sink, char32_t d) {sink.sink_write(d); return sink;}
	inline sink_ch32& operator<<(sink_ch32& sink, const char32_t* d) {sink.sink_write(d, std::char_traits<char32_t>::length(d)); return sink;}
	inline sink_ch32& operator<<(sink_ch32& sink, std::u32string_view d) {sink.sink_write(d); return sink;}
	inline sink_ch32& operator<<(sink_ch32& sink, const std::u32string& d) {sink.sink_write(d); return sink;}
	inline sink_ch32& operator<<(sink_ch32& sink, char c) {sink.sink_write((char32_t)c); return sink;}
	char32sink_DECLOP(double);
	char32sink_DECLOP(uint64_t);
	char32sink_DECLOP(int64_t);
	char32sink_DECLOP(uint32_t);
	char32sink_DECLOP(int32_t);
	char32sink_DECLOP(uint16_t);
	char32sink_DECLOP(int16_t);
	char32sink_DECLOP(uint8_t);
	char32sink_DECLOP(const char*);
	char32sink_DECLOP(std::string_view);
	char32sink_DECLOP(const std::string&);

	#define DASTD_DEF_WRITE_TO_SINK(t) inline dastd::sink_ch32& operator<< (dastd::sink_ch32& o, const t& n) {n.write_to_sink(o); return o;}

	size_t read_utf8_asciiz(const void* p_chars, char32_t& code_point);

	/// Write a zero terminated array of UTF-8 characters in the sink
	inline void sink_ch32::sink_write(const char* utf8)
	{
		size_t i, len;
		char32_t code_point;
		for (i=0; utf8[i]!='\0'; i+=len) {
			len = read_utf8_asciiz(utf8+i, code_point);
			if (len < 1) break;
			sink_write(code_point);
		}
	}

	/// Write a zero terminated array of UTF-8 characters in the sink
	inline void sink_ch32::sink_write(const char* utf8, size_t utf8len)
	{
		size_t i, len;
		char32_t code_point;
		for (i=0; i<utf8len; i+=len) {
			len = read_utf8_asciiz(utf8+i, code_point);
			if (len < 1) break;
			sink_write(code_point);
		}
	}
} // namespace dastd

#endif
