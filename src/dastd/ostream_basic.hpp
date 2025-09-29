/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-AUG-2023
**/
#pragma once
#include <ostream>
#include <streambuf>

namespace dastd {

class ostream_basic;

/// @brief Basic ostreambuf implementation that delegates printing to the ostream_basic
class ostreambuf_basic: public std::streambuf {
	public:
		/// Constructor
		ostreambuf_basic(ostream_basic& its_ostream): m_its_ostream(its_ostream) {}

		/// Method redefined from std::streambuf
		virtual std::streamsize xsputn(const char_type* s, std::streamsize n) override;

		/// Method redefined from std::streambuf
		virtual int_type overflow(int_type ch) override;

		/// Method redefined from std::streambuf
		virtual int sync() override;

	private:
		ostream_basic& m_its_ostream;
};

/// @brief Basic ostream implementation with virtual methods for writing one/multiple characters
///
/// The goal of this class is to have an ostream that can be derived and implemented with
/// a custom write without the burden of having a separate ostreambuf. Just derive from this
/// and redefine the `write_char` method.
class ostream_basic: public std::ostream {
	public:
		/// @brief Constructor
		ostream_basic(): std::ostream(&m_buf), m_buf(*this) {}

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) = 0;

		/// @brief Write multiple characters to the target stream
		///
		/// The default is to loop on "write_char". It can be redefined for better
		/// optimization.
		virtual void write_chars(const char_type* s, std::streamsize n) {std::streamsize i; for(i=0; i<n; i++) write_char(s[i]);}

		/// @brief Implementation of the "flush" action
		///
		/// @return Return `false` in case of error, `true` if ok.
		virtual bool sync() {return true;}

	private:
		/// @brief Associated ostreambuf_basic
		ostreambuf_basic m_buf;
};

/// Method redefined from std::streambuf
inline std::streamsize ostreambuf_basic::xsputn(const char_type* s, std::streamsize n)
{
	m_its_ostream.write_chars(s, n);
	return n;
}

/// Method redefined from std::streambuf
inline ostreambuf_basic::int_type ostreambuf_basic::overflow(ostreambuf_basic::int_type ch)
{
	if (ch != EOF) m_its_ostream.write_char((char)ch);
	return ch;
}

/// Method redefined from std::streambuf
inline int ostreambuf_basic::sync()
{
	return (m_its_ostream.sync() ? 0 : -1);
}


} /// namespace dastd
