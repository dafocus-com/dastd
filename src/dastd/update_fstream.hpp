/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "defs.hpp"
// Requires at least C++ 17
static_assert(DASTD_CPP_VER >= 17);

#include <sstream>
#include <filesystem>
#include <fstream>
#include "exception.hpp"

namespace dastd {

/// @brief Manages an output file that is to be written only if the new contents are different
///
/// It saves all the text on a string. It writes it to the actual file only if the
/// target file does not exist or if the existing file is different.
class update_fstream: public std::ostringstream {
	public:
		// @brief Return codes
		enum write_ret {
			FILE_CREATED,   //< The file has been created
			FILE_UPDATED,   //< The file has been updated
			FILE_UNCHANGED, //< The file exists and it has not been updated since it is identical
			FILE_ERROR,     //< Some error has occured; check `errno` for details
		};

		/// @brief Writes the internal string if different from the file
		///
		/// @param file_name Name of the file to be created or updated.
		/// @return Return a `write_ret` entry indicating what happened
		write_ret write_if_changed(const std::filesystem::path& file_name);

		/// @brief Set to true to have this throw an exception in case of error
		bool m_throw_on_error = false;
};

/// Writes the internal string if different from the file
inline update_fstream::write_ret update_fstream::write_if_changed(const std::filesystem::path& file_name)
{
	update_fstream::write_ret ret;

	// Get the size of the file
	std::error_code ec;
	std::uintmax_t size = std::filesystem::file_size(file_name, ec);
	// If it returns some kind of error code, the file might not exist
	// We write as new.
	if (ec) {
		ret = FILE_CREATED;
	}
	// If the size of the file does not match the size of the internal string,
	// it is to be updated anyway.
	else if(size != str().size()) {
		ret = FILE_UPDATED;
	}
	// Otherwise, we must compare the file with our string
	else {
		// Open the file for reading
		std::ifstream file_on_disk(file_name, std::ios_base::in|std::ios_base::binary);
		if (file_on_disk.fail()) {
			if (m_throw_on_error) DASTD_THROW(dastd::exception, "Error opening file '" << file_name.generic_string() << "' for reading: " << dastrerror());
			return FILE_ERROR;
		}

		ret = FILE_UNCHANGED;

		// Compare the file with the string
		size_t i;
		for (i=0; i<str().size(); i++) {
			int ch = file_on_disk.get();
			if (ch == EOF) {
				ret = FILE_UPDATED;
				break;
			}
			if (((char)ch) != str()[i]) {
				ret = FILE_UPDATED;
				break;
			}
		}
		file_on_disk.close();
		if (ret == FILE_UNCHANGED) return ret;
	}

	// If reaches here is because the file is to be written
	assert((ret==FILE_UPDATED) || (ret==FILE_CREATED));

	// Open the file for writing
	std::ofstream file_on_disk(file_name, ios_base::out|ios_base::binary);
	if (file_on_disk.fail()) {
		if (m_throw_on_error) DASTD_THROW(dastd::exception, "Error opening file '" << file_name.generic_string() << "' for writing: " << dastrerror());
		return FILE_ERROR;
	}
	file_on_disk.write(str().c_str(), str().size());
	if (file_on_disk.fail()) {
		if (m_throw_on_error) DASTD_THROW(dastd::exception, "Error writing on file '" << file_name.generic_string() << "': " << dastrerror());
		return FILE_ERROR;
	}
	file_on_disk.close();
	return ret;
}

inline std::ostream& operator<<(std::ostream& o, update_fstream::write_ret r)
{
	switch(r) {
		case update_fstream::FILE_CREATED  : o << "FILE_CREATED"; break;
		case update_fstream::FILE_UPDATED  : o << "FILE_UPDATED"; break;
		case update_fstream::FILE_UNCHANGED: o << "FILE_UNCHANGED"; break;
		case update_fstream::FILE_ERROR    : o << "FILE_ERROR"; break;
		default: o << "FILE_xxx(" << (unsigned)r << ")";
	}
	return o;
}

} // namespace dastd
