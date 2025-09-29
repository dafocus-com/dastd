/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 17-AUG-2025
**/
#pragma once
#include "defs.hpp"
#include "power_of_2.hpp"
#include <atomic>
#include <cassert>

namespace dastd {

/// @brief Value returned when there is no more data from the consumer
static constexpr size_t PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA = std::numeric_limits<size_t>::max();

/// @brief Fast ring buffer
/// @tparam ELEM_TYPE Type of the ring buffer element
/// 
/// Implements a fast ring buffer designed for a producer to feed data to a consumer.
/// 
/// @note This object is thread-safe for consumers; it is *not* thread safe for producers.
///       It expects a single producer. If multiple producers are expected, protect with a mutex.
/// 
/// This ring buffer operates with one producer and multiple consumers. 
/// In this design, the consumers are expected to be faster than the producer:
/// no wait action is provided for the consumer that will simply overwrite the older
/// elements even if not consumed yet.
/// The size must be chosen bit enough to avoid the producer to overwrite the data
/// being read by the consumers.
/// 
/// The position of the ringbuffer is maintained by a index value (`m_readable_abs_top`) which is
/// alway increased. The buffer keeps the last `m_producer_consumer_ring_buffer_size` elements and discards
/// the previous ones.
/// 
template<typename ELEM_TYPE>
class producer_consumer_ring_buffer {

	private:
		/// @brief Size of the buffer
		size_t m_producer_consumer_ring_buffer_size = 0;
		
		/// @brief Precalculated mask
		size_t m_mask = 0;

		/// @brief Indicates the first element that can not be read yet
		std::atomic_size_t m_readable_abs_top = 0;

		/// @brief Indicates the first available writable spot in absolute position
		size_t m_writable_abs_top = 0;

		/// @brief Buffer of `m_producer_consumer_ring_buffer_size` elements
		ELEM_TYPE* m_producer_consumer_ring_buffer = nullptr;

		/// @brief Copy constructor not supported
		producer_consumer_ring_buffer(const producer_consumer_ring_buffer&) = delete;

		/// @brief Operator= not supported
		producer_consumer_ring_buffer& operator=(const producer_consumer_ring_buffer&) = delete;

		/// @brief Convert an absolute position to a position in the ring buffer
		/// @param abs_pos Absolute position
		/// @return 
		size_t producer_consumer_ring_buffer_pos(size_t abs_pos) const {return abs_pos & m_mask;}

	public:
		/// @brief Creates a ring buffer
		/// @param min_size Minmum number of required elements. For efficiency needs, it can allocate more.
		producer_consumer_ring_buffer(size_t min_size);

		/// @brief Creates a ring buffer
		/// @param min_size Minmum number of required elements. For efficiency needs, it can allocate more.
		producer_consumer_ring_buffer() {}

		/// @brief Destructor
		~producer_consumer_ring_buffer() {clear();}

		/// @brief Allocate a ring buffer before use
		void allocate(size_t min_size);

		/// @brief Clear a ring buffer to empty
		void clear();

		/// @brief Push an element on the ring buffer.
		/// @param elem The element to be pushed
		/// 
		/// The element will be immediately available to the consumers.
		void push_back(ELEM_TYPE elem);

		/// @brief Push an element on the ring buffer without publishing.
		/// @param elem The element to be pushed
		/// 
		/// The element will be available to the consumers once `publish()` is called.
		void push_back_no_publish(ELEM_TYPE elem);

		/// @brief Get the maximum contiguous available data 
		/// @return Returns the maximum number of contiguous elements that can be obtained by @ref get_contiguous_buffer
		size_t get_max_contiguous_buffer_size() const {return m_producer_consumer_ring_buffer_size - producer_consumer_ring_buffer_pos(m_writable_abs_top);}

		/// @brief Get a contiguous buffer pointer
		/// @param max_size Maximum size to be allocated
		/// @param avail_size Size actually available; it will be `<=max_size`
		/// @return Returns a pointer to `avail_size` elements to be written
		///	
		/// This method returns a pointer of the remaining part of the internal
		/// ring buffer before looping back. It automatically increases the write
		/// buffer of `avail_size`. Call @ref publish() once done writing or
		/// when ready to publish.
		ELEM_TYPE* get_contiguous_buffer(size_t max_size, size_t& avail_size);

		/// @brief Update the top position after having written
		/// @param new_top New absolute position
		void publish();

		/// @brief The producer will call this to signal that no more data is available.
		/// 
		/// Once called, no further addition is allowed. The @ref get_readable_abs_top and
		/// @ref wait methods will return @ref PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA
		void signal_no_more_data();

		/// @brief Operator[] - Same as @ref get_for_reading
		/// @param abs_pos Absolute position
		/// @return Returns the ring buffer element at the requested position
		/// This method is called by the consumer. It is meant to write the elements
		/// before `m_readable_abs_top`. If attempting to access for writing elements that
		/// have not been marked as readable, it will fail an assertion (in debug mode).
		const ELEM_TYPE& operator[](size_t abs_pos) const;

