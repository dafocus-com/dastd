/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 16-MAY-2026
**/
#pragma once
#include "ostream_transacted.hpp"
#include "time.hpp"
#include "fmt.hpp"
#include <fstream>
#include <filesystem>
#include <limits>
#include <cassert>

namespace dastd {

/// @brief Writes data to a file that contains a daily timestamp in its name
///
/// This file writes its data to a file that contains a YYYYMMDD timestamp.
/// The data is written to the file that has the indicated timestamp according to
/// the current date/time.
/// 
/// Can be used, for example, to write logs on separate files day by day.
/// 
/// IMPORTANT NOTES:
///  *  writing must be executed only within `transaction_begin()` and
///     `transaction_end()`
///  *  this class is not reentrant; callers must make sure that the
///     sequence `transaction_begin()`, writes, `transaction_end()` are
///     executed by the same thread.
class ostream_daily_file: public ostream_transacted {
	public:
		/// @brief Constructor
		/// @param base_file_name Base file name; the name will be altered to have the timestamp added.
		ostream_daily_file(const std::filesystem::path& base_file_name):
			m_base_file_name(base_file_name) {}

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) ;

		/// @brief Write multiple characters to the target stream
		///
		/// The default is to loop on "write_char". It can be redefined for better
		/// optimization.
		virtual void write_chars(const char_type* s, std::streamsize n);

		/// @brief Implementation of the "flush" action
		///
		/// @return Return `false` in case of error, `true` if ok.
		virtual bool sync();

		/// @brief Get current time from epoch
		/// 
		/// This method must return the time in the timezone required.
		/// Default implementation returns UTC.
		/// 
		/// @return Returns the number of seconds from epoch
		virtual epoch64_t now() const;

		/// @brief Write one character to the target stream
		virtual void transaction_begin();

		/// @brief Write one character to the target stream
		virtual void transaction_end();

		/// @brief Get the current file name
		/// @return Returns the name of the file being written right now
		const std::filesystem::path& get_current_file_name() const {return m_current_file_name;}

	protected:
		/// @brief Ensure the file is open.
		///
		/// Opens the file if not open.
		/// 
		/// @throws Throws a std::exception exception in case of error
		virtual void ensure_open();

		/// @brief Create a new file name
		/// @param current_time Current epoch time previously returned by `now()`
		virtual void create_new_file_name(epoch64_t current_time);

	private:
		/// @brief Minimum epoch for the current filename
		epoch64_t m_current_min = std::numeric_limits<epoch64_t>::min();

		/// @brief Maxmimum epoch for the current filename
		epoch64_t m_current_max = std::numeric_limits<epoch64_t>::min();

		/// @brief File name being currently used
		std::filesystem::path m_current_file_name;

		/// @brief Base file name, used to calculate the current file name
		std::filesystem::path m_base_file_name;

		/// @brief Associated file
		std::ofstream m_fstream;
};


// Write one character to the target stream
inline void ostream_daily_file::write_char(char_type c)
{
	assert(m_fstream.is_open());
	m_fstream.put(c);
}

// Write multiple characters to the target stream
inline void ostream_daily_file::write_chars(const char_type* s, std::streamsize n)
{
	assert(m_fstream.is_open());
	m_fstream.write(s, n);
}

// Implementation of the "flush" action
inline bool ostream_daily_file::sync()
{
	assert(m_fstream.is_open());
	m_fstream.flush();
	return true;
}

// Get current time from epoch
inline epoch64_t ostream_daily_file::now() const
{
	return time::epoch_nowUTC64();
}

// Ensure the file is open.
inline void ostream_daily_file::ensure_open()
{
	// See if the filename is still valid
	epoch64_t current_time = now();

	// If we are out of range, calculate the new file name
	if ((current_time < m_current_min) || (current_time >= m_current_min)) {
		if (m_fstream.is_open()) m_fstream.close();
		create_new_file_name(current_time);
	}

	if (!m_fstream.is_open()) {
		m_fstream.exceptions(std::ios::failbit | std::ios::badbit);
		m_fstream.open(m_current_file_name, std::ios::out | std::ios::app);
	}
}

/// @brief Write one character to the target stream
inline void ostream_daily_file::transaction_begin()
{
	ensure_open();
}

/// @brief Write one character to the target stream
inline void ostream_daily_file::transaction_end()
{
}

// @brief Create a new file name
inline void ostream_daily_file::create_new_file_name(epoch64_t current_time)
{
	time now_time;
	now_time.set_from_epochUTC(current_time);
	decomposed_time decomp;
	now_time.decompose(decomp);

	// Update time/date
	m_current_min = time::make_epoch64(decomp.m_year, decomp.m_month, decomp.m_day);
	m_current_max = m_current_min + 86400;

	// Create the file name
	std::ostringstream file_name;
	file_name << fmt(decomp.m_year, 10, 4) << fmt(decomp.m_month, 10, 2) << fmt(decomp.m_day, 10, 2) << "_" << m_base_file_name.filename().string();

	m_current_file_name = m_base_file_name;
	m_current_file_name.replace_filename(file_name.str());
}

} /// namespace dastd
