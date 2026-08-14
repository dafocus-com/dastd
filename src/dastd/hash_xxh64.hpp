/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-LUG-2026
*
* Implementation of the XXHash64 non-cryptographic hash function
*  
**/
#pragma once
#include "hash.hpp"
#include "fmt.hpp"

namespace dastd {
/// @brief XXH64 hash calculator
///
/// Given any sequence of bytes, generates a XXH64 hash.
/// Code taken and adapted from https://github.com/stbrumme/xxhash by Stephan Brumme.
/// Thanks Stephan for sharing!
/// 
class hash_xxh64: public hash {
	private:
		/// @brief Algorithm constants
		/// 
		/// Constants used by the xxHash64 mixing function
		///@{
		static constexpr uint64_t k_PRIME1 = 11400714785074694791ULL;
		static constexpr uint64_t k_PRIME2 = 14029467366897019727ULL;
		static constexpr uint64_t k_PRIME3 =  1609587929392839161ULL;
		static constexpr uint64_t k_PRIME4 =  9650029242287828579ULL;
		static constexpr uint64_t k_PRIME5 =  2870177450012600261ULL;
		///@}

		/// @brief Temporarily store up to 31 bytes between multiple add() calls
		static constexpr uint64_t k_MAX_BUFFER_SIZE = 31+1;

		/// @brief Seed used for the algorithm
		uint64_t m_seed;

		uint64_t  m_state[4];
		uint8_t   m_buffer[k_MAX_BUFFER_SIZE];
		size_t    m_buffer_size;
		size_t    m_total_length;

		/// @brief Rotate bits, should compile to a single CPU instruction (ROL)
		static uint64_t rotate_left(uint64_t x, uint8_t bits);

		/// @brief Process a single 64 bit value
		static uint64_t process_single(uint64_t previous, uint64_t input);
		
		/// @brief Process a block of 4x4 bytes, this is the main part of the XXHash32 algorithm
		static void process(const void* data, uint64_t& state0, uint64_t& state1, uint64_t& state2, uint64_t& state3);

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
		explicit hash_xxh64(uint64_t seed = 0);

		/// @brief Get the hash value
		/// @return Returns the XXH64 hash value of the data added so far
		uint64_t get() const;

		/// @brief reset the hash calculator to its initial state.
		virtual void clear() override;

