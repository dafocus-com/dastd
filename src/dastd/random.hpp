/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 21-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include "spinlock.hpp"
#include <random>
#include <mutex>
#include <limits>
#include <chrono>

namespace dastd {

/// @brief
class random {
	public:
		/// @brief Constructor with auto-seeding
		random() {
			auto seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			m_generator.seed(static_cast<std::mt19937::result_type>(seed));
		}

		/// @brief Returns a random number with the indicated uniform_int_distribution
		/// @tparam TYPE   Integral type
		/// @param min     Minimum value
		/// @param max     Maximum value
		/// @return        Returns a random number in the indicated range
		template<concept_integral NUMTYPE>
		NUMTYPE uniform_int_distribution(NUMTYPE min = std::numeric_limits<NUMTYPE>::lowest(), NUMTYPE max = std::numeric_limits<NUMTYPE>::max()) {
			std::lock_guard<spinlock> lock(m_lock);
			std::uniform_int_distribution<NUMTYPE> distribution(min,max);
			return distribution(m_generator);
		}

		/// @brief Generate random bytes
		/// @param buffer Buffer where the bytes will be written
		/// @param length Number of bytes to be generated
		void random_bytes(void* buffer, size_t length);

	protected:
		/// @brief Random generator
		std::mt19937 m_generator;

		/// @brief Spinlock for multithreading access
		spinlock m_lock;
};

inline random g_random;


//------------------------------------------------------------------------------
// (brief) Generate random bytes
// (param) buffer Buffer where the bytes will be written
// (param) length Number of bytes to be generated
//------------------------------------------------------------------------------
inline void random::random_bytes(void* buffer, size_t length)
{
	size_t i;
	for (i=0; i<length; i++) {
		((uint8_t*)buffer)[i] = (uint8_t)uniform_int_distribution<uint32_t>(0, 255);
	}
}

} // namespace dastd
