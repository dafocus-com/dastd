/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 24-JUL-2024
**/
#pragma once
#include "defs.hpp"
#include "hash_crc32.hpp"
#include <filesystem>
#include <chrono>
namespace dastd {

/// @brief Value returned by `file_fast_signature` in case the file is not valid
constexpr uint32_t file_fast_signature_bad = 0;

/// @brief Calculates a uint32_t value based on the file size and last write time, likely to change if the file is modified
/// @param filePath Path of the file to be analyzed
/// @return Returns a uint32_t or `file_fast_signature_bad` if the file is not found or anything else goes wrong.
inline uint32_t file_fast_signature(const std::filesystem::path& filePath)
{
	try {
		std::ostringstream text;
		std::filesystem::file_time_type last_write = std::filesystem::last_write_time(filePath);
		size_t size=0;
		if (std::filesystem::is_directory(filePath)) {
			text << "D ";
		}
		else {
			size = std::filesystem::file_size(filePath);
		}

		auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(last_write);
		text << ns.time_since_epoch().count() << " " << size;
		uint32_t signature = crc32(text.str());
		if (signature == file_fast_signature_bad) signature++;
		return signature;
	}
	catch(...) {
		return file_fast_signature_bad;
	}
}

} // namespace dastd
