/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 02-APR-2024
*
*
*/
#pragma once
#include "defs.hpp"
#include "ostream_basic.hpp"
#include "sink_byte_buffered.hpp"
#include "source_buffered.hpp"
#include <functional>
#include <mutex>

namespace dastd {

/// @brief High performance raw-bytes queue
///
/// The "chunked_binary_queue" is a binary queue that allows pushing bytes on
/// one side and extracting them from the other side.
/// Internally it is based on a linked list of fixed size chunks.
///
/// MULTITASKING/REENTRANCY
/// =======================
/// This class is thread protected.
///
class chunked_binary_queue: public sink_byte_buffered, public source_buffered<char> {
	private:
		/// @brief Chunk header
		///
		/// This structure is placed at the beginning of each chunk. The rest of
		/// the chunk is available to store the data
		struct chunked_binary_queue_header {
			/// @brief Pointer to the next chunk
			chunked_binary_queue_header* m_next_chunk;

			/// @brief Access the memory chunk for reading and writing
			uint8_t* payload() {return ((uint8_t*)this)+sizeof(*this);}

			/// @brief Access the memory chunk for reading only
			const uint8_t* payload() const {return ((const uint8_t*)this)+sizeof(*this);}
		};

		/// @brief Size of the chunk payload.
		///
		/// This is the size of the available memory. The actual allocation request
		/// will be "m_chunk_payload_size+sizeof(chunked_binary_queue_header)"
		size_t m_chunk_payload_size = 0;

		/// @brief Number of bytes in the queue
		size_t m_bytes_count = 0;

		/// @brief First available byte in the last chunk. If equal to "m_chunkSize", the
		/// chunk is full. Used when writing;
		size_t m_first_avail_byte_in_last_chunk = 0;

		/// @brief First unread byte in the first chunk. This is used when reading
		/// and it is updated by the "discard" method.
		size_t m_first_unread_byte_in_first_chunk = 0;

		/// @brief Pointer to the first chunk or nullptr if no chunks have been allocated
		chunked_binary_queue_header* m_first_chunk = nullptr;

		/// @brief Pointer to the last chunk or nullptr if no chunks have been allocated
		chunked_binary_queue_header* m_last_chunk = nullptr;

		/// @brief Semaphore
		std::recursive_mutex m_mutex;

	public:
		static constexpr size_t chunk_size_default = 8192;

		/// @brief Constructor
		/// @param chunk_size Size of the chunk allocated by this object, including the header;
		///                  The memory available to the user on each chunk will be less than that
		///                  of the size of the `chunked_binary_queue_header` header.
		chunked_binary_queue(size_t chunk_size=chunk_size_default);

		/// @brief Destructor
		virtual ~chunked_binary_queue() {clear();}

		/// @brief Reset the memory freeing all the resources
		void clear();

		/// @brief Iterate from the internal data starting from the first available
		/// byte up to 'bytes'.
		///
		/// The iterator can change the internal data.
		///
		/// @param callback Function allowed to modify the byte; it must return `true` to continue or `false` to abandon
		/// @param bytes Maximum number of bytes that is allowed to extract; use bytes=SIZE_MAX to process all bytes.
		void iterate(std::function<bool(char& ch, size_t offset)> callback, size_t bytes = SIZE_MAX);

		/// @brief Iterate from the internal data starting from the first available
		/// byte up to 'bytes'.
		///
		/// The iterator can read the internal data.
		///
		/// @param callback Function allowed to red the byte; it must return `true` to continue or `false` to abandon
		/// @param length Maximum number of bytes that is allowed to extract; use bytes=SIZE_MAX to process all bytes.
		void iterate(std::function<bool(char ch, size_t offset)> callback, size_t bytes = SIZE_MAX) const;

		/// @brief Blocking method to write data to the sink
		///
		/// This method will return when the data has been sent. This method will
		/// do its best to queue the data and return immediately.
		/// In case of catastrophic error, it will throw an exception.
		///
		/// @param data      Pointer to the data to be sent
		/// @param data_size Amount of bytes it will send
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual void sink_write(const void* data, size_t data_size) override;