		/// @brief Returns the top absolute index for reading
		/// @return Returns the current readable absolute top or @ref PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA if
		///         no further data will be available; this can be used by the consumer to terminate itself.
		size_t get_readable_abs_top() const {return m_readable_abs_top.load();}

		/// @brief Lock the calling thread until the readable top absolute index changes
		/// @brief last_known_readable_abs_top Pass the last known `readable_abs_top` value
		/// @return Returns the current readable absolute top or @ref PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA if
		///         no further data will be available; this can be used by the consumer to terminate itself.
		size_t wait(size_t last_known_readable_abs_top) const {m_readable_abs_top.wait(last_known_readable_abs_top); return get_readable_abs_top();}
};

// Creates a ring buffer
template<typename ELEM_TYPE>
inline producer_consumer_ring_buffer<ELEM_TYPE>::producer_consumer_ring_buffer(size_t min_size)
{
	allocate(min_size);
}

// Allocate a ring buffer before use
template<typename ELEM_TYPE>
inline void producer_consumer_ring_buffer<ELEM_TYPE>::allocate(size_t min_size)
{
	clear();
	if (min_size < 2) min_size = 2;
	m_producer_consumer_ring_buffer_size = round_to_nearest_higher_power_of_2<size_t>(min_size);
	m_mask = m_producer_consumer_ring_buffer_size-1;
	m_producer_consumer_ring_buffer = new ELEM_TYPE[m_producer_consumer_ring_buffer_size];
}


// Clear a ring buffer to empty
template<typename ELEM_TYPE>
inline void producer_consumer_ring_buffer<ELEM_TYPE>::clear()
{
	if (m_producer_consumer_ring_buffer) {
		delete[] m_producer_consumer_ring_buffer;
		m_producer_consumer_ring_buffer = nullptr;
	}
	m_producer_consumer_ring_buffer_size = 0;
	m_mask = 0;
	m_readable_abs_top = 0;
	m_writable_abs_top = 0;
}

// Push an element on the ring buffer without publishing.
template<typename ELEM_TYPE>
inline void producer_consumer_ring_buffer<ELEM_TYPE>::push_back_no_publish(ELEM_TYPE elem)
{
	assert(m_writable_abs_top != PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA);
	assert(m_writable_abs_top >= m_readable_abs_top);
	assert((m_writable_abs_top - m_readable_abs_top) < m_producer_consumer_ring_buffer_size);
	m_producer_consumer_ring_buffer[producer_consumer_ring_buffer_pos(m_writable_abs_top)] = elem;
	m_writable_abs_top++;
}


// Push an element on the ring buffer.
template<typename ELEM_TYPE>
inline void producer_consumer_ring_buffer<ELEM_TYPE>::push_back(ELEM_TYPE elem)
{
	push_back_no_publish(elem);
	publish();
}

// Update the top position after having written
template<typename ELEM_TYPE>
inline void producer_consumer_ring_buffer<ELEM_TYPE>::publish()
{
	assert(m_writable_abs_top >= m_readable_abs_top);
	m_readable_abs_top.store(m_writable_abs_top);
	m_readable_abs_top.notify_all();
}

// The producer will call this to signal that no more data is available.
template<typename ELEM_TYPE>
inline void producer_consumer_ring_buffer<ELEM_TYPE>::signal_no_more_data()
{
	// There must not be any unpublished data
	assert(m_writable_abs_top == m_readable_abs_top);
	m_writable_abs_top = PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA;
	m_readable_abs_top.store(m_writable_abs_top);
	m_readable_abs_top.notify_all();

}

// Access an element in the ring buffer for reading
template<typename ELEM_TYPE>
inline const ELEM_TYPE& producer_consumer_ring_buffer<ELEM_TYPE>::operator[](size_t abs_pos) const
{
	assert(abs_pos < m_readable_abs_top);
	return m_producer_consumer_ring_buffer[producer_consumer_ring_buffer_pos(abs_pos)];
}

// Get a contiguous buffer pointer
template<typename ELEM_TYPE>
inline ELEM_TYPE* producer_consumer_ring_buffer<ELEM_TYPE>::get_contiguous_buffer(size_t max_size, size_t& avail_size)
{
	assert(m_writable_abs_top != PRODUCER_CONSUMER_RING_BUFFER_NO_MORE_DATA);
	assert((m_writable_abs_top - m_readable_abs_top + max_size) <= m_producer_consumer_ring_buffer_size);
	size_t curr_ofs = producer_consumer_ring_buffer_pos(m_writable_abs_top);
	size_t remaining = get_max_contiguous_buffer_size();
	assert(remaining > 0);
	avail_size = std::min(max_size, remaining);
	m_writable_abs_top += avail_size;
	return m_producer_consumer_ring_buffer + curr_ofs;
}

} // namespace dastd
