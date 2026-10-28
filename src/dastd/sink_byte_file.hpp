/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "sink_byte_ostream.hpp"
#include <fstream>

namespace dastd {

/// @brief Binary sink object that writes to a std::ostream
class sink_byte_file: public sink_byte_ostream {
	private:
		/// @brief Input file stream
		std::ofstream m_file;

		/// @brief Configure the file object
		void configure();

	public:
		/// @brief Constrtuctor
		sink_byte_file(): sink_byte_ostream(m_file) {configure();}

		/// @brief Constrtuctor
		/// @param file_name The path of the file to be read
		/// @param append If `true` if appends if the file already exists; otherwise, the file is rewritten.
		/// @param binary Set to `true` to read binary files
		/// @throws In case of error, throws `std::ios_base::failure`, derived from `std::exception`
		sink_byte_file(const std::filesystem::path& file_name, bool append = false, bool binary = true): sink_byte_ostream(m_file) 
		{
			configure();
			open(file_name, append, binary);
		}

		/// @brief Open the file for writing
		/// @param file_name The path of the file to be read
		/// @param append If `true` if appends if the file already exists; otherwise, the file is rewritten.
		/// @param binary Set to `true` to read binary files
		/// @throws In case of error, throws `std::ios_base::failure`, derived from `std::exception`
		void open(const std::filesystem::path& file_name, bool append = false, bool binary = true);
};

//------------------------------------------------------------------------------
// (brief) Configure the file object
//------------------------------------------------------------------------------
inline void sink_byte_file::configure()
{
	m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
}


//------------------------------------------------------------------------------
// (brief) Open the file for writing
// (param) file_name The path of the file to be read
// (param) append If `true` if appends if the file already exists; otherwise, the file is rewritten.
// (param) binary Set to `true` to read binary files
// (throws) In case of error, throws `std::ios_base::failure`, derived from `std::exception`
//------------------------------------------------------------------------------
inline void sink_byte_file::open(const std::filesystem::path& file_name, bool append, bool binary)
{
	std::ios_base::openmode open_mode = std::ios_base::out;
	if (append) open_mode |= std::ios_base::app;
	if (binary) open_mode |= std::ios_base::binary;
	m_file.open(file_name, open_mode);
}

} // namespace dastd
