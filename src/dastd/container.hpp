/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include <memory>
#include <set>
#include <vector>
#include <map>

namespace dastd {

template<class CLS>
using vector_shared_ptr = std::vector<std::shared_ptr<CLS>>;


/// @brief Comparator class that uses the "<" operator in the object owned by a shared_ptr
///
/// Example:
///
///     std::set<std::shared_ptr<MyClass>, dastd::shared_ptr_less<MyClass>> myMap;
///
template<class CLS>
struct shared_ptr_less {
    bool operator()(const std::shared_ptr<CLS>& lhs, const std::shared_ptr<CLS>& rhs) const {return (*lhs) < (*rhs);}
};

/// @brief Implementation of the std::set that owns its elements allocated with new and allows searching with a unique key.
///
/// The class CLS must have the `key()` and the `operator<()` methods.
/// Example:
///
///     class ExampleClass2 {
///        public:
///          ExampleClass2(int _val): val(_val) {}
///          int key() const {return val;}
///          bool operator<(const ExampleClass2& o) const {return key() < o.key();}
///          int val;
///         void doSomething();
///      };
///
///      ...
///     dastd::set_key_shared_ptr<int,ExampleClass2> myMap;
///
///     // Add an instance created externally with new
///      ExampleClass2* createdWithNew = new ExampleClass2(100);
///      myMap.insert(std::shared_ptr<ExampleClass2>(createdWithNew));
///
///     // Add instances created locally using the "make_unique" factory
///      myMap.insert(std::make_unique<ExampleClass2>(100));
///      myMap.insert(std::make_unique<ExampleClass2>(120));
///      myMap.insert(std::make_unique<ExampleClass2>(80));
///
///      for (auto & myInstance: myMap) {myInstance->doSomething();}
///
/// NOTE: in case the KEY is a string, use "const std::string" as <KEY>.
/// Never use "const char*", otherwise comparson will be made comparing the
/// pointers and not the string.
///
template<class KEY, class CLS>
class set_key_shared_ptr: public std::set<std::shared_ptr<CLS>, shared_ptr_less<CLS>> {
	public:
		// @brief Iterator type
		typedef typename std::set<std::shared_ptr<CLS>, shared_ptr_less<CLS>>::iterator iterator;

		/// @brief Get an iterator that points on the searched key or the one immediately after if not found
		///
		/// @param key The key that is being searched for
		/// @return The returned iterator "it" can be checked with "it==set.end()" to see if it is after the last one.
		iterator get_lower_bound(const KEY& key) const {
			return std::lower_bound(this->begin(), this->end(), nullptr, [key](const std::shared_ptr<CLS>& p1, const std::shared_ptr<CLS>& p2)->bool {
				KEY v1 = (p1 ? p1->key() : key);
				KEY v2 = (p2 ? p2->key() : key);
				return v1 < v2;
			});
		}

		/// @brief Return the element that matches the key or nullptr if not found
		///
		/// @param key The key that is being searched for
		/// @return Returns the associated object or `nullptr` if not found
		std::shared_ptr<CLS> find_or_null(const KEY& key) const {
			iterator lower_bound = get_lower_bound(key);
			if (lower_bound == this->end()) return nullptr;
			if ((*lower_bound)->key() == key) return (*lower_bound);
			return nullptr;
		}

		/// @brief Return the iteratod that matches the key or end() if not found
		///
		/// @param key The key that is being searched for
		/// @return Returns the associated ierator or `end()` if not found
		iterator find_key(const KEY& key) const {
			iterator lower_bound = get_lower_bound(key);
			if (lower_bound == this->end()) return this->end();
			if ((*lower_bound)->key() == key) return lower_bound;
			return this->end();
		}

		/// @brief Insert an element that has been allocated with new and whose ownership goes to this container
		///
		/// @param obj The object, allocated with `new`, that is to be added to the set
		/// @return Returns a pair where `first` is the iterator pointing at the element, and `second` is true if the object has been added, false if deleted because dupe
		std::pair<iterator, bool> insert(std::shared_ptr<CLS> obj) {return std::set<std::shared_ptr<CLS>, shared_ptr_less<CLS>>::insert(obj);}
};

/// @brief Implementation of a std::map that has a shared_ptr as data element
template<class KEY, class CLS>
class map_shared_ptr: public std::map<KEY, std::shared_ptr<CLS>> {
	public:
		/// @brief Searches for a key
		/// @return Returns a shared pointer to it or nullptr
		std::shared_ptr<CLS> find_or_null(const KEY& key) const {
			if (auto iter = this->find(key); iter != this->end()) return iter->second;
			return nullptr;
		}
};


} // namespace dastd
