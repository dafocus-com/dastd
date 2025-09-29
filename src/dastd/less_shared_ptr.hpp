/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 30-JUL-2024
*
**/
#pragma once
#include "defs.hpp"
namespace dastd {

/// @brief Implementation of the `std::less` comparator but operating on the values of two shared pointers
/// @tparam T Type of the data pointed by the shared pointer
///
/// Example:
///
///        std::set<std::shared_ptr<MyClass>, dastd::less_shared_ptr<MyClass>> s;
///
template<class T>
class less_shared_ptr {
	public:
		constexpr bool operator()(const std::shared_ptr<const T>& lhs, const std::shared_ptr<const T>& rhs) const {return (*lhs) < (*rhs);}
};


} // namespace dastd