		/// @brief Check if the queue is empty
		/// @return Returns `true` if the outgoing queue is empty
		virtual bool empty() const override {return tentative_count() == 0;}

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read up to `data_size` bytes.
		/// If less (or even zero) bytes are available, it will read them
		/// and return.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns the number of characters actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_read(char* data, size_t data_size) override;

		/// @brief Return the number of bytes that could be read at the next tentative_read
		///
		/// This method returns the current max value that can be obtained by calling
		/// `tentative_read` right now. Note that between the call to `tentative_count` and
		/// `tentative_read`, the value might changed. So there is no guarantee that the
		/// following calls will conform to this value.
		///
		/// This call is meant to allow preallocation of data with a good guess.
		virtual size_t tentative_count() const override;

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read exactly `data_size` bytes.
		/// If less bytes are available, it will not read anything and
		/// it will return false.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns `true` if `data_size` bytes have been read; returns `false` if nothing has been read (still not enough data)
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual bool atomic_read(char* data, size_t data_size) override;

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read exactly `data_size` bytes.
		/// If less bytes are available, it will not read anything and
		/// it will return false.
		///
		/// @param data      Available data will be appended to the `data` string
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns `true` if `data_size` bytes have been read; returns `false` if nothing has been read (still not enough data)
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual bool atomic_read(std::string& data, size_t data_size) override;

		/// @brief Execute an atomic iteration on the data
		///
		/// This method will iterate over the first `data_size` bytes in the queue.
		/// For each byte, it will call the `callback` function, allowing it to change the value if
		/// desired.
		/// If the queue does not contain at least `data_size` bytes, no processing will be done
		/// and it will return `false`.
		///
		/// Example
		///
		///     myqueue.atomic_iterate([](uint8_t& ch, size_t offset)->bool {/*...*/ return true}, 100);
		///
		/// @param callback  Function invoked for each byte. The byte value can be read and changed by the function.
		///                  Return `true` to continue (up to `data_size` bytes), `false` to terminate.
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns `true` if `data_size` characters have been processed; returns `false` if no processing was done because not enough characters were available.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual bool atomic_iterate(std::function<bool(char& ch, size_t offset)> callback, size_t data_size) override;

		/// @brief Non-blocking method to read data from the source without extracting it
		///
		/// This method will attempt to read up to `data_size` bytes.
		/// If less (or even zero) bytes are available, it will read them
		/// and return. The data will not be extracted from the buffer so the
		/// next time the same data will be returned again.
		/// Call `tentative_discard` to discard the data that has been actually used.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns the number of characters actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_peek(char* data, size_t data_size) override;

	private:
		/// @brief Run a consisency check on this object. Asserts in case of errors.
		///
		/// Used during debug to make sure the implementation is not flawed.
		void debug_check() const;

		/// @brief Return the total number of all the bytes available in the queue
		size_t get_bytes_count() const {return m_bytes_count;}

		/// @brief Writes a memory block to the stream.
		void write_binary(const void* source, size_t bytes);

		/// @brief Extract the data from this object and copy it to the given memory target
		/// @param destination Target buffer that must be able to receive `min(bytes, get_bytes_count())` bytes
		/// @param bytes Maxmimum number of bytes to be extracted
		/// @return Returns the number of bytes actually read
		size_t read_binary(void* destination, size_t bytes = SIZE_MAX);

		/// @brief Extract the data from this object and copy it to the given memory target
		///
		/// @param destination Target string
		/// @param bytes Maxmimum number of bytes to be extracted
		void read_binary(std::string& destination, size_t bytes = SIZE_MAX);

		/// @brief Request some contiguous memory to write up to `avail_bytes`
		/// @param requested_bytes Number of bytes the caller needs to write
		/// @param avail_bytes Number of bytes the caller must write at this turn
		/// @return Returns a pointer to the contiguous bytes buffer that will host `avail_bytes` bytes
		///
		/// Once called, the queue will have `avail_bytes` more. The caller will have
		/// to write `avail_bytes` at the pointer that has been returned.
		void* request_memory_for_writing(size_t requested_bytes, size_t& avail_bytes);

