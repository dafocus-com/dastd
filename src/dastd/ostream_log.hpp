/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include "defs.hpp"
#include "time.hpp"
#include "ostream_broadcast.hpp"
#include "thread_affinity.hpp"
#include <thread>
#include <mutex>
#include <iostream>
#ifdef DASTD_ANDROID
#include <jni.h>
#include <android/log.h>
#endif
#ifdef DASTD_ESP32
#include "esp_log.h"
#endif

// The "ERROR" macro is defined in "wingdi.h" as "#define ERROR 0". We must get rid of it.
#ifdef ERROR
#undef ERROR
#endif

#ifdef DASTD_LOG_HAS_TAG
#define DASTD_LOG_TAG_PARAM const char* tag
#else
#define DASTD_LOG_TAG_PARAM
#endif

namespace dastd {

/// @brief Maximum size for the thread name to be printed on the log
/// It includes the terminating zero.
static constexpr size_t k_thread_name_size = 32;

/// @brief Log level.
///
/// Each log level will add a prefix like "[DEBUG]" or "[ERROR]".
/// More verbose log information can be excluded.
enum class ostream_log_level_t {
	/// @brief Used for debugging information, usually turned off
	log_DEBUG,
	/// @brief Used for non-essential runtime information; can be turned off
	log_INFO,
	/// @brief Used to report succesful operations; should not be turned off
	log_OK,
	/// @brief Used to report warnings; should not be turned off
	log_WARNING,
	/// @brief Used to report errors; should not be turned off
	log_ERROR,
};

/// @brief Thread synchronized log stream
///
/// This stream can be instanced as a singleton and used to log information to
/// other streams like std::cerr or files from multiple thread.
///
/// By default, it starts with "std::cout" registered. Call "clear()" to remove
/// it and add your own streams.
class ostream_log: public ostream_broadcast {
	private:
		static thread_local char g_thread_name[k_thread_name_size];

	public:
		/// @brief Constructor
		ostream_log();

		/// @brief Destructor
		virtual ~ostream_log();

		/// @brief Set the minimum log level
		/// @param min_log_level  Minimum log level
		void set_min_log_level(ostream_log_level_t min_log_level);

		/// @brief Call before writing
		/// @param log_level  Required log level; it will print a prefix to the log entry and exclude information if required log level is elevated.
		/// @param line_evidencer  Text placed in front of each line to allow grepping lines with a given text inside
		///                        This text is reset at and.
		///
		/// Once done, call `end()`.
		void begin(ostream_log_level_t log_level, const char* line_evidencer=nullptr);

		/// @brief Call when done to release the log
		/// @param tag The `tag` is used only by the logging systems supporting a tag for each log entry
		///
		/// This method will automatically add a newline if the last line did not
		/// terminate with newline.
		void end(const char* tag=nullptr);

		/// @brief Write one character to the target stream
		virtual void write_char(char_type c) override;

		/// @brief Implementation of the "flush" action
		///
		/// @return Return `false` in case of error, `true` if ok.
		virtual bool sync() override;

		/// @brief Enable/disable timestamp, one each line
		///
		/// @param timestamp_type See `TIME_xxx` values in dastd_time.hpp. Use TIME_NONE to disable.
		void set_time_stamp(uint32_t timestamp_type) {m_timestamp_type = timestamp_type;}

		/// @brief Use local time instead of UTC
		void set_local_time(bool use_local_time=true) {m_use_local_time = use_local_time;}

		/// @brief Enable or disable the thread id information
		void display_thread_id(bool enable_thread_id=true) {m_enable_thread_id = enable_thread_id;}

		/// @brief Enable or disable the thread name
		void display_thread_name(bool enable_thread_name=true) {m_enable_thread_name = enable_thread_name;}

		/// @brief Enable or disable the core id information
		void display_core_id(bool enable_core_id=true) {m_enable_core_id = enable_core_id;}

		/// @brief Enable or disable the log level prefix like "[INFO]" or "[DEBUG]".
		/// @param enabled Set to `true` to enable; set to `false` to disable
		void set_log_level_prefix(bool enabled) {m_enable_log_level_prefix = enabled;}

