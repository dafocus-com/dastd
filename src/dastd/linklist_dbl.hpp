/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-SEP-2024
**/
/// @class dastd::linklist_dbl_link
/// @brief Implements a very basic double linked list
/// @note  This is class is not thread-safe and it must be protected externally with a mutex
///
/// This class implements the raw management of a double linked list.
/// It is up to the owner to call "unlink()" when done.
/// It is also up to the owner to manage the locking in case
/// of multithread access.
///
/// EXAMPLE:
///
///       class LinkListTest {
///       public:
///         /// @brief Contructor
///         LinkListTest(uint32_t value=0): m_value(value), m_link(this) {}
///
///         /// @brief Destructor
///         ~LinkListTest() {m_link.unlink();}
///
///         /// @brief Internal value
///         uint32_t m_value=0;
///
///         /// @brief Link maintenance
///         dastd::linklist_dbl_link<LinkListTest> m_link;
///       };
///
///       dastd::linklist_dbl_root<LinkListTest> root;
///       LinkListTest t1(1);
///       t1.m_link.link_as_last(root);
///
///       dastd::linklist_dbl_link<LinkListTest>* link;
///       for (link=root.first(); link!=nullptr; link=link->next()) {
///          std::cout << " " << link->owner().m_value;
///       }

#pragma once
#include "defs.hpp"

namespace dastd {
	template<typename OWNERTYPE> class linklist_dbl_root;

	/// @brief Link object to be placed inside the object that needs to be linked
	/// @tparam OWNERTYPE Type of the owner object
	template<typename OWNERTYPE>
	class linklist_dbl_link {
		// This object is not copiable
		linklist_dbl_link<OWNERTYPE>& operator=(const linklist_dbl_link<OWNERTYPE>& o) = delete;
		linklist_dbl_link(const linklist_dbl_link<OWNERTYPE>& o) = delete;

		public:
			/// @brief Constructor
			/// @param owner Object owning this link
			linklist_dbl_link(OWNERTYPE* owner): m_owner(owner) {}

			/// @brief Destructor
			~linklist_dbl_link() {assert(!is_linked()); /* Call unlink() before destroying */}

			/// @brief Return the owner object
			OWNERTYPE& owner() const {return *m_owner;}

			/// @brief Return the next or nullptr if none
			linklist_dbl_link<OWNERTYPE>* next() const {return m_next;}

			/// @brief Return the previous or nullptr if none
			linklist_dbl_link<OWNERTYPE>* prev() const {return m_prev;}

			/// @brief Link as first element; if already linked, it is unlinked and relinked
			void link_as_first(linklist_dbl_root<OWNERTYPE>& root);

			/// @brief Link as last element; if already linked, it is unlinked and relinked
			void link_as_last(linklist_dbl_root<OWNERTYPE>& root);

			/// @brief Unlink if not already unlinked
			void unlink();

			/// @brief Check if linked
			/// @return Return true if linked
			bool is_linked() const {return m_root != nullptr;}

			#ifdef DASTD_IS_DEBUG
			/// @brief Check the integrity
			void debug_check_integrity();
			#else
			void debug_check_integrity() {}
			#endif

		private:
			/// @brief Owner object
			OWNERTYPE* m_owner;

			/// @brief Next element in the list
			linklist_dbl_link<OWNERTYPE>* m_next = nullptr;

			/// @brief Previous element in the list
			linklist_dbl_link<OWNERTYPE>* m_prev = nullptr;

			/// @brief Pointer to the root object if linked
			linklist_dbl_root<OWNERTYPE>* m_root = nullptr;
	};

	/// @brief Root object to be placed inside the object that manages the linked list
	/// @tparam OWNERTYPE Type of the owner object
	template<typename OWNERTYPE>
	class linklist_dbl_root {
		friend class linklist_dbl_link<OWNERTYPE>;

		// This object is not copiable
		linklist_dbl_root<OWNERTYPE>& operator=(const linklist_dbl_root<OWNERTYPE>& o) = delete;
		linklist_dbl_root(const linklist_dbl_root<OWNERTYPE>& o) = delete;
		public:
			/// @brief Constructor
			linklist_dbl_root() {}

			/// @brief Destructor
			~linklist_dbl_root() {assert(m_first == nullptr); /* Call unlink_all() before destroying */}

			/// @brief Unlink all elements
			void unlink_all() {while (m_first) m_first->unlink();}

			/// @brief Returns the first link or nullptr if empty
			linklist_dbl_link<OWNERTYPE>* first() const {return m_first;}

			/// @brief Returns the last link or nullptr if empty
			linklist_dbl_link<OWNERTYPE>* last() const {return m_last;}

		private:
			/// @brief First element in the list
			linklist_dbl_link<OWNERTYPE>* m_first = nullptr;

			/// @brief Last element in the list
			linklist_dbl_link<OWNERTYPE>* m_last = nullptr;
	};

	// (brief) Unlink if not already unlinked
	template<typename OWNERTYPE>
	void linklist_dbl_link<OWNERTYPE>::unlink() {
		debug_check_integrity();
		if (!is_linked()) return;
		if (m_prev == nullptr) m_root->m_first = m_next;
		if (m_next == nullptr) m_root->m_last = m_prev;
		if (m_next) {
			m_next->m_prev = m_prev;
		}
		if (m_prev) {
			m_prev->m_next = m_next;
		}
		if (m_next) m_next->debug_check_integrity();
		if (m_prev) m_prev->debug_check_integrity();

		m_next = nullptr;
		m_prev = nullptr;
		m_root = nullptr;
		debug_check_integrity();
	}

	// (brief) Link as first element; if already linked, it is unlinked and relinked
	template<typename OWNERTYPE>
	void linklist_dbl_link<OWNERTYPE>::link_as_first(linklist_dbl_root<OWNERTYPE>& root) {
		unlink();
		m_root = &root;
		m_next = root.m_first;
		root.m_first = this;
		if (m_next) {
			m_next->m_prev = this;
			m_next->debug_check_integrity();
		}
		else {
			root.m_last = this;
		}
		debug_check_integrity();
	}

	// (brief) Link as last element; if already linked, it is unlinked and relinked
	template<typename OWNERTYPE>
	void linklist_dbl_link<OWNERTYPE>::link_as_last(linklist_dbl_root<OWNERTYPE>& root) {
		unlink();
		m_root = &root;
		m_prev = root.m_last;
		root.m_last = this;
		if (m_prev) {
			m_prev->m_next = this;
			m_prev->debug_check_integrity();
		}
		else {
			root.m_first = this;
		}
	}

	#ifdef DASTD_IS_DEBUG
	// (brief) Check the integrity
	template<typename OWNERTYPE>
	void linklist_dbl_link<OWNERTYPE>::debug_check_integrity()
	{
		if (m_root == nullptr) {
			assert(m_prev == nullptr);
			assert(m_next == nullptr);
			return;
		}
		if (m_prev != nullptr) assert(m_prev->m_next == this);
		else assert(m_root->m_first == this);

		if (m_next != nullptr) assert(m_next->m_prev == this);
		else assert(m_root->m_last == this);
	}
	#endif

} // namespace dastd