		/// @brief Request some contiguous memory to read up to `avail_bytes`
		/// @param requested_bytes Number of bytes the caller needs to read; it must be that `requested_bytes<=get_bytes_count()`
		/// @param avail_bytes Number of bytes the caller must read at this turn
		/// @return Returns a pointer to the contiguous bytes buffer that host `avail_bytes` incoming bytes (avail_bytes<=requested_bytes)
		///
		/// Once called, the queue will have `avail_bytes` more. The caller will have
		/// to read `avail_bytes` at the pointer that has been returned.
		///
		/// Once the caller has finished processing the buffer, the buffer must be
		/// released by calling "discard(avail_bytes)". Then, the client can `request_memory_for_reading`
		/// again to obtains more bytes, if required.
		const void* request_memory_for_reading(size_t requested_bytes, size_t& avail_bytes) const;

		/// @brief This method discards "discard_bytes" from the input and frees the unused memory.
		/// @param discard_bytes Number of bytes to be discarded; it must be `<=get_bytes_count()`
		///
		/// This method can be used to discard unwanted bytes or bytes that have already
		/// been processed due to a previous call to `request_memory_for_reading`.
		void discard(size_t discard_bytes);

		/// @brief Allocate a new chunk
		void add_new_chunk();

		/// @brief Return the number of bytes available in the last chunk
		size_t get_last_chunk_free_bytes();

