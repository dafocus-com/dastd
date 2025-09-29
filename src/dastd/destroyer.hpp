/*
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-DEC-2024
* @brief Simple template class to implement object destruction in template containers
**/
#pragma once
#include "defs.hpp"

namespace dastd {

/// @brief Destroyer template class that by default does nothing
/// @tparam TYPE Type of the data to be destroyed
template<class TYPE>
class destroyer {
	public:
		/// @brief Method invoked when the object is to be deleted
		/// @param  Object to be deleted (unused)
		static void destroy(TYPE) {}
};

/// @brief Destroyer template class that works with pointers that require "delete" to be deleted
/// @tparam TYPE Type of the data to be destroyed
template<class TYPE>
class destroyer_delete {
	static_assert(std::is_pointer_v<TYPE>, "destroyer_delete can be used only with pointer types");
	public:
		/// @brief Method invoked when the object is to be deleted
		/// @param obj Object to be deleted
		static void destroy(TYPE obj) {delete obj;}
};

} // namespace dastd