		/// @brief Set the next line prefix
		///
		/// This string is printed after each line header but before each line content
		/// for the lines after the first one.
		///
		/// The default string is "| ", that gives an effect like this:
		///
		///     2023-09-27 This is the first line
		///     2023-09-27 | this is the second line
		///     2023-09-27 | this is the third line
		///
		void set_next_line_prefix(const std::string& next_line_prefix) {m_next_line_prefix = next_line_prefix;}

		/// @brief Set the thread name, to help logging
		/// @param thread_name Name of the thread; truncated at `k_thread_name_size`
		static void set_thread_name(const std::string& thread_name);

	protected:
		/// @brief Write the line header with the timestamp and any other active option
		///
		/// This method can be overridden to have custom line headers. It will be invoked
		/// with the stream already locked.
		///
		/// @param line_number  Indicates the line number within the same begin/end root
		///                    locking pair. First line is 0.
		virtual void print_header(uint32_t line_number);

		/// @brief Close the last writing session, adding a newline if required
		/// @param tag The `tag` is used only by the logging systems supporting a tag for each log entry
		virtual void close_session(const char* tag);

		/// @brief Terminate the log entry with a newline
        ///
        /// This method is invoked if the log entry is ended but it has not been terminated with a newline.
        /// It is used to terminate the log line in the logging system that do not record single
        /// separate log entries.
        /// 
        /// Override to change the behaviour if required.
		virtual void terminate_log_entry_with_newline();

		/// @brief Actually write one character on the log
		/// @param c Character to be printed
		/// 
		/// Override this method to implement custom print modes.
		virtual void write_char_on_log(char_type c);

		/// @brief Current log level
		ostream_log_level_t m_log_level = ostream_log_level_t::log_INFO;

		/// @brief Minimum log level
		ostream_log_level_t m_min_log_level = ostream_log_level_t::log_DEBUG;

		/// @brief Return true if the text is to be printed according to the current log level
		bool log_level_is_enabled() const {return m_log_level >= m_min_log_level;}

		/// @brief Timestamp type
		uint32_t m_timestamp_type = TIME_DEFAULT;

		/// @brief If set, prints the thread id
		bool m_enable_thread_id = false;

		/// @brief If set, prints the thread name
		bool m_enable_thread_name = false;

		/// @brief If set, prints the core number
		bool m_enable_core_id = false;

		/// @brief Internal lock
		std::recursive_mutex m_mutex;

		/// @brief Line number counter
		uint32_t m_line_number=0;

		/// @brief Use local time instead of UTC
		bool m_use_local_time = false;

		/// @brief True if the last character was not a newline
		bool m_requires_new_line = false;

		/// @brief True if the next character will be at the beginning of a line
		bool m_next_char_is_first_in_its_line = true;

		/// @brief Size of the line header buffer
		static constexpr size_t k_line_header_buffer_size = 512;

		/// @brief Enable the log level prefix string like "[DEBUG]" or "[ERROR]"
		bool m_enable_log_level_prefix = true;

		/// @brief String printed before the lines after the first one
		std::string m_next_line_prefix = "| ";

		/// @brief String used to evidence some log lines.
		///
		/// The indicated text is printed before each line, after the timestamp and the
		/// `m_next_line_prefix`.
		std::string m_line_evidencer = "";

		/// @brief Line header buffer
		///
		/// Stores the line header text to be printed on all the identical lines
		char_type m_line_header_buffer[k_line_header_buffer_size];

		#ifdef DASTD_IS_DEBUG
		/// @brief [DEBUG] Used to assert if the stream is used when not locked
		std::thread::id m_owning_thread_id;
		#endif

		#if defined DASTD_ANDROID && !defined DASTD_DISABLE_THREAD_ID
		/// @brief UTF-8 string used to print messages on the Android log
		std::string m_android_text;
		#endif

		#if defined DASTD_ESP32
		/// @brief ASCIIZ string used to print messages on the ESP32 log
		std::string m_esp32_text;
		#endif
};


/// @brief Operates as a guard object for writing on a @ref ostream_log object
///
/// The log is locked for the time the `ostream_log_guard` stays alive.
///
/// Example:
///
///     {
///        ostream_log_guard guard(g_log, ostream_log_level_t::log_INFO);
///        g_log << "Write on locked log..."
///     }
///     // guard is out of scope and the log lock has been released.
class ostream_log_guard {
	private:
		/// @brief Internal reference to the log object
		ostream_log& m_log;