		/// @brief Free a `chunked_binary_queue_header` and the memory following it
		static void free_chunk(chunked_binary_queue_header* hdr) {delete[]((uint8_t*)hdr);}
};


//------------------------------------------------------------------------------
// (brief) Constructor
// (param) chunk_size Size of the chunk allocated by this object, including the header;
//                  The memory available to the user on each chunk will be less than that
//                  of the size of the `chunked_binary_queue_header` header.
//------------------------------------------------------------------------------
inline chunked_binary_queue::chunked_binary_queue(size_t chunk_size):
	m_chunk_payload_size(chunk_size-sizeof(chunked_binary_queue_header))
{
	debug_check();
}


//------------------------------------------------------------------------------
// (brief) Run a consisency check on this object. Asserts in case of errors.
//
// Used during debug to make sure the implementation is not flawed.
//------------------------------------------------------------------------------
inline void chunked_binary_queue::debug_check() const
{
#ifdef DASTD_IS_DEBUG
	// The m_first_chunk/m_last_chunk pointers must either both nullptr or both valid
	assert((m_last_chunk==nullptr) == (m_first_chunk==nullptr));

	// The "m_first_avail_byte_in_last_chunk" must not exceed the "m_chunk_payload_size"
	assert(m_first_avail_byte_in_last_chunk <= m_chunk_payload_size);

	// The "m_first_unread_byte_in_first_chunk" must always be less than "m_chunk_payload_size".
	// When "m_first_unread_byte_in_first_chunk" reaches "m_chunk_payload_size", the chunk must
	// have been discarded.
	assert(m_first_unread_byte_in_first_chunk < m_chunk_payload_size);

	if (m_last_chunk == nullptr) {
		assert(m_bytes_count == 0);
		assert(m_first_avail_byte_in_last_chunk == 0);
	}
	else {
		assert(m_last_chunk->m_next_chunk == nullptr);
	}

	// Check the linked list
	if (m_first_chunk != nullptr) {
		unsigned chunks = 0;
		const chunked_binary_queue_header* curr = m_first_chunk;
		for (;;) {
			chunks++;
			// If m_next_chunk is nullptr, this must be the last chunk
			if (curr->m_next_chunk == nullptr) {
				assert(curr == m_last_chunk);
				break;
			}
			curr = curr->m_next_chunk;
		}

		// Calculate the memory stored in the chunks
		size_t chunks_total_memory = chunks*m_chunk_payload_size;

		// Calculate the actual used memory
		size_t actual_memory = chunks_total_memory-m_first_unread_byte_in_first_chunk-(m_chunk_payload_size-m_first_avail_byte_in_last_chunk);
		assert(actual_memory == m_bytes_count);
	}
#endif
}

//------------------------------------------------------------------------------
// (brief) Allocate a new chunk
//------------------------------------------------------------------------------
inline void chunked_binary_queue::add_new_chunk()
{
	debug_check();

	// We can allocate a new chunk only if there are no chunks or if
	// the last one is 100% fill.
	assert((m_last_chunk==nullptr) || (m_first_avail_byte_in_last_chunk == m_chunk_payload_size));

	// Allocate a new chunk and get the actual size of the chunk.
	// The memory manager might allocate a memory segment that is bigger than the
	// one requested by us, so we want to use it completely.
	size_t requested_size = m_chunk_payload_size+sizeof(chunked_binary_queue_header);
	chunked_binary_queue_header* new_chunk = (chunked_binary_queue_header*)new uint8_t[requested_size];

	// If this is the first allocation, we update the "m_chunk_payload_size"
	if (m_last_chunk == nullptr) {
		m_chunk_payload_size = requested_size-sizeof(chunked_binary_queue_header);
		m_first_chunk = new_chunk;
	}
	else {
		m_last_chunk->m_next_chunk = new_chunk;
	}
	m_last_chunk = new_chunk;
	new_chunk->m_next_chunk = nullptr;
	m_first_avail_byte_in_last_chunk = 0;


	debug_check();
}

//------------------------------------------------------------------------------
// (brief) Return the number of bytes available in the last chunk
//------------------------------------------------------------------------------
inline size_t chunked_binary_queue::get_last_chunk_free_bytes()
{
	if (m_last_chunk == nullptr) return 0;
	assert(m_first_avail_byte_in_last_chunk <= m_chunk_payload_size);
	return m_chunk_payload_size-m_first_avail_byte_in_last_chunk;
}


//------------------------------------------------------------------------------
// (brief) Reset the memory freeing all the resources
//------------------------------------------------------------------------------
inline void chunked_binary_queue::clear()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	debug_check();
	chunked_binary_queue_header* curr;
	chunked_binary_queue_header* next;
	for (curr=m_first_chunk; curr != nullptr; curr=next) {
		next = curr->m_next_chunk;
		free_chunk(curr);
	}
	m_bytes_count = 0;
	m_first_avail_byte_in_last_chunk = 0;
	m_first_unread_byte_in_first_chunk = 0;
	m_first_chunk = nullptr;
	m_last_chunk = nullptr;
	debug_check();
}

//------------------------------------------------------------------------------
// (brief) Request some memory to write up to `avail_bytes`
// (param) requested_bytes Number of bytes the caller needs to write
// (param) avail_bytes Number of bytes the caller must write at this turn
// (return) Returns a pointer to the contiguous bytes buffer that will host `avail_bytes`
//
// Once called, the queue will have `avail_bytes` more. The caller will have
// to write `avail_bytes` at the pointer that has been returned.
//------------------------------------------------------------------------------
inline void* chunked_binary_queue::request_memory_for_writing(size_t requested_bytes, size_t& avail_bytes)
{
	debug_check();
	size_t free_bytes_in_last_chunk = get_last_chunk_free_bytes();
	if (free_bytes_in_last_chunk == 0) {
		add_new_chunk();
		free_bytes_in_last_chunk = get_last_chunk_free_bytes();
	}

	uint8_t* bytes = m_last_chunk->payload()+m_first_avail_byte_in_last_chunk;
	avail_bytes = std::min(free_bytes_in_last_chunk, requested_bytes);
	m_first_avail_byte_in_last_chunk += avail_bytes;
	m_bytes_count += avail_bytes;

	debug_check();
	return (void*)bytes;
}


