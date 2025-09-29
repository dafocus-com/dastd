/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
*
*
	Given any sequence of bytes, it generates the 160-bit (20-byte) hash value
	known as a "message digest" according to the SHA-1 (Secure Hash Algorithm 1)
	format.

	Code is partially taken from https://raw.githubusercontent.com/clibs/sha1,
	by Steve Reid <steve@edmweb.com> 100% Public Domain

**/
#pragma once
#include "hash.hpp"
#include "fmt.hpp"
#include "fmt_bin.hpp"
#include <string>

namespace dastd {
/// @brief SHA-1 hash calculator
///
/// Given any sequence of bytes, it generates the 160-bit (20-byte) hash value
/// known as a "message digest" according to the SHA-1 (Secure Hash Algorithm 1)
/// format.
///
/// Code is partially taken from https://raw.githubusercontent.com/clibs/sha1,
/// by Steve Reid <steve@edmweb.com> 100% Public Domain (thanks for sharing it).
///
class hash_sha1: public hash {
	private:
		/// @brief Structure describing a SHA-1 state
		struct sha1_state {
			/// @brief Constructor
			sha1_state() {clear();}

			/// @brief Copy constructor
			sha1_state(const sha1_state& o) {operator=(o);}

			/// @brief Assignment
			sha1_state& operator=(const sha1_state& o) {memcpy(m_state, o.m_state, sizeof(m_state)); memcpy(m_count, o.m_count, sizeof(m_count)); memcpy(m_ring_buffer, o.m_ring_buffer, sizeof(m_ring_buffer)); return *this;}

		  /// @brief Internal state
			uint32_t m_state[5];

		  /// @brief Internal count
			size_t m_count[2];

		  /// @brief Internal buffer
			unsigned char m_ring_buffer[64];

			/// @brief Virtual method that adds the indicated bytes to the current hash sum using SHA-1
			///
			/// @param bytes Pointer to the array of raw bytes to be processed
			/// @param length Number of bytes to be processed
			void add_binary(const void* bytes, size_t length);

		  /// @brief Hash a single 512-bit block stored in the 'm_ring_buffer' variable
		  void do_transform512_bit(const void* param_ring_buffer);

			/// @brief Clear to a initial state
			void clear();
		};

		/// @brief Current state
		sha1_state m_sha1_state;

	protected:
		/// @brief Virtual method that adds the indicated bytes to the current hash sum using SHA-1
		///
		/// @param bytes Pointer to the array of raw bytes to be processed
		/// @param length Number of bytes to be processed
		///
		/// @return Returns this hash to allow chaining
		virtual hash& add_binary(const void* bytes, size_t length) override {m_sha1_state.add_binary(bytes, length); return *this;}

	public:
		/// @brief Size of the SHA-1 digest
		static constexpr size_t DIGEST_SIZE = 20;

		/// @brief reset the hash calculator to its initial state.
		virtual void clear() override {m_sha1_state.clear();}

		/// @brief Print the current hash
		virtual void print(std::ostream& o) const override;