	public:
		/// @brief Constructor
		/// @param log The log object to be used
		/// @param log_level The log level of this entry
		ostream_log_guard(ostream_log& log, ostream_log_level_t log_level): m_log(log) {log.begin(log_level);}
		
		/// @brief Destructor
		~ostream_log_guard() {m_log.end();}

		ostream_log_guard(const ostream_log_guard&) = delete;
		ostream_log_guard& operator=(const ostream_log_guard&) = delete;
};


#ifndef DASTD_DISABLE_DEFAULT_LOG

//------------------------------------------------------------------------------
// (brief) Global instance of log
//------------------------------------------------------------------------------
#ifndef DASTD_CUSTOM_GLOBAL_LOG

// Define DASTD_LOG_IS_DYNAMIC to use the dynamic pointer. It is up to the
// user to initialize `g_log_ptr` with a valid value before using the log.
#ifdef DASTD_LOG_IS_DYNAMIC
inline std::shared_ptr<ostream_log> g_log_ptr;
inline ostream_log& global_log() {return *g_log_ptr;}

// Define DASTD_LOG_IS_EXTERNAL to use an external function `global_log()`
#elif defined DASTD_LOG_IS_EXTERNAL
extern ostream_log& global_log();
#else
inline ostream_log g_log2;
inline constexpr ostream_log& global_log() {return g_log2;}
#endif
#else
DASTD_CUSTOM_GLOBAL_LOG
#endif

#define g_log global_log()

#ifndef DASTD_TAG
#define DASTD_TAG __func__
#endif

#define DASTD_LOG_DEBUG(txt) (::dastd::g_log.begin(::dastd::ostream_log_level_t::log_DEBUG), (::dastd::g_log << txt), ::dastd::g_log.end(DASTD_TAG))
#define DASTD_LOG_INFO(txt) (::dastd::g_log.begin(::dastd::ostream_log_level_t::log_INFO), (::dastd::g_log << txt), ::dastd::g_log.end(DASTD_TAG))
#define DASTD_LOG_WARNING(txt) (::dastd::g_log.begin(::dastd::ostream_log_level_t::log_WARNING), (::dastd::g_log << txt), ::dastd::g_log.end(DASTD_TAG))
#define DASTD_LOG_OK(txt) (::dastd::g_log.begin(::dastd::ostream_log_level_t::log_OK), (::dastd::g_log << txt), ::dastd::g_log.end(DASTD_TAG))
#define DASTD_LOG_ERROR(txt) (::dastd::g_log.begin(::dastd::ostream_log_level_t::log_ERROR), (::dastd::g_log << txt), ::dastd::g_log.end(DASTD_TAG))

/// To be used for longer operations on log:
///
/// Example:
/// 
///      DASTD_LOG_BEGIN(DEBUG);
///      DASTD_LOG_STREAM << "Hello world" << std::endl;
///      DASTD_LOG_STREAM << "Write some more...";
///      DASTD_LOG_END;
/// 
#define DASTD_LOG_BEGIN(level) ::dastd::g_log.begin(::dastd::ostream_log_level_t::log_##level)
#define DASTD_LOG_STREAM ::dastd::g_log
#define DASTD_LOG_END ::dastd::g_log.end(DASTD_TAG)

#endif // DASTD_DISABLE_DEFAULT_LOG

} // namespace dastd

//    ___ _   _ _     ___ _   _ _____
//   |_ _| \ | | |   |_ _| \ | | ____|
//    | ||  \| | |    | ||  \| |  _|
//    | || |\  | |___ | || |\  | |___
//   |___|_| \_|_____|___|_| \_|_____|
//