//------------------------------------------------------------------------------
// (brief) Request some contiguous memory to read up to `avail_bytes`
// (param) requested_bytes Number of bytes the caller needs to read; it must be that `requested_bytes<=get_bytes_count()`
// (param) avail_bytes Number of bytes the caller must read at this turn
// (return) Returns a pointer to the contiguous bytes buffer that host `avail_bytes` incoming bytes (avail_bytes<=requested_bytes)
//
// Once called, the queue will have `avail_bytes` more. The caller will have
// to write `avail_bytes` at the pointer that has been returned.
//
// Once the caller has finished processing the buffer, the buffer must be
// released by calling "discard(avail_bytes)". Then, the client can `request_memory_for_reading`
// again to obtains more bytes, if required.
//------------------------------------------------------------------------------
inline const void* chunked_binary_queue::request_memory_for_reading(size_t requested_bytes, size_t& avail_bytes) const
{
	debug_check();
	if (requested_bytes > get_bytes_count()) requested_bytes = get_bytes_count();

	// In case it requested zero bytes, we can satisfy the request in any condition,
	// even if there are no chunks allocated at all.
	if (requested_bytes == 0) {avail_bytes=0; return nullptr;}

	size_t memory_available_in_first_chunk = m_chunk_payload_size-m_first_unread_byte_in_first_chunk;

	assert(memory_available_in_first_chunk > 0);
	avail_bytes = std::min(memory_available_in_first_chunk, requested_bytes);
	return m_first_chunk->payload()+m_first_unread_byte_in_first_chunk;
}

//------------------------------------------------------------------------------
// (brief) This method discards "discard_bytes" from the input and frees the unused memory.
// (param) discard_bytes Number of bytes to be discarded; it must be `<=get_bytes_count()`
//
// This method can be used to discard unwanted bytes or bytes that have already
// been processed due to a previous call to `request_memory_for_reading`.
//------------------------------------------------------------------------------
inline void chunked_binary_queue::discard(size_t discard_bytes)
{
	debug_check();
	assert(discard_bytes <= get_bytes_count());

	m_bytes_count -= discard_bytes;
	while (discard_bytes) {
		// Calculate how many bytes we have in the first chunk
		size_t memory_available_in_first_chunk = m_chunk_payload_size-m_first_unread_byte_in_first_chunk;

		// See if we have to get rid of the chunk
		if (memory_available_in_first_chunk <= discard_bytes) {
			chunked_binary_queue_header* m_next_chunk = m_first_chunk->m_next_chunk;
			free_chunk(m_first_chunk);
			m_first_chunk = m_next_chunk;
			if (m_first_chunk == nullptr) {
				m_last_chunk = nullptr;
				m_first_avail_byte_in_last_chunk = 0;
			}
			m_first_unread_byte_in_first_chunk = 0;
			discard_bytes -= memory_available_in_first_chunk;
			debug_check();
		}
		// Otherwise we have to reduce the first chunk by "discard_bytes"
		else {
			m_first_unread_byte_in_first_chunk += discard_bytes;
			break;
		}
	}
	debug_check();
}

//------------------------------------------------------------------------------
// (brief) Writes a memory block to the stream.
//------------------------------------------------------------------------------
inline void chunked_binary_queue::write_binary(const void* source, size_t bytes)
{
	debug_check();
	size_t avail_bytes;
	const uint8_t* ptr = (const uint8_t*)source;
	while (bytes > 0) {
		void* mem = request_memory_for_writing(bytes, avail_bytes);
		assert(avail_bytes <= bytes);
		memcpy(mem, ptr, avail_bytes);
		bytes -= avail_bytes;
		ptr += avail_bytes;
	}
	debug_check();
}

