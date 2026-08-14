/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 16-MAY-2026
**/
#pragma once
#include "ostream_basic.hpp"

namespace dastd {

/// @brief Base class for streams that support "transactions"
///
/// A "transaction" is an exclusive session of printing that begins and ends.
/// 
/// The caller invokes `transaction_begin()`, then writes as many times it needs,
/// then it calls `transaction_end()`. 
/// 
/// The stream can use this information to manage the stream. For example, it can
/// close the file after a given time no transactions have occurred and reopen it
/// at the next `transaction_begin()`.
/// 
/// NOTE: this implementation does not push any transactional or exclusivity
/// management on the stream: `transaction_begin()` and `transaction_end()` are simple 
/// notifications coming from the callers to allow this stream to manage its behaviour
/// more efficiently.
class ostream_transacted: public ostream_basic{
	public:
		/// @brief Write one character to the target stream
		virtual void transaction_begin() {}

		/// @brief Write one character to the target stream
		virtual void transaction_end() {}
};

} /// namespace dastd
