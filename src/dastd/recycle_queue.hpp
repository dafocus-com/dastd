/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 25-NOV-2024
**/
/*******************************************************************************************

********************************************************************************************/
#pragma once
#include "defs.hpp"
#include "linklist_dbl.hpp"
#include <atomic>
#include <cassert>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace dastd::recycle_queue {

template<class PAYLOAD> class pool;
template<class PAYLOAD> class queue;
template<class PAYLOAD> class msg;

/// @brief Class representing a message for the recycle_queue subsystem
/// @tparam PAYLOAD Type of the payload
template<class PAYLOAD>
class msg_body {
	friend class pool<PAYLOAD>;
	friend class queue<PAYLOAD>;
	friend class msg<PAYLOAD>;
	private:
		/// @brief Pool of provenience
		pool<PAYLOAD>* m_pool = nullptr;

		/// @brief Linked list implementation
		linklist_dbl_link<msg_body<PAYLOAD>> m_link = this;

		/// @brief Payload of the message
		PAYLOAD m_payload;

		/// @brief Release to the pool
		void clear() {assert(m_pool); m_pool->release(this);}
};

/// @brief Smart pointer to a @ref msg_body object
/// @tparam PAYLOAD Type of the payload
/// @note This type is meant to be manipulated by a single thread at a time
///
/// This object keeps alive a message. A message can be given to
/// a queue or released to the pool.
/// This object is not copiable.
template<class PAYLOAD>
class msg {
	friend class queue<PAYLOAD>;
	friend class pool<PAYLOAD>;

	/// @brief Operator = not allowed
	msg& operator=(const msg<PAYLOAD>&) = delete;

	/// @brief Copy constructor not allowed
	msg(const msg<PAYLOAD>&) = delete;

	private:
		/// @brief Pointer to the actual message body
		msg_body<PAYLOAD>* m_msg_body = nullptr;

		/// @brief Initial constructor called by the queue when it is releasing the message
		msg(msg_body<PAYLOAD>* p_msg_body): m_msg_body(p_msg_body) {
			// This method is called only by the `queue` when it is releasing a message.
			// Thus, it must not be linked.
			assert(!m_msg_body->m_link.is_linked());
		}

	public:
		/// @brief Empty constructor
		msg() {}

		/// @brief Destructor
		~msg() {clear();}

		/// @brief Release the message to the pool
		void clear();

		/// @brief Return true if valid
		bool valid() const {return m_msg_body != nullptr;}

		/// @brief Access the message payload
		/// @note Invoke only if the message is `valid()`
		PAYLOAD& payload() {assert(valid()); return m_msg_body->m_payload;}

		/// @brief Access the message payload (const version)
		/// @note Invoke only if the message is `valid()`
		const PAYLOAD& payload() const {assert(valid()); return m_msg_body->m_payload;}
};


/// @brief Queue that can receive messages coming from a pool or another queue and deliver them
/// @tparam PAYLOAD Type of the payload
/// @note This type is thread safe
/// @warning Destroy or empty all the queues before deleting the pools
template<class PAYLOAD>
class queue {
	protected:
		/// @brief Double linked list root
		linklist_dbl_root<msg_body<PAYLOAD>> m_root;

		/// @brief Mutex for queue access
		std::mutex m_mutex;

	public:
		/// @brief Destructor
		~queue() {clear();}

		/// @brief Clear the queue discarding and releasing all the messages it contains
		void clear();

		/// @brief Returns true if the queue contains at least one message
		bool valid() const;

		/// @brief Add a message to this queue
		/// @param m Message to be added
		void push(msg<PAYLOAD>& m);

		/// @brief Extract a message
		void pop(msg<PAYLOAD>& m);
};

/// @brief Version of the queue containing a locking mechanism that allows a thread to wait until a message is available
/// @tparam PAYLOAD Type of the payload
/// @note This type is thread safe
/// @warning Destroy or empty all the queues before deleting the pools
template<class PAYLOAD>
class blocking_queue: public queue<PAYLOAD> {
	private:
		/// @brief Condition variable used to unlock the listeners
		std::condition_variable m_condvar;

	public:
		/// @brief Wait on the queue for a message to be available
		/// @param timeout Timeout after which this method returns anyway
		///
		/// Returns if a message is available in the queue or if the timeout has
		/// expired. Note that this method can return even if the timeout has
		/// not expired nor any message is available in the queue (the @ref pop()
		/// method can always return an invalid message).
		template<class Rep, class Period>
		void wait(const std::chrono::duration<Rep, Period>& timeout);

		/// @brief Signal the waiting threads that something has happened
		void signal() {m_condvar.notify_all();}

		/// @brief Add a message to this queue and signal
		/// @param m Message to be added
		void push(msg<PAYLOAD>& m);
};

