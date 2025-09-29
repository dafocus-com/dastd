/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include "exception.hpp"
#include <filesystem>
#include <vector>

namespace dastd {

/// @brief Finds a file in a sequence of search paths
///
/// This class contains a set of paths. It will look for the indicated
/// file in the listed paths or in the current path if allowed.
/// It can be used, for example, to implement a C-like #include
/// inclusion system.
class filefinder {
	private:
		/// @brief Contains the list of absolute search paths in the required search order
		std::vector<std::filesystem::path> m_search_paths;

		/// @brief Return the path, no matter it is a directory or a file
		static std::filesystem::path get_path(const std::filesystem::path& file_or_dir) {return (std::filesystem::is_directory(file_or_dir) ? file_or_dir : file_or_dir.parent_path());}

	public:
		/// @brief Add a search path
		/// @param search_path Search path to be added to the search ma
		/// @param ref_path    Absolute path used as a base if `search_path` is relative; if `ref_path` is a file,
		///                    it will use the path where the file is found.
		///
		/// @throw Throws a `std::exception` in case of error, including if the search path does not exist
		void add_search_path(const std::filesystem::path& search_path, const std::filesystem::path& ref_path);

		/// @brief Return the absolute path of the searched file or empty if not found
		/// @param file_name   File name to be resolved
		/// @param ref_path    If not empty, it is used as first choice for searching for the path.
		///
		/// @return Returns the absolute path of the file or empty if not found.
		std::filesystem::path find(const std::filesystem::path& file_name, const std::filesystem::path& curr_path) const noexcept;
};


//------------------------------------------------------------------------------
// (brief) Add a search path
// (param) search_path Search path to be added to the search ma
// (param) ref_path    Absolute path used as a base if `search_path` is relative; if `ref_path` is a file,
//                    it will use the path where the file is found.
//------------------------------------------------------------------------------
inline void filefinder::add_search_path(const std::filesystem::path& search_path, const std::filesystem::path& ref_path)
{
	std::filesystem::path& new_path = m_search_paths.emplace_back();
	if (search_path.is_absolute()) {
		new_path = search_path.lexically_normal();
	}
	else {
		// Create the target path
		new_path = (get_path(ref_path) / search_path).lexically_normal();
	}

	// See if the target path exists and it is a directory
	if (!std::filesystem::is_directory(new_path)) {
		DASTD_THROW(exception, "filefinder::add_search_path, base path " << new_path << " is not a directory");
	}
}

//------------------------------------------------------------------------------
// (brief) Return the absolute path of the searched file or empty if not found
// (param) file_name   File name to be resolved
// (param) ref_path    Absolute path used as a base if `file_name` is relative; if `ref_path` is a file,
//                     it will use the path where the file is found.
//------------------------------------------------------------------------------
inline std::filesystem::path filefinder::find(const std::filesystem::path& file_name, const std::filesystem::path& ref_path) const noexcept
{
	try {
		// If the file_name is absolute, no resolution is required
		if (file_name.is_absolute()) {
			if (std::filesystem::exists(file_name)) return std::filesystem::weakly_canonical(file_name);
			return std::filesystem::path();
		}
		std::filesystem::path file_being_examined;

		// If not empty, try first with
		if (!ref_path.empty()) {
			file_being_examined = (get_path(ref_path)/file_name).lexically_normal();
			if (std::filesystem::exists(file_being_examined)) return file_being_examined;
		}

		// Try all the stored paths
		for(auto const & search_path: m_search_paths) {
			file_being_examined = (search_path/file_name).lexically_normal();
			if (std::filesystem::exists(file_being_examined)) return std::filesystem::weakly_canonical(std::filesystem::absolute(file_being_examined));
		}
	}
	catch(...) {}
	return std::filesystem::path();
}


} // namespace dastd
