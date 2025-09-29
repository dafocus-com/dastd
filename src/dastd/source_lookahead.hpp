/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-APR-2024
**/
#pragma once
#include "source_with_peek.hpp"

namespace dastd {

///	@brief This class implements a source<char> that maintains the ability of
///	looking "n" CHARTYPEs ahead.
///
///	The caller can fetch one character and being able to "preview" the next
///	"N" characters as defined in the LOOKAHEAD value.
template<class CHARTYPE, size_t LOOKAHEAD> class source_lookahead: public source<CHARTYPE> {
	public:
		/// @brief Constructor
		source_lookahead(source_with_peek<CHARTYPE>& source);

		/// @brief Non-blocking method to read data from the source
		///
		/// This method will attempt to read up to `data_size` characters.
		/// If less (or even zero) characters are available, it will read them
		/// and return.
		///
		/// @param data      Pointer to the buffer that will host the data read
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns the number of characters actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_read(CHARTYPE* data, size_t data_size) override;

		/// @brief Non-blocking method to discard data from the source
		///
		/// This method will attempt to discard up to `data_size` characters.
		/// If less (or even zero) characters are available, it will discard them
		/// and return.
		///
		/// @param data_size Max amount of characters it should try to read
		/// @return          Returns the number of characters actually read. It can be zero.
		/// @throw           It can throw a std::exception or one of its derivatives
		virtual size_t tentative_discard(size_t data_size) override;

		/// @brief Peek in the look ahead
		///
		/// @param pos Lookahead position; `pos=0` is the one that will be returned at next read.
		/// @param data The byte read
		/// @return Returns `true` if the byte at lookahead position `pos` is available; `false` otherwise.
		bool peek(size_t pos, CHARTYPE& data);

		/// @brief Peeks the required character; if not available, it returns the `default_ch`
		/// @tparam CHARTYPE2 Type of the character to be returned
		/// @param pos       Position in the lookahead buffer
		/// @param default_ch Character to ber returned in case the position `pos` is not available
		/// @return          Returns the CHARTYPEater or `default_ch`
		template<typename CHARTYPE2> CHARTYPE2 peek_with_default(size_t pos, CHARTYPE2 default_ch) {
			CHARTYPE2 ch;
			if (peek(pos, ch)) return (CHARTYPE2)ch;
			return default_ch;
		}

	private:
		/// @brief Data source
		source_with_peek<CHARTYPE>& m_source;

		/// @brief Lookahead buffer
		CHARTYPE m_look_ahead_buffer[LOOKAHEAD];

		/// @brief Elements in the lookAheadBuffer
		size_t m_look_ahead_buffer_items=0;

		/// @brief Compact the `m_look_ahead_buffer` once the first `characters_used` have been used
		/// @param characters_used Number of characters at the beginning of the buffer that have been used
		void compact_look_ahead_buffer(size_t characters_used);
};

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
template<class CHARTYPE, size_t LOOKAHEAD>
source_lookahead<CHARTYPE, LOOKAHEAD>::source_lookahead(source_with_peek<CHARTYPE>& source): m_source(source)
{
	static_assert(LOOKAHEAD > 0);
}


//------------------------------------------------------------------------------
// (brief) Compact the `m_look_ahead_buffer` once the first `characters_used` have been used
// (param) characters_used Number of characters at the beginning of the buffer that have been used
//------------------------------------------------------------------------------
template<class CHARTYPE, size_t LOOKAHEAD>
void source_lookahead<CHARTYPE, LOOKAHEAD>::compact_look_ahead_buffer(size_t characters_used)
{
	if (characters_used) {
		// If the buffer requres shifting, do it
		if (characters_used < m_look_ahead_buffer_items) {
			memmove(m_look_ahead_buffer, m_look_ahead_buffer+characters_used, m_look_ahead_buffer_items-characters_used);
		}
		m_look_ahead_buffer_items-=characters_used;
	}
}

//------------------------------------------------------------------------------
// (brief) Non-blocking method to read data from the source
//
// This method will attempt to read up to `data_size` characters.
// If less (or even zero) characters are available, it will read them
// and return.
//
// (param) data      Pointer to the buffer that will host the data read
// (param) data_size Max amount of characters it should try to read
// (return)          Returns the number of characters actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
template<class CHARTYPE, size_t LOOKAHEAD>
size_t source_lookahead<CHARTYPE, LOOKAHEAD>::tentative_read(CHARTYPE* data, size_t data_size)
{
	size_t characters_returned = 0;

	// Start by extracting the characters already stored in the lookahead
	size_t characters_from_look_ahead = std::min(data_size, m_look_ahead_buffer_items);
	if (characters_from_look_ahead) {
		memcpy(data, m_look_ahead_buffer, characters_from_look_ahead);
		data_size -= characters_from_look_ahead;
		characters_returned += characters_from_look_ahead;
		compact_look_ahead_buffer(characters_from_look_ahead);
	}

	// Then extract characters from the real source
	if (data_size > 0) {
		characters_returned += m_source.tentative_read(data+characters_returned, data_size);
	}
	return characters_returned;
}

//------------------------------------------------------------------------------
// (brief) Peek in the look ahead
//
// (param) pos Lookahead position; `pos=0` is the one that will be returned at next read.
// (param) data The byte read
// (return) Returns `true` if the byte at lookahead position `pos` is available; `false` otherwise.
//------------------------------------------------------------------------------
template<class CHARTYPE, size_t LOOKAHEAD>
bool source_lookahead<CHARTYPE, LOOKAHEAD>::peek(size_t pos, CHARTYPE& data)
{
	assert(pos < LOOKAHEAD);
	// If required try to read more data
	if (pos >= m_look_ahead_buffer_items) {
		m_look_ahead_buffer_items += m_source.tentative_read(m_look_ahead_buffer+m_look_ahead_buffer_items, LOOKAHEAD-m_look_ahead_buffer_items);
	}

	if (pos >= m_look_ahead_buffer_items) return false;

	data = m_look_ahead_buffer[pos];
	return true;
}


//------------------------------------------------------------------------------
// (brief) Non-blocking method to discard data from the source
//
// This method will attempt to discard up to `data_size` characters.
// If less (or even zero) characters are available, it will discard them
// and return.
//
// (param) data_size Max amount of characters it should try to read
// (return)          Returns the number of characters actually read. It can be zero.
// (throw)           It can throw a std::exception or one of its derivatives
//------------------------------------------------------------------------------
template<class CHARTYPE, size_t LOOKAHEAD>
size_t source_lookahead<CHARTYPE, LOOKAHEAD>::tentative_discard(size_t data_size)
{
	size_t characters_returned = 0;

	// Start by discarding the characters already stored in the lookahead
	size_t characters_from_look_ahead = std::min(data_size, m_look_ahead_buffer_items);
	if (characters_from_look_ahead) {
		compact_look_ahead_buffer(characters_from_look_ahead);
		data_size -= characters_from_look_ahead;
		characters_returned += characters_from_look_ahead;
	}
	if (data_size) characters_returned += m_source.tentative_discard(data_size);
	return characters_returned;
}

} // namespace dastd
