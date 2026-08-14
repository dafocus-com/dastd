/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-DEC-2024
**/
#pragma once
#include "defs.hpp"
#include <atomic>
#include <cassert>
#include <mutex>
#include <chrono>
#include <queue>
#include <condition_variable>
#include "destroyer.hpp"

namespace dastd {

/// @brief Implementation of a std::queue with thread protection
/// @tparam ELEMENT   Type of the element to be managed
/// @tparam DESTROYER Class with a static method "destroy" that receives the element to be deleted when
///                   the queue is deleted with elements still in it; use `destroyer_delete<ELEMENT>`
///                   if the element is to be deleted with `delete`.
template<class ELEMENT, class DESTROYER=destroyer<ELEMENT>>
class multithread_queue {
	protected:
		/// @brief Internal queue object
		std::queue<ELEMENT> m_queue;

		/// @brief Mutex for queue access
		std::mutex m_mutex;

		/// @brief Condition variable used to unlock the listeners
		std::condition_variable m_condvar;

	public:
		/// @brief Destructor
		~multithread_queue() {clear();}

		/// @brief Clear the queue discarding and releasing all the messages it contains
		void clear();

		/// @brief Returns true if the queue contains at least one message
		bool valid() const;

		/// @brief Add a message to this queue
		/// @param m Message to be added
		void push(ELEMENT m);

		/// @brief Extract a message
		/// @param m Reference to the object that will receive a copy of the extracted element
		/// @return Returns true if the queue returned an element, false if emtpy
		bool pop(ELEMENT& m);

		/// @brief Wait on the queue for a message to be available
		/// @param timeout Timeout after which this method returns anyway; for example `std::chrono::milliseconds(2000)`
		///
		/// Returns if a message is available in the queue or if the timeout has
		/// expired. Note that this method can return even if the timeout has
		/// not expired nor any message is available in the queue (the @ref pop()
		/// method can always return an invalid message).
		template<class Rep, class Period>
		void wait(const std::chrono::duration<Rep, Period>& timeout);

		/// @brief Wait on the queue for a message to be available and return it if so
		/// @param m Reference to the object that will receive a copy of the extracted element
		/// @param timeout Timeout after which this method returns anyway; for example `std::chrono::milliseconds(2000)`
		///
		/// The combination of `wait` followed by `pop`.
		template<class Rep, class Period>
		bool wait_and_pop(ELEMENT& m, const std::chrono::duration<Rep, Period>& timeout);

		/// @brief Signal the waiting threads that something has happened
		void signal() {m_condvar.notify_all();}
};

//------------------------------------------------------------------------------
// (brief) Clear the queue discarding and releasing all the messages it contains
//------------------------------------------------------------------------------
template<class ELEMENT, class DESTROYER>
void multithread_queue<ELEMENT,DESTROYER>::clear()
{
	std::lock_guard<std::mutex> guard(m_mutex);
	while(!m_queue.empty()) {
		DESTROYER::destroy(m_queue.front());
		m_queue.pop();
	}
}


//------------------------------------------------------------------------------
// (brief) Returns true if the queue contains at least one message
//------------------------------------------------------------------------------
template<class ELEMENT, class DESTROYER>
bool multithread_queue<ELEMENT,DESTROYER>::valid() const
{
	std::lock_guard<std::mutex> guard(((multithread_queue<ELEMENT,DESTROYER>*)this)->m_mutex);
	return !m_queue.empty();
}

//--------------------------------------------------7----------------------------
// (brief) Add a message to this queue
//------------------------------------------------------------------------------
template<class ELEMENT, class DESTROYER>
void multithread_queue<ELEMENT,DESTROYER>::push(ELEMENT m)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	m_queue.push(m);
	signal();
}

//------------------------------------------------------------------------------
// (brief) Extract a message
// (return) Returns the first message in the queue; returned message is invalid if the queue is empty
//------------------------------------------------------------------------------
template<class ELEMENT, class DESTROYER>
bool multithread_queue<ELEMENT,DESTROYER>::pop(ELEMENT& m)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	if (m_queue.empty()) return false;
	m = m_queue.front();
	m_queue.pop();
	return true;
}


//------------------------------------------------------------------------------
// (brief) Wait on the queue for a message to be available
// (param) timeout Timeout after which this method returns anyway
//
// Returns if a message is available in the queue or if the timeout has
// expired. Note that this method can return even if the timeout has
// not expired nor any message is available in the queue (the @ref pop()
// method can always return an invalid message).
//------------------------------------------------------------------------------
template<class ELEMENT, class DESTROYER> template<class Rep, class Period>
void multithread_queue<ELEMENT,DESTROYER>::wait(const std::chrono::duration<Rep, Period>& timeout)
{
	std::unique_lock<std::mutex> lock(this->m_mutex);
	m_condvar.wait_for(lock, timeout);
}

//------------------------------------------------------------------------------
// (brief) Wait on the queue for a message to be available and return it if so
// (param) m Reference to the object that will receive a copy of the extracted element
// (param) timeout Timeout after which this method returns anyway; for example `std::chrono::milliseconds(2000)`
//
// The combination of `wait` followed by `pop`.
//------------------------------------------------------------------------------
template<class ELEMENT, class DESTROYER> template<class Rep, class Period>
bool multithread_queue<ELEMENT,DESTROYER>::wait_and_pop(ELEMENT& m, const std::chrono::duration<Rep, Period>& timeout)
{
	if (timeout != std::chrono::duration<Rep, Period>::zero()) {
		std::unique_lock<std::mutex> lock(this->m_mutex);
		m_condvar.wait_for(lock, timeout);
	}
	return pop(m);
}

} // namespace dastd
