/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "defs.hpp"
#include "source_with_peek.hpp"
#include <functional>

namespace dastd {

/// @brief Simple binary buffered source class
///
/// Implementation of a buffered binary source.
/// This class is meant to implement binary sources that have a buffer that is
/// fed during time. This allows the caller to wait until all the requested
/// data is available before reading it.
/// It also gives methods to know how much data is buffered and to process
/// it without extracting it from the buffer.
template<class CHARTYPE>
class source_buffered: public source_with_peek<CHARTYPE> {
	public:
		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read exactly `data_size` characters.
		/// If less characters are available, it will not read anything and
		/// it will return false.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns `true` if `data_size` characters have been read; returns `false` if nothing has been read (still not enough data)
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual bool atomic_read(CHARTYPE* data, size_t data_size) = 0;

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read exactly `data_size` characters.
		/// If less characters are available, it will not read anything and
		/// it will return false.
		///
		/// @param data      Available data will be appended to the `data` string
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns `true` if `data_size` characters have been read; returns `false` if nothing has been read (still not enough data)
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual bool atomic_read(std::basic_string<CHARTYPE>& data, size_t data_size) = 0;

		/// @brief Execute an atomic iteration on the data
		///
		/// This method will iterate over the first `data_size` characters in the queue.
		/// For each byte, it will call the `callback` function, allowing it to change the value if
		/// desired.
		/// If the queue does not contain at least `data_size` characters, no processing will be done
		/// and it will return `false`.
		///
		/// @param callback  Function invoked for each byte. The byte value can be read and changed by the function.
		///                  Return `true` to continue (up to `data_size` characters), `false` to terminate.
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns `true` if `data_size` characters have been processed; returns `false` if no processing was done because not enough characters were available.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual bool atomic_iterate(std::function<bool(CHARTYPE& ch, size_t offset)> callback, size_t data_size) = 0;
};

} // namespace dastd
