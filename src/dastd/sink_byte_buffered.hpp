/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "sink_byte.hpp"

namespace dastd {

/// @brief Simple binary buffered sink interface
///
/// This class implements a sink object that is able to accept bytes
/// buffer them and slowly place them somewhere.
/// It adds methods to know the state of the buffer.
class sink_byte_buffered: public sink_byte {
	public:
		/// @brief Check if the queue is empty
		/// @return Returns `true` if the outgoing queue is empty
		virtual bool empty() const = 0;
};

} // namespace dastd
