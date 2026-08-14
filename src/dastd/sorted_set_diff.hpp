/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 03-AUG-2024
**/
#pragma once
#include <iterator>
#include <functional>
#include <algorithm>

namespace dastd {

/// @brief Operates on two sorted iterables; it calls some user defined callbacks when an item
///        is present only in the first set, only in the second set or both.
/// 
/// @tparam ITER1 Type of the iterator for the first set
/// @tparam ITER2 Type of the iterator for the second set
/// @tparam LESS_COMPARATOR Less-than comparator used to compare the iterator values; defaults to `std::less<>`
/// @param beg1 Iterator placed on the start position of set 1 (usually `set1.begin()`)
/// @param end1 Iterator placed on the final position of set 1 (usually `set1.end()`) 
/// @param beg2 Iterator placed on the start position of set 2 (usually `set2.begin()`)
/// @param end2 Iterator placed on the final position of set 2 (usually `set2.end()`) 
/// @param only_in_set_1 Callback function called when the iterator on set 1 points to an element that is only in set 1
/// @param only_in_set_2 Callback function called when the iterator on set 2 points to an element that is only in set 2
/// @param in_both_sets Callback function called when the two iterators both point to the same element present in both sets
/// 
/// This function works even if the compared sets are of different nature, as long as the
/// iterators return the data to be compared properly ordered and the supplied comparator
/// is able to compare them.
/// 
/// Example:
/// 
///			// Compare a std::set with a sorted std::vector
///   	using t_set1 = std::set<int>;
///   	using t_iter1 = t_set1::const_iterator;
///   	t_set1 s1{7, 10, 20, 30, 40, 50};
///   
///   	using t_set2 = std::vector<int>;
///   	using t_iter2 = t_set2::const_iterator;
///   	t_set2 s2{10, 20, 25, 40, 50, 55};
///   
///   	dastd::sorted_set_diff<t_iter1,t_iter2>(
///   		s1.begin(), s1.end(), 
///   		s2.begin(), s2.end(), 
///   		[](t_iter1 i1) {std::cout << "only_in_set_1: " << *i1 << std::endl;},
///   		[](t_iter2 i2) {std::cout << "only_in_set_2: " << *i2 << std::endl;},
///   		[](t_iter1 i1, t_iter2 i2) {std::cout << "in_both_sets: " << *i1 << "==" << *i2 << std::endl;}
///   	);
///   
/// 
template<class ITER1, class ITER2, class LESS_COMPARATOR = std::less<>>
inline void sorted_set_diff(
	ITER1 beg1, ITER1 end1, 
	ITER2 beg2, ITER2 end2,
	std::function<void(ITER1)> only_in_set_1,
	std::function<void(ITER2)> only_in_set_2,
	std::function<void(ITER1,ITER2)> in_both_sets
)
{
	auto it1 = beg1;
	auto it2 = beg2;
	LESS_COMPARATOR comp;
	while ((it1 != end1) && (it2 != end2)) {
    if (comp(*it1, *it2)) {
			only_in_set_1(it1);
			++it1;
    } 
		else if (comp(*it2, *it1)) {
      only_in_set_2(it2);
			++it2;
    } else {
			in_both_sets(it1, it2);
			++it1; 
			++it2;
    }
	}
	while (it1 != end1) {
		only_in_set_1(it1);
		++it1;
	}
	while (it2 != end2) {
		only_in_set_2(it2);
		++it2; 
	}
}


} // namespace dastd