namespace dastd {

inline thread_local char ostream_log::g_thread_name[k_thread_name_size] = {0};

//------------------------------------------------------------------------------
// (brief) Constructor
//------------------------------------------------------------------------------
inline ostream_log::ostream_log()
{
	#if !defined DASTD_ANDROID && !defined DASTD_ESP32
	add_stream_not_owned(std::cout);
	#endif

	#if defined DASTD_ANDROID || defined DASTD_ESP32
	// In case of Android, we disable the timestamp by default since it is already
	// shown in the Android log itself.
	set_time_stamp(TIME_NONE);

	// In case of Android, disable the [INFO], [ERROR] etc. prefix. Android
	// already shows a color and a box for this information.
	set_log_level_prefix(false);
	#endif
}

//------------------------------------------------------------------------------
// (brief) Destructor
//------------------------------------------------------------------------------
inline ostream_log::~ostream_log()
{
	m_mutex.lock();
	clear();
	m_mutex.unlock();
}

//------------------------------------------------------------------------------
// (brief) Implementation of the "flush" action
//
// (return) Return `false` in case of error, `true` if ok.
//------------------------------------------------------------------------------
inline bool ostream_log::sync()
{
	const std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return ostream_broadcast::sync();
}

//------------------------------------------------------------------------------
// (brief) Write the line header with the timestamp and any other active option
//
// This method can be overridden to have custom line headers. It will be invoked
// with the stream already locked.
//
// (param) line_number  Indicates the line number within the same end root
//                     locking pair. First line is 0.
//------------------------------------------------------------------------------
inline void ostream_log::print_header(uint32_t line_number)
{
	m_next_char_is_first_in_its_line = false;
	if (line_number > 0) {
		(*this) << m_line_header_buffer << m_line_evidencer << m_next_line_prefix;
		return;
	}
	m_line_header_buffer[0] = '\0';
	ostream_charbuf header(m_line_header_buffer, k_line_header_buffer_size);

	// If required, print the timestamp
	if (m_timestamp_type != TIME_NONE) {
		time now;
		if (m_use_local_time) now.set_current_time_local();
		else now.set_current_timeUTC();
		now.print(header, m_timestamp_type);
		header << " ";
	}

	#ifndef DASTD_DISABLE_THREAD_ID
	if (m_enable_thread_id) {
		header << "0x" << fmt<uint64_t>((uint64_t)std::hash<std::thread::id>{}(std::this_thread::get_id()), 16, 16) << " ";
	}
	#endif
	if (m_enable_thread_name) {
		header << "[" << g_thread_name << "] ";

	}
	if (m_enable_core_id) {
		header << "CORE:" << get_current_core_id() << " ";
	}

	(*this) << m_line_header_buffer << m_line_evidencer;
}


//------------------------------------------------------------------------------
// (brief) Write one character to the target stream
//------------------------------------------------------------------------------
inline void ostream_log::write_char(char_type c)
{
	if (!log_level_is_enabled()) return;
	if (c == '\r') return;
	#ifndef DASTD_DISABLE_THREAD_ID
	assert(std::this_thread::get_id() == m_owning_thread_id);
	#endif
	if (m_next_char_is_first_in_its_line) {
		print_header(m_line_number);
		m_line_number++;
	}

	#ifdef DASTD_ANDROID
	/// UTF-8 string used to print messages on the Android log
	m_android_text.push_back(c);
	#endif
	#ifdef DASTD_ESP32
	m_esp32_text.push_back(c);
	#endif

	write_char_on_log(c);
	m_requires_new_line = (c != '\n');
	if (c == '\n') m_next_char_is_first_in_its_line = true;
}

//------------------------------------------------------------------------------
// (brief) Actually write one character on the log
//------------------------------------------------------------------------------
inline void ostream_log::write_char_on_log(char_type c)
{
	ostream_broadcast::write_char(c);
}

//------------------------------------------------------------------------------
// (brief) Set the minimum log level
// (param) log_level  Minimum log level
//------------------------------------------------------------------------------
inline void ostream_log::set_min_log_level(ostream_log_level_t min_log_level)
{
	m_mutex.lock();
	m_min_log_level = min_log_level;
	m_mutex.unlock();
}

//------------------------------------------------------------------------------
// (brief) Call before writing
//------------------------------------------------------------------------------
inline void ostream_log::begin(ostream_log_level_t log_level, const char* line_evidencer)
{
	m_mutex.lock();
	m_log_level = log_level;
	m_line_header_buffer[0] = '\0';
	m_line_number = 0;
	assert(!m_requires_new_line);
	if (line_evidencer) m_line_evidencer = line_evidencer;

	#if defined DASTD_IS_DEBUG && !defined DASTD_DISABLE_THREAD_ID
	assert(m_owning_thread_id == std::thread::id());
	m_owning_thread_id = std::this_thread::get_id();
	#endif

	// This macro prints "[DEBUG] " in case the log level is "DEBUG" and so on.
	#define DASTD_LOG_CASE_STR(k) \
		case ostream_log_level_t::log_##k: {\
			static constexpr const char s##k[] = "[" #k "] "; \
			write_chars(s##k, sizeof(s##k)/sizeof(s##k[0])-1); \
			break; \
		}

	if (m_enable_log_level_prefix) {
		switch(log_level) {
			DASTD_LOG_CASE_STR(DEBUG)
			DASTD_LOG_CASE_STR(INFO)
			DASTD_LOG_CASE_STR(OK)
			DASTD_LOG_CASE_STR(WARNING)
			DASTD_LOG_CASE_STR(ERROR)
		}
	}
}

//------------------------------------------------------------------------------
// (brief) Call when done to release the log
//
// This method will automatically add a newline if the last line did not
// terminate with newline.
//------------------------------------------------------------------------------
inline void ostream_log::end(const char* tag)
{
	#ifndef DASTD_DISABLE_THREAD_ID
	assert(std::this_thread::get_id() == m_owning_thread_id);
	#endif
	close_session(tag);
	m_line_number = 0;
	#if defined DASTD_IS_DEBUG && !defined DASTD_DISABLE_THREAD_ID
	m_owning_thread_id = std::thread::id();
	#endif
	flush();
	m_line_evidencer.clear();
	m_mutex.unlock();
}


//------------------------------------------------------------------------------
// (brief) Close the last writing session, adding a newline if required
//------------------------------------------------------------------------------
inline void ostream_log::close_session(const char* tag)
{
	DASTD_NOWARN_UNUSED(tag);
	if (m_requires_new_line) {
		terminate_log_entry_with_newline();
		m_requires_new_line = false;
	}
	m_next_char_is_first_in_its_line = true;

	#ifdef DASTD_ANDROID
	if (!m_android_text.empty()) {
		int prio = ANDROID_LOG_DEFAULT;
		switch(m_log_level) {
			case ostream_log_level_t::log_DEBUG  : prio = ANDROID_LOG_DEBUG  ; break;
			case ostream_log_level_t::log_INFO   : prio = ANDROID_LOG_INFO   ; break;
			case ostream_log_level_t::log_OK     : prio = ANDROID_LOG_INFO   ; break;
			case ostream_log_level_t::log_WARNING: prio = ANDROID_LOG_WARN   ; break;
			case ostream_log_level_t::log_ERROR  : prio = ANDROID_LOG_ERROR  ; break;
		}
		__android_log_write(prio, "C++", m_android_text.c_str());
		m_android_text.clear();
	}
	#endif
	#ifdef DASTD_ESP32
	if (!m_esp32_text.empty()) {
		switch(m_log_level) {
			case ostream_log_level_t::log_DEBUG  : ESP_LOGD(tag, "%s", m_esp32_text.c_str()); break;
			case ostream_log_level_t::log_INFO   : ESP_LOGI(tag, "%s", m_esp32_text.c_str()); break;
			case ostream_log_level_t::log_OK     : ESP_LOGI(tag, "%s", m_esp32_text.c_str()); break;
			case ostream_log_level_t::log_WARNING: ESP_LOGW(tag, "%s", m_esp32_text.c_str()); break;
			case ostream_log_level_t::log_ERROR  : ESP_LOGE(tag, "%s", m_esp32_text.c_str()); break;
		}
		m_esp32_text.clear();
	}
	#endif
}

//------------------------------------------------------------------------------
// (brief) Terminate the log entry with a newline
//------------------------------------------------------------------------------
inline void ostream_log::terminate_log_entry_with_newline()
{
    #ifndef DASTD_ESP32
    write_char('\n');
    #endif
}

//------------------------------------------------------------------------------
// (brief) Set the thread name, to help logging
// (param) thread_name Name of the thread; truncated at `k_thread_name_size`
//------------------------------------------------------------------------------
inline void ostream_log::set_thread_name(const std::string& thread_name)
{
	strncpy(g_thread_name, thread_name.c_str(), k_thread_name_size-1);
	g_thread_name[k_thread_name_size-1] = 0;
}



} // namespace dastd