		/// @brief Finalize, extract the current SHA-1 digest and clear the object
		///
		/// @param digest Buffer that will receive the SHA-1 digest
		void get(uint8_t digest[DIGEST_SIZE]) const;

};

//------------------------------------------------------------------------------
// (brief) reset the hash calculator to its initial state.
//------------------------------------------------------------------------------
inline void hash_sha1::sha1_state::clear()
{
	// SHA1 initialization constants
	m_state[0] = 0x67452301;
	m_state[1] = 0xEFCDAB89;
	m_state[2] = 0x98BADCFE;
	m_state[3] = 0x10325476;
	m_state[4] = 0xC3D2E1F0;
	m_count[0] = 0;
	m_count[1] = 0;
	memset(m_ring_buffer, 0, sizeof(m_ring_buffer));
}

#define SHA1_rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* SHA1_blk0() and SHA1_blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef DASTD_LITTLE_ENDIAN
#define SHA1_blk0(i) (block->l[i] = (SHA1_rol(block->l[i],24)&0xFF00FF00)|(SHA1_rol(block->l[i],8)&0x00FF00FF))
#elif defined DASTD_BIG_ENDIAN
#define SHA1_blk0(i) block->l[i]
#else
#error "Endianness not defined!"
#endif

#define SHA1_blk(i) (block->l[i&15] = SHA1_rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^ block->l[(i+2)&15]^block->l[i&15],1))

/* (SHA1_R0+SHA1_R1), SHA1_R2, SHA1_R3, SHA1_R4 are the different operations used in SHA1 */
#define SHA1_R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+SHA1_blk0(i)+0x5A827999+SHA1_rol(v,5);w=SHA1_rol(w,30);
#define SHA1_R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+SHA1_blk(i)+0x5A827999+SHA1_rol(v,5);w=SHA1_rol(w,30);
#define SHA1_R2(v,w,x,y,z,i) z+=(w^x^y)+SHA1_blk(i)+0x6ED9EBA1+SHA1_rol(v,5);w=SHA1_rol(w,30);
#define SHA1_R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+SHA1_blk(i)+0x8F1BBCDC+SHA1_rol(v,5);w=SHA1_rol(w,30);
#define SHA1_R4(v,w,x,y,z,i) z+=(w^x^y)+SHA1_blk(i)+0xCA62C1D6+SHA1_rol(v,5);w=SHA1_rol(w,30);


//------------------------------------------------------------------------------
// (brief) Hash a single 512-bit block stored in the 'm_ring_buffer' variable
//------------------------------------------------------------------------------
inline void hash_sha1::sha1_state::do_transform512_bit(const void* param_ring_buffer)
{
	uint32_t a, b, c, d, e;

	typedef union
	{
		unsigned char c[64];
		uint32_t l[16];
	} CHAR64LONG16;

	CHAR64LONG16 block[1];      /* use array to appear as a pointer */

	memcpy(block, param_ring_buffer, 64);

	/* Copy context->m_state[] to working vars */
	a = m_state[0];
	b = m_state[1];
	c = m_state[2];
	d = m_state[3];
	e = m_state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	SHA1_R0(a, b, c, d, e, 0);
	SHA1_R0(e, a, b, c, d, 1);
	SHA1_R0(d, e, a, b, c, 2);
	SHA1_R0(c, d, e, a, b, 3);
	SHA1_R0(b, c, d, e, a, 4);
	SHA1_R0(a, b, c, d, e, 5);
	SHA1_R0(e, a, b, c, d, 6);
	SHA1_R0(d, e, a, b, c, 7);
	SHA1_R0(c, d, e, a, b, 8);
	SHA1_R0(b, c, d, e, a, 9);
	SHA1_R0(a, b, c, d, e, 10);
	SHA1_R0(e, a, b, c, d, 11);
	SHA1_R0(d, e, a, b, c, 12);
	SHA1_R0(c, d, e, a, b, 13);
	SHA1_R0(b, c, d, e, a, 14);
	SHA1_R0(a, b, c, d, e, 15);
	SHA1_R1(e, a, b, c, d, 16);
	SHA1_R1(d, e, a, b, c, 17);
	SHA1_R1(c, d, e, a, b, 18);
	SHA1_R1(b, c, d, e, a, 19);
	SHA1_R2(a, b, c, d, e, 20);
	SHA1_R2(e, a, b, c, d, 21);
	SHA1_R2(d, e, a, b, c, 22);
	SHA1_R2(c, d, e, a, b, 23);
	SHA1_R2(b, c, d, e, a, 24);
	SHA1_R2(a, b, c, d, e, 25);
	SHA1_R2(e, a, b, c, d, 26);
	SHA1_R2(d, e, a, b, c, 27);
	SHA1_R2(c, d, e, a, b, 28);
	SHA1_R2(b, c, d, e, a, 29);
	SHA1_R2(a, b, c, d, e, 30);
	SHA1_R2(e, a, b, c, d, 31);
	SHA1_R2(d, e, a, b, c, 32);
	SHA1_R2(c, d, e, a, b, 33);
	SHA1_R2(b, c, d, e, a, 34);
	SHA1_R2(a, b, c, d, e, 35);
	SHA1_R2(e, a, b, c, d, 36);
	SHA1_R2(d, e, a, b, c, 37);
	SHA1_R2(c, d, e, a, b, 38);
	SHA1_R2(b, c, d, e, a, 39);
	SHA1_R3(a, b, c, d, e, 40);
	SHA1_R3(e, a, b, c, d, 41);
	SHA1_R3(d, e, a, b, c, 42);
	SHA1_R3(c, d, e, a, b, 43);
	SHA1_R3(b, c, d, e, a, 44);
	SHA1_R3(a, b, c, d, e, 45);
	SHA1_R3(e, a, b, c, d, 46);
	SHA1_R3(d, e, a, b, c, 47);
	SHA1_R3(c, d, e, a, b, 48);
	SHA1_R3(b, c, d, e, a, 49);
	SHA1_R3(a, b, c, d, e, 50);
	SHA1_R3(e, a, b, c, d, 51);
	SHA1_R3(d, e, a, b, c, 52);
	SHA1_R3(c, d, e, a, b, 53);
	SHA1_R3(b, c, d, e, a, 54);
	SHA1_R3(a, b, c, d, e, 55);
	SHA1_R3(e, a, b, c, d, 56);
	SHA1_R3(d, e, a, b, c, 57);
	SHA1_R3(c, d, e, a, b, 58);
	SHA1_R3(b, c, d, e, a, 59);
	SHA1_R4(a, b, c, d, e, 60);
	SHA1_R4(e, a, b, c, d, 61);
	SHA1_R4(d, e, a, b, c, 62);
	SHA1_R4(c, d, e, a, b, 63);
	SHA1_R4(b, c, d, e, a, 64);
	SHA1_R4(a, b, c, d, e, 65);
	SHA1_R4(e, a, b, c, d, 66);
	SHA1_R4(d, e, a, b, c, 67);
	SHA1_R4(c, d, e, a, b, 68);
	SHA1_R4(b, c, d, e, a, 69);
	SHA1_R4(a, b, c, d, e, 70);
	SHA1_R4(e, a, b, c, d, 71);
	SHA1_R4(d, e, a, b, c, 72);
	SHA1_R4(c, d, e, a, b, 73);
	SHA1_R4(b, c, d, e, a, 74);
	SHA1_R4(a, b, c, d, e, 75);
	SHA1_R4(e, a, b, c, d, 76);
	SHA1_R4(d, e, a, b, c, 77);
	SHA1_R4(c, d, e, a, b, 78);
	SHA1_R4(b, c, d, e, a, 79);

	/* Add the working vars back into context.m_state[] */
	m_state[0] += a;
	m_state[1] += b;
	m_state[2] += c;
	m_state[3] += d;
	m_state[4] += e;
}
#undef SHA1_blk0
#undef SHA1_blk
#undef SHA1_R0
#undef SHA1_R1
#undef SHA1_R2
#undef SHA1_R3
#undef SHA1_R4
#undef SHA1_rol


//------------------------------------------------------------------------------
// (brief) Finalize, extract the current SHA-1 digest and clear the object
//
// (param) digest Buffer that will receive the SHA-1 digest
//------------------------------------------------------------------------------
inline void hash_sha1::get(uint8_t digest[DIGEST_SIZE]) const
{
	sha1_state temp_sha1_state(m_sha1_state);
	unsigned i;
	unsigned char finalcount[8];
	unsigned char c;
	uint8_t* digestU8 = (uint8_t*)digest;

	for (i = 0; i < 8; i++) {
		finalcount[i] = (unsigned char) ((temp_sha1_state.m_count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);      /* Endian independent */
	}
	c = 0200;
	temp_sha1_state.add_binary(&c, 1);
	while ((temp_sha1_state.m_count[0] & 504) != 448) {
		c = 0000;
		temp_sha1_state.add_binary(&c, 1);
	}
	temp_sha1_state.add_binary(finalcount, 8);
	for (i = 0; i < 20; i++) {
		digestU8[i] = (unsigned char)((temp_sha1_state.m_state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
	}
}

//------------------------------------------------------------------------------
// (brief) Virtual method that adds the indicated bytes to the current hash sum using SHA-1
//
// (param) bytes Pointer to the array of raw bytes to be processed
// (param) length Number of bytes to be processed
//------------------------------------------------------------------------------
inline void hash_sha1::sha1_state::add_binary(const void* bytes, size_t length)
{
	size_t i;
	size_t j;
	const uint8_t* dataU8 = (const uint8_t*)bytes;

	j = m_count[0];
	if ((m_count[0] += length << 3) < j) m_count[1]++;
	m_count[1] += (length >> 29);
	j = (j >> 3) & 63;
	if ((j + length) > 63) {
		i = 64 - j;
		memcpy(m_ring_buffer+j, dataU8, i);
		do_transform512_bit(m_ring_buffer);
		for (; i + 63 < length; i += 64) {
			do_transform512_bit(dataU8+i);
		}
		j = 0;
	}
	else {
		i = 0;
	}
	memcpy(m_ring_buffer+j, dataU8+i, length - i);
}

//------------------------------------------------------------------------------
// (brief) Print the current hash
//------------------------------------------------------------------------------
inline void hash_sha1::print(std::ostream& o) const
{
	uint8_t digest[DIGEST_SIZE];
	get(digest);
	o << "SHA-1:" << fmt_bin(digest, DIGEST_SIZE, fmt_bin_format::TWO_HEX_DIGITS_LC);
}


}