//------------------------------------------------------------------------------
// (brief) Extract the data from this object and copy it to the given memory target
// (param) destination Target buffer that must be able to receive `min(bytes, get_bytes_count())` bytes
// (param) bytes Maxmimum number of bytes to be extracted
// (return) Returns the number of bytes actually read
//------------------------------------------------------------------------------
inline size_t chunked_binary_queue::read_binary(void* destination, size_t bytes)
{
	DASTD_DEBUG(size_t expected_size_at_end = (bytes < get_bytes_count() ? get_bytes_count()-bytes : 0);)
	size_t remaining_bytes = bytes;

	uint8_t* target = (uint8_t*)destination;
	while (remaining_bytes > 0) {
		size_t avail_bytes;
		const void* ptr = request_memory_for_reading(remaining_bytes, avail_bytes);
		if (avail_bytes == 0) break;
		if (avail_bytes > remaining_bytes) avail_bytes = remaining_bytes;

		memcpy(target, ptr, avail_bytes);
		discard(avail_bytes);
		remaining_bytes -= avail_bytes;
		target += avail_bytes;
	}

	assert(expected_size_at_end == get_bytes_count());
	assert(remaining_bytes <= bytes);
	return bytes-remaining_bytes;
}

//------------------------------------------------------------------------------
// (brief) Extract the data from this object and copy it to the given memory target
//
// (param) destination Target string
// (param) bytes Maxmimum number of bytes to be extracted
//------------------------------------------------------------------------------
inline void chunked_binary_queue::read_binary(std::string& destination, size_t bytes)
{
	if (bytes > get_bytes_count()) bytes=get_bytes_count();
	destination.clear();
	destination.resize(bytes);
	read_binary(destination.data(), bytes);
}



//------------------------------------------------------------------------------
// (brief) Iterate from the internal data starting from the first available
// byte up to 'bytes'.
//
// The iterator can change the internal data.
//
// (param) callback Function allowed to modify the byte; it must return `true` to continue or `false` to abandon
// (param) bytes Maximum number of bytes that is allowed to extract; use bytes=SIZE_MAX to process all bytes.
//------------------------------------------------------------------------------
inline void chunked_binary_queue::iterate(std::function<bool(char& ch, size_t offset)> callback, size_t bytes)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	assert((bytes==SIZE_MAX) || (bytes <=get_bytes_count()));
	if (bytes > get_bytes_count()) bytes=get_bytes_count();

	size_t offset_in_chunk=m_first_unread_byte_in_first_chunk;
	chunked_binary_queue_header* curr_chunk = m_first_chunk;
	size_t i;
	for (i=0; i<bytes; i++) {
		if (offset_in_chunk >= m_chunk_payload_size) {
			offset_in_chunk = 0;
			curr_chunk = curr_chunk->m_next_chunk;
		}
		if (!callback(((char*)curr_chunk->payload())[offset_in_chunk], i)) break;
		offset_in_chunk++;
	}
}

//------------------------------------------------------------------------------
// (brief) Iterate from the internal data starting from the first available
// byte up to 'bytes'.
//
// The iterator can read the internal data.
//
// (param) callback Function allowed to red the byte; it must return `true` to continue or `false` to abandon
// (param) length Maximum number of bytes that is allowed to extract; use bytes=SIZE_MAX to process all bytes.
//------------------------------------------------------------------------------
inline void chunked_binary_queue::iterate(std::function<bool(char ch, size_t offset)> callback, size_t bytes) const
{
	std::lock_guard<std::recursive_mutex> lock(((chunked_binary_queue*)this)->m_mutex);
	assert((bytes==SIZE_MAX) || (bytes <=get_bytes_count()));
	if (bytes > get_bytes_count()) bytes=get_bytes_count();

	size_t offset_in_chunk=m_first_unread_byte_in_first_chunk;
	chunked_binary_queue_header* curr_chunk = m_first_chunk;
	size_t i;
	for (i=0; i<bytes; i++) {
		if (offset_in_chunk >= m_chunk_payload_size) {
			offset_in_chunk = 0;
			curr_chunk = curr_chunk->m_next_chunk;
		}
		if (!callback(curr_chunk->payload()[offset_in_chunk], i)) break;
		offset_in_chunk++;
	}
}