		/// @brief Print the current hash
		virtual void print(std::ostream& o) const override;
};


// Constructor
inline hash_xxh64::hash_xxh64(uint64_t seed): m_seed(seed)
{
	clear();
}

// Rotate bits, should compile to a single CPU instruction (ROL)
inline uint64_t hash_xxh64::rotate_left(uint64_t x, uint8_t bits)
{
  return (x << bits) | (x >> (64 - bits));
}

// Process a single 64 bit value
inline uint64_t hash_xxh64::process_single(uint64_t previous, uint64_t input)
{
  return rotate_left(previous + input * k_PRIME2, 31) * k_PRIME1;
}

// Process a block of 4x4 bytes, this is the main part of the XXHash32 algorithm
inline void hash_xxh64::process(const void* data, uint64_t& state0, uint64_t& state1, uint64_t& state2, uint64_t& state3)
{
  const uint64_t* block = (const uint64_t*) data;
  state0 = process_single(state0, block[0]);
  state1 = process_single(state1, block[1]);
  state2 = process_single(state2, block[2]);
  state3 = process_single(state3, block[3]);
}

// Get current hash
inline uint64_t hash_xxh64::get() const
{
  // fold 256 bit m_state into one single 64 bit value
  uint64_t result;
  if (m_total_length >= k_MAX_BUFFER_SIZE) {
    result = rotate_left(m_state[0],  1) +
              rotate_left(m_state[1],  7) +
              rotate_left(m_state[2], 12) +
              rotate_left(m_state[3], 18);
    result = (result ^ process_single(0, m_state[0])) * k_PRIME1 + k_PRIME4;
    result = (result ^ process_single(0, m_state[1])) * k_PRIME1 + k_PRIME4;
    result = (result ^ process_single(0, m_state[2])) * k_PRIME1 + k_PRIME4;
    result = (result ^ process_single(0, m_state[3])) * k_PRIME1 + k_PRIME4;
  }
  else {
    // internal m_state wasn't set in add(), therefore original seed is still stored in state2
    result = m_state[2] + k_PRIME5;
  }

  result += m_total_length;

  // process remaining bytes in temporary m_buffer
  const uint8_t* data = m_buffer;
  // point beyond last byte
  const uint8_t* stop = data + m_buffer_size;

  // at least 8 bytes left ? => eat 8 bytes per step
  for (; data + 8 <= stop; data += 8) {
    result = rotate_left(result ^ process_single(0, *(uint64_t*)data), 27) * k_PRIME1 + k_PRIME4;
	}

  // 4 bytes left ? => eat those
  if (data + 4 <= stop) {
    result = rotate_left(result ^ (*(uint32_t*)data) * k_PRIME1, 23) * k_PRIME2 + k_PRIME3;
    data  += 4;
  }

  // take care of remaining 0..3 bytes, eat 1 byte per step
  while (data != stop) {
    result = rotate_left(result ^ (*data++) * k_PRIME5, 11) * k_PRIME1;
	}

  // mix bits
  result ^= result >> 33;
  result *= k_PRIME2;
  result ^= result >> 29;
  result *= k_PRIME3;
  result ^= result >> 32;
  return result;
}

// Virtual method that adds the indicated bytes to the current hash sum using SHA-1
inline void hash_xxh64::add_binary(const void* bytes, size_t length)
{
  // No data ?
  if (!bytes || length == 0) return;

  m_total_length += length;
  // byte-wise access
  const uint8_t* data = (const uint8_t*)bytes;

  // Unprocessed old data plus new data still fit in temporary m_buffer ?
  if (m_buffer_size + length < k_MAX_BUFFER_SIZE) {
    // just add new data
    while (length-- > 0)
      m_buffer[m_buffer_size++] = *data++;
    return;
  }

  // Point beyond last byte
  const uint8_t* stop      = data + length;
  const uint8_t* stopBlock = stop - k_MAX_BUFFER_SIZE;

  // Some data left from previous update ?
  if (m_buffer_size > 0) {
    // Make sure temporary m_buffer is full (16 bytes)
    while (m_buffer_size < k_MAX_BUFFER_SIZE) {
      m_buffer[m_buffer_size++] = *data++;
		}

    // process these 32 bytes (4x8)
    process(m_buffer, m_state[0], m_state[1], m_state[2], m_state[3]);
  }

  // copying m_state to local variables helps optimizer A LOT
  uint64_t s0 = m_state[0], schema1 = m_state[1], s2 = m_state[2], s3 = m_state[3];

  // 32 bytes at once
  while (data <= stopBlock) {
    // local variables s0..s3 instead of m_state[0]..m_state[3] are much faster
    process(data, s0, schema1, s2, s3);
    data += 32;
  }

  // copy back
  m_state[0] = s0; m_state[1] = schema1; m_state[2] = s2; m_state[3] = s3;

  // copy remainder to temporary m_buffer
  m_buffer_size = stop - data;
  for (uint64_t i = 0; i < m_buffer_size; i++) {
    m_buffer[i] = data[i];
	}
}

// reset the hash calculator to its initial state.
inline void hash_xxh64::clear()
{
  m_state[0] = m_seed + k_PRIME1 + k_PRIME2;
  m_state[1] = m_seed + k_PRIME2;
  m_state[2] = m_seed;
  m_state[3] = m_seed - k_PRIME1;
  m_buffer_size  = 0;
  m_total_length = 0;
}

// Print the current hash
inline void hash_xxh64::print(std::ostream& o) const
{
	o << fmt(get(), 16, 16);
}

} // namespace dastd
