/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-LUG-2026
*
* Implementation of the XXHash32 non-cryptographic hash function
*  
**/
#pragma once
#include "hash.hpp"
#include "fmt.hpp"

namespace dastd {
/// @brief XXH32 hash calculator
///
/// Given any sequence of bytes, generates a XXH32 hash.
/// Code taken and adapted from https://github.com/stbrumme/xxhash by Stephan Brumme.
/// Thanks Stephan for sharing!
/// 
class hash_xxh32: public hash {
	private:
		/// @brief Algorithm constants
		/// 
		/// Constants used by the xxHash32 mixing function
		///@{
		static constexpr uint32_t k_PRIME1 = 2654435761U;
		static constexpr uint32_t k_PRIME2 = 2246822519U;
		static constexpr uint32_t k_PRIME3 = 3266489917U;
		static constexpr uint32_t k_PRIME4 =  668265263U;
		static constexpr uint32_t k_PRIME5 =  374761393U;
		///@}

		/// @brief Temporarily store up to 15 bytes between multiple add() calls
		static constexpr uint64_t k_MAX_BUFFER_SIZE = 15+1;

		/// @brief Seed used for the algorithm
		uint32_t m_seed;

		uint32_t  m_state[4];
		uint8_t   m_buffer[k_MAX_BUFFER_SIZE];
		size_t    m_buffer_size;
		size_t    m_total_length;

		/// @brief Rotate bits, should compile to a single CPU instruction (ROL)
		static uint32_t rotate_left(uint32_t x, uint8_t bits);
		
		/// @brief Process a block of 4x4 bytes, this is the main part of the XXHash32 algorithm
		static void process(const void* data, uint32_t& state0, uint32_t& state1, uint32_t& state2, uint32_t& state3);

	protected:
		/// @brief Virtual method that adds the indicated bytes to the current hash sum using SHA-1
		///
		/// @param bytes Pointer to the array of raw bytes to be processed
		/// @param length Number of bytes to be processed
		///
		/// @return Returns this hash to allow chaining
		virtual void add_binary(const void* bytes, size_t length) override;

	public:
		/// @brief Constructor
		/// @param seed Algorithm seed value, even zero is a valid seed
		explicit hash_xxh32(uint32_t seed = 0);

		/// @brief Get the hash value
		/// @return Returns the XXH32 hash value of the data added so far
		uint32_t get() const;

		/// @brief reset the hash calculator to its initial state.
		virtual void clear() override;

		/// @brief Print the current hash
		virtual void print(std::ostream& o) const override;
};


// Constructor
inline hash_xxh32::hash_xxh32(uint32_t seed): m_seed(seed)
{
	clear();
}

// Rotate bits, should compile to a single CPU instruction (ROL)
inline uint32_t hash_xxh32::rotate_left(uint32_t x, uint8_t bits)
{
  return (x << bits) | (x >> (32 - bits));
}

// Process a block of 4x4 bytes, this is the main part of the XXHash32 algorithm
inline void hash_xxh32::process(const void* data, uint32_t& state0, uint32_t& state1, uint32_t& state2, uint32_t& state3)
{
	const uint32_t* block = (const uint32_t*) data;
	state0 = rotate_left(state0 + block[0] * k_PRIME2, 13) * k_PRIME1;
	state1 = rotate_left(state1 + block[1] * k_PRIME2, 13) * k_PRIME1;
	state2 = rotate_left(state2 + block[2] * k_PRIME2, 13) * k_PRIME1;
	state3 = rotate_left(state3 + block[3] * k_PRIME2, 13) * k_PRIME1;
}

// Get current hash
inline uint32_t hash_xxh32::get() const
{
  uint32_t result = (uint32_t)m_total_length;

  // fold 128 bit m_state into one single 32 bit value
  if (m_total_length >= k_MAX_BUFFER_SIZE) {
    result += rotate_left(m_state[0],  1) +
              rotate_left(m_state[1],  7) +
              rotate_left(m_state[2], 12) +
              rotate_left(m_state[3], 18);
	}
  else {
    // internal m_state wasn't set in add(), therefore original seed is still stored in state2
    result += m_state[2] + k_PRIME5;
	}

  // process remaining bytes in temporary buffer
  const uint8_t* data = m_buffer;
  // point beyond last byte
  const uint8_t* stop = data + m_buffer_size;

  // at least 4 bytes left ? => eat 4 bytes per step
  for (; data + 4 <= stop; data += 4)
    result = rotate_left(result + *(uint32_t*)data * k_PRIME3, 17) * k_PRIME4;

  // take care of remaining 0..3 bytes, eat 1 byte per step
  while (data != stop) {
    result = rotate_left(result + (*data++) * k_PRIME5, 11) * k_PRIME1;
	}

  // Mix bits
  result ^= result >> 15;
  result *= k_PRIME2;
  result ^= result >> 13;
  result *= k_PRIME3;
  result ^= result >> 16;
  return result;
}

// Virtual method that adds the indicated bytes to the current hash sum using SHA-1
inline void hash_xxh32::add_binary(const void* bytes, size_t length)
{
  // no data ?
  if (!bytes || length == 0) {
    return;
	}

  m_total_length += length;
  // byte-wise access
  const uint8_t* data = (const uint8_t*)bytes;

  // unprocessed old data plus new data still fit in temporary buffer ?
  if (m_buffer_size + length < k_MAX_BUFFER_SIZE)
  {
    // just add new data
    while (length-- > 0) {
      m_buffer[m_buffer_size++] = *data++;
		}
    return;
  }

  // point beyond last byte
  const uint8_t* stop      = data + length;
  const uint8_t* stopBlock = stop - k_MAX_BUFFER_SIZE;

  // some data left from previous update ?
  if (m_buffer_size > 0)
  {
    // make sure temporary buffer is full (16 bytes)
    while (m_buffer_size < k_MAX_BUFFER_SIZE) {
      m_buffer[m_buffer_size++] = *data++;
		}

    // process these 16 bytes (4x4)
    process(m_buffer, m_state[0], m_state[1], m_state[2], m_state[3]);
  }

  // copying m_state to local variables helps optimizer A LOT
  uint32_t s0 = m_state[0], schema1 = m_state[1], s2 = m_state[2], s3 = m_state[3];

  // 16 bytes at once
  while (data <= stopBlock) {
    // local variables s0..s3 instead of m_state[0]..m_state[3] are much faster
    process(data, s0, schema1, s2, s3);
    data += 16;
  }
  // copy back
  m_state[0] = s0; m_state[1] = schema1; m_state[2] = s2; m_state[3] = s3;

  // copy remainder to temporary buffer
  m_buffer_size = stop - data;
  for (unsigned int i = 0; i < m_buffer_size; i++) {
    m_buffer[i] = data[i];
	}
}

// reset the hash calculator to its initial state.
inline void hash_xxh32::clear()
{
  m_state[0] = m_seed + k_PRIME1 + k_PRIME2;
  m_state[1] = m_seed + k_PRIME2;
  m_state[2] = m_seed;
  m_state[3] = m_seed - k_PRIME1;
  m_buffer_size  = 0;
  m_total_length = 0;
}

// Print the current hash
inline void hash_xxh32::print(std::ostream& o) const
{
	o << fmt(get(), 16, 8);
}

} // namespace dastd