//------------------------------------------------------------------------------
// (brief) Blocking method to write data to the sink
//
// This method will return when the data has been sent. This method will
// do its best to queue the data and return immediately.
// In case of catastrophic error, it will throw an exception.
//
// (param) data      Pointer to the data to be sent
// (param) data_size Amount of bytes it will send
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline void chunked_binary_queue::sink_write(const void* data, size_t data_size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	write_binary(data, data_size);
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source
//
// This method will attempt to read up to `data_size` bytes.
// If less (or even zero) bytes are available, it will read them
// and return.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of characters it should try to read
// (return)          Returns the number of characters actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline size_t chunked_binary_queue::tentative_read(char* data, size_t data_size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (get_bytes_count() < data_size) data_size = get_bytes_count();
	return read_binary(data, data_size);
}

//------------------------------------------------------------------------------
// (brief) Return the number of bytes that could be read at the next tentative_read
//
// This method returns the current max value that can be obtained by calling
// `tentative_read` right now. Note that between the call to `tentative_count` and
// `tentative_read`, the value might changed. So there is no guarantee that the
// following calls will conform to this value.
//
// This call is meant to allow preallocation of data with a good guess.
//------------------------------------------------------------------------------
inline size_t chunked_binary_queue::tentative_count() const
{
	std::lock_guard<std::recursive_mutex> lock(((chunked_binary_queue*)this)->m_mutex);
	return get_bytes_count();
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source
//
// This method will attempt to read exactly `data_size` bytes.
// If less bytes are available, it will not read anything and
// it will return false.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of characters it should try to read
// (return)          Returns `true` if `data_size` bytes have been read; returns `false` if nothing has been read (still not enough data)
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline bool chunked_binary_queue::atomic_read(char* data, size_t data_size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (get_bytes_count() < data_size) return false;
	DASTD_DEBUG(size_t bytes_read=) read_binary(data, data_size);
	assert(bytes_read == data_size);
	return true;
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source
//
// This method will attempt to read exactly `data_size` bytes.
// If less bytes are available, it will not read anything and
// it will return false.
//
// (param) data      Available data will be appended to the `data` string
// (param) data_size Max amount of characters it should try to read
// (return)          Returns `true` if `data_size` bytes have been read; returns `false` if nothing has been read (still not enough data)
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline bool chunked_binary_queue::atomic_read(std::string& data, size_t data_size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (get_bytes_count() < data_size) return false;
	size_t offset = data.size();
	data.resize(offset + data_size);
	DASTD_DEBUG(size_t bytes_read=) read_binary(data.data() + offset, data_size);
	assert(bytes_read == data_size);
	return true;
}

//------------------------------------------------------------------------------
// (brief) Execute an atomic iteration on the data
//
// This method will iterate over the first `data_size` bytes in the queue.
// For each byte, it will call the `callback` function, allowing it to change the value if
// desired.
// If the queue does not contain at least `data_size` bytes, no processing will be done
// and it will return `false`.
//
// (param) callback  Function invoked for each byte. The byte value can be read and changed by the function.
//                  Return `true` to continue (up to `data_size` bytes), `false` to terminate.
// (param) data_size Max amount of characters it should try to read
// (return)          Returns `true` if `data_size` characters have been processed; returns `false` if no processing was done because not enough characters were available.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline bool chunked_binary_queue::atomic_iterate(std::function<bool(char& ch, size_t offset)> callback, size_t data_size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (get_bytes_count() < data_size) return false;
	iterate(callback);
	return true;
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source without extracting it
//
// This method will attempt to read up to `data_size` bytes.
// If less (or even zero) bytes are available, it will read them
// and return. The data will not be extracted from the buffer so the
// next time the same data will be returned again.
// Call `tentative_discard` to discard the data that has been actually used.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of characters it should try to read
// (return)          Returns the number of characters actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
inline size_t chunked_binary_queue::tentative_peek(char* data, size_t data_size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	size_t avail_bytes;
	const void* buffer = request_memory_for_reading(data_size, avail_bytes);
	memcpy(data, buffer, avail_bytes);
	return avail_bytes;
}

} // namespace dastd