/// @brief Pool of messages that are recycled
/// @tparam PAYLOAD Type of the payload
/// @note This type is thread safe
template<class PAYLOAD>
class pool {
	friend class msg_body<PAYLOAD>;
	private:
		/// @brief Double linked list root
		linklist_dbl_root<msg_body<PAYLOAD>> m_root;

		/// @brief Mutex for queue access
		std::mutex m_mutex;

		/// @brief Release the indicated message to this pool
		void release(msg_body<PAYLOAD>* b);

	public:
		/// @brief Destructor
		~pool() {clear();}

		/// @brief Clear the pool deleting all the messages it currently owns
		void clear();

		/// @brief Get a new message to be enqueued
		void get(msg<PAYLOAD>& m);
};


//------------------------------------------------------------------------------
// (brief) Get a new message to be enqueued
// (return) Returns a valid message
//------------------------------------------------------------------------------
template<class PAYLOAD>
void pool<PAYLOAD>::get(msg<PAYLOAD>& m)
{
	m.clear();
	std::lock_guard<std::mutex> guard(m_mutex);
	if (m_root.first()) {
		m.m_msg_body = &m_root.first()->owner();
		m.m_msg_body->m_link.unlink();
	}
	else {
		m.m_msg_body = new msg_body<PAYLOAD>();
	}
	m.m_msg_body->m_pool = this;
}


//------------------------------------------------------------------------------
// (brief) Release the indicated message to this pool
//------------------------------------------------------------------------------
template<class PAYLOAD>
void pool<PAYLOAD>::release(msg_body<PAYLOAD>* b)
{
	assert(b != nullptr);
	std::lock_guard<std::mutex> guard(m_mutex);
	// We should release the messages to their own pool
	assert(b->m_pool == this);
	assert(!b->m_link.is_linked());
	b->m_pool = nullptr;
	b->m_link.link_as_first(m_root);
}


//------------------------------------------------------------------------------
// (brief) Release the message to the pool
//------------------------------------------------------------------------------
template<class PAYLOAD>
void msg<PAYLOAD>::clear()
{
	if (!valid()) return;
	m_msg_body->clear();
	m_msg_body = nullptr;
}


//------------------------------------------------------------------------------
// (brief) Clear the queue discarding and releasing all the messages it contains
//------------------------------------------------------------------------------
template<class PAYLOAD>
void queue<PAYLOAD>::clear()
{
	std::lock_guard<std::mutex> guard(m_mutex);
	while(m_root.first()) {
		msg_body<PAYLOAD>& b = m_root.first()->owner();
		b.m_link.unlink();
		b.clear();
	}
}


//------------------------------------------------------------------------------
// (brief) Returns true if the queue contains at least one message
//------------------------------------------------------------------------------
template<class PAYLOAD>
bool queue<PAYLOAD>::valid() const
{
	std::lock_guard<std::mutex> guard(((queue<PAYLOAD>*)this)->m_mutex);
	return (m_root.first() != nullptr);
}

//--------------------------------------------------7----------------------------
// (brief) Add a message to this queue
//------------------------------------------------------------------------------
template<class PAYLOAD>
void queue<PAYLOAD>::push(msg<PAYLOAD>& m)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	assert(m.valid());
	assert(!m.m_msg_body->m_link.is_linked());
	m.m_msg_body->m_link.link_as_last(m_root);
	m.m_msg_body = nullptr;
}


//------------------------------------------------------------------------------
// (brief) Extract a message
// (return) Returns the first message in the queue; returned message is invalid if the queue is empty
//------------------------------------------------------------------------------
template<class PAYLOAD>
void queue<PAYLOAD>::pop(msg<PAYLOAD>& m)
{
	m.clear();
	std::lock_guard<std::mutex> guard(m_mutex);
	linklist_dbl_link<msg_body<PAYLOAD>>* first = m_root.first();
	if (first == nullptr) return;

	m.m_msg_body = &first->owner();
	m.m_msg_body->m_link.unlink();
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
template<class PAYLOAD> template<class Rep, class Period>
void blocking_queue<PAYLOAD>::wait(const std::chrono::duration<Rep, Period>& timeout)
{
	std::unique_lock<std::mutex> lock(this->m_mutex);
	m_condvar.wait_for(lock, timeout);
}

//------------------------------------------------------------------------------
// (brief) Add a message to this queue
//------------------------------------------------------------------------------
template<class PAYLOAD>
void blocking_queue<PAYLOAD>::push(msg<PAYLOAD>& m)
{
	queue<PAYLOAD>::push(m);
	signal();
}


//------------------------------------------------------------------------------
// (brief) Clear the pool deleting all the messages it currently owns
//------------------------------------------------------------------------------
template<class PAYLOAD>
void pool<PAYLOAD>::clear()
{
	std::lock_guard<std::mutex> guard(m_mutex);
	while(m_root.first()) {
		msg_body<PAYLOAD>* b = &m_root.first()->owner();
		b->m_link.unlink();
		delete b;
	}
}


} // namespace dastd::recycle_queue
