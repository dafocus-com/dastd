/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 13-SEP-2023
**/
/// @class dastd::linklist_link
/// @brief Implements a very basic linked list
///
/// This class implements the raw management of a linked list.
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
///         /// @brief
///         dastd::linklist_link<LinkListTest> m_link;
///       };
///
///       dastd::linklist_root<LinkListTest> root;
///       LinkListTest t1(1);
///       t1.m_link.link(root);
///
///       dastd::linklist_link<LinkListTest>* link;
///       for (link=root.first(); link!=nullptr; link=link->next()) {
///          std::cout << " " << link->owner().m_value;
///       }

#pragma once
#include "defs.hpp"

namespace dastd {
	template<typename OWNERTYPE> class linklist_root;

	/// @brief Link object to be placed inside the object that needs to be linked
	/// @tparam OWNERTYPE Type of the owner object
	template<typename OWNERTYPE>
	class linklist_link {
		// This object is not copiable
		linklist_link<OWNERTYPE>& operator=(const linklist_link<OWNERTYPE>& o) = delete;
		linklist_link(const linklist_link<OWNERTYPE>& o) = delete;

		public:
			/// @brief Constructor
			/// @param owner Object owning this link
			linklist_link(OWNERTYPE* owner): m_owner(owner) {}

			/// @brief Destructor
			~linklist_link() {assert(!is_linked()); /* Call unlink() before destroying */}

			/// @brief Return the owner object
			OWNERTYPE& owner() const {return *m_owner;}

			/// @brief Return the next or nullptr if none
			linklist_link<OWNERTYPE>* next() const {return m_next;}

			/// @brief Link; if already linked, it is unlinked
			void link(linklist_root<OWNERTYPE>& root);

			/// @brief Unlink if not already unlinked
			void unlink();

			/// @brief Check if linked
			/// @return Return true if linked
			bool is_linked() const {return m_prev_ptr != nullptr;}

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
			linklist_link<OWNERTYPE>* m_next = nullptr;

			/// @brief Address of the pointer in the previous object that points to this
			linklist_link<OWNERTYPE>** m_prev_ptr = nullptr;
	};

	/// @brief Root object to be placed inside the object that manages the linked list
	/// @tparam OWNERTYPE Type of the owner object
	template<typename OWNERTYPE>
	class linklist_root {
		friend class linklist_link<OWNERTYPE>;

		// This object is not copiable
		linklist_root<OWNERTYPE>& operator=(const linklist_root<OWNERTYPE>& o) = delete;
		linklist_root(const linklist_root<OWNERTYPE>& o) = delete;
		public:
			/// @brief Constructor
			linklist_root() {}

			/// @brief Destructor
			~linklist_root() {assert(m_first == nullptr); /* Call unlink_all() before destroying */}

			/// @brief Unlink all elements
			void unlink_all() {while (m_first) m_first->unlink();}

			/// @brief Returns the first link or nullptr if not available
			linklist_link<OWNERTYPE>* first() const {return m_first;}

		private:
			/// @brief First element in the list
			linklist_link<OWNERTYPE>* m_first = nullptr;
	};

	// (brief) Unlink if not already unlinked
	template<typename OWNERTYPE>
	void linklist_link<OWNERTYPE>::unlink() {
		debug_check_integrity();
		if (!is_linked()) return;
		(*m_prev_ptr) = m_next;
		if (m_next) {
			m_next->m_prev_ptr = m_prev_ptr;
			m_next->debug_check_integrity();
		}
		m_next = nullptr;
		m_prev_ptr = nullptr;
		debug_check_integrity();
	}

	// (brief) Unlink if not already unlinked
	template<typename OWNERTYPE>
	void linklist_link<OWNERTYPE>::link(linklist_root<OWNERTYPE>& root) {
		unlink();
		m_next = root.m_first;
		m_prev_ptr = &root.m_first;
		root.m_first = this;
		if (m_next) {
			m_next->m_prev_ptr = &m_next;
			m_next->debug_check_integrity();
		}
		debug_check_integrity();
	}

	#ifdef DASTD_IS_DEBUG
	// (brief) Check the integrity
	template<typename OWNERTYPE>
	void linklist_link<OWNERTYPE>::debug_check_integrity()
	{
		if (m_prev_ptr) {
			assert((*m_prev_ptr) == this);
			assert((m_next==nullptr) || (m_next->m_prev_ptr == &m_next));
		}
		else {
			assert(m_next == nullptr);
		}
	}
	#endif

} // namespace dastd
