/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 03-JUN-2026
**/
#pragma once
#include "defs.hpp"
#include "sink_ch32.hpp"
#include "sink_ch32_u32string.hpp"
#include "source.hpp"
#include "source_istream.hpp"
#include "ostream_utf8.hpp"
#include <filesystem>
#include <fstream>

namespace dastd {


/// @brief Load data from an UTF-8 source into a UTF-32 destination
/// @param src The 8-bit source
/// @param snk The 32-bit destination
void load_utf8_source_into_utf32_sink(source<char>& src, sink_ch32& snk);

/// @brief Load data from an UTF-8 source into a UTF-32 destination
/// @param file_path The file to be read
/// @param str The target string
/// @throws Throws std::exception in case of errors opening the file etc.
void load_utf8_file_into_utf32_string(const std::filesystem::path& file_path, std::u32string& str);





/*
 ___ _   _ _     ___ _   _ _____
|_ _| \ | | |   |_ _| \ | | ____|
 | ||  \| | |    | ||  \| |  _|
 | || |\  | |___ | || |\  | |___
|___|_| \_|_____|___|_| \_|_____|

*/

// Load data from an UTF-8 source into a UTF-32 destination
inline void load_utf8_source_into_utf32_sink(source<char>& src, sink_ch32& snk)
{
	static constexpr size_t	BUFSIZE = 128;
	char buffer[BUFSIZE];
	ostream_utf8 out(snk);
	for(;;) {
		size_t chars_read = src.tentative_read(buffer, BUFSIZE);
		out.write(buffer, chars_read);
		if (chars_read < BUFSIZE) break;
	}
}

// Load data from an UTF-8 file into a UTF-32 string
inline void load_utf8_file_into_utf32_string(const std::filesystem::path& file_path, std::u32string& str)
{
	std::ifstream in;
	in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	in.open(file_path, std::ios::binary);

	// Remove failbit, otherwise it will throw the exception while attemping to read
	// past the end of the file.
	in.exceptions(std::ifstream::badbit);
	
	source_istream<char> src(in);
	sink_ch32_u32string snk(str);
	load_utf8_source_into_utf32_sink(src, snk);
	in.close();
}

}
