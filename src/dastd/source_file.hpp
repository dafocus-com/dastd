/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "source_istream.hpp"
#include <fstream>
#include <filesystem>

namespace dastd {

/// @brief Binary data source bound to a std::ifstream
///
/// It can be used to quickly read from a file
template<class CHARTYPE = char>
class source_file: public source_istream<CHARTYPE> {
	// This class currently supports only 1-byte long types
	static_assert(sizeof(CHARTYPE) == 1, "source_fike currently supports only 1-byte long characters");
	private:
		/// @brief Input file stream
		std::ifstream m_file;

		/// @brief Configure the file object
		void configure();

	public:
		/// @brief Constructor
		source_file(): source_istream<CHARTYPE>(m_file) {configure();}

		/// @brief Constructor
		source_file(const std::filesystem::path& file_name, bool binary = true): source_istream<CHARTYPE>(m_file) 
		{
			configure();
			open(file_name, binary);
		}

		/// @brief Open the file for reading
		/// @param file_name The path of the file to be read
		/// @param binary Set to `true` to read binary files
		/// @throws In case of error, throws `std::ios_base::failure`, derived from `std::exception`
		void open(const std::filesystem::path& file_name, bool binary = true);
};


//------------------------------------------------------------------------------
// (brief) Configure the file object
//------------------------------------------------------------------------------
template<class CHARTYPE>
inline void source_file<CHARTYPE>::configure()
{
	m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
}

//------------------------------------------------------------------------------
// (brief) Open the file for reading
// (param) file_name The path of the file to be read
// (param) binary Set to `true` to read binary files
// (throws) In case of error, throws `std::ios_base::failure`, derived from `std::exception`
//------------------------------------------------------------------------------
template<class CHARTYPE>
inline void source_file<CHARTYPE>::open(const std::filesystem::path& file_name, bool binary)
{
	std::ios_base::openmode open_mode = std::ios_base::in;
	if (binary) open_mode |= std::ios_base::binary;
	m_file.open(file_name, open_mode);
}

} // namespace dastd
