/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 21-AUG-2023
**/

///  DATE FORMAT
///
///  Date is stored in a unsigned 64-bit value, divided in this way:
///  - lower 8 bits contain the time-zone in steps of 15 minutes (see below);
///  - higher 56 bits contain the number of microseconds from Jan. 1st,
///    PCTIME_YEAR_ZERO up to Dec. 31th, PCTIME_YEAR_LAST.
///  - actually, year 'PCTIME_YEAR_ZERO' can't be used because it is considered
///    as the 'invalid' value; in this way, a time set to 0 is automatically
///    invalid.
///
///  TIMEZONE
///  Timezone is stored in this way:
///    0x00 UTC
///    0x01 UTC+00:15
///    ...
///    0x04 UTC+01:00
///    ...
///    0xFF UTC-00:15
///    ...
///    0xFC UTC-01:00
///
///  The conversion is done in this way: Italy, for example, has UTC+01:00.
///  When UTC time is 09:00:00, Italy time is 10:00:00.

#pragma once
#include "defs.hpp"
#include <chrono>
#include <ratio>
#include <array>

// C++ 2020 time looks a bit immature. The stdlibc++ library, at least some versions,
// do not have "std::chrono::current_zone()" defined.
#ifndef USE_CPP_20_TIME
	#define USE_CPP_20_TIME 0
#endif

namespace dastd {

/// @brief Do no print date/time
///
/// Value that can be used by other routines to disable date/time printing.
/// Guaranteed to exist and have the indicated meaning.
static constexpr uint32_t TIME_NONE       = 0x00;

//--------------------------------------------------
// TIME FORMATS
//--------------------------------------------------
/// @brief Mask to extract time format from the time print options
static constexpr uint32_t TIME_FMT_MASK   = 0x0F;

/// @brief Select ISO 8601 timestamp
///
/// The timestamp is in the ISO 8601 form "YYYY-MM-DDThh:mm:ss<TZ>".
/// The <TZ> part is the timezone: "Z" for UTC, "+/-hh:mm" or "+/-hh" for
/// other time zones.
static constexpr uint32_t TIME_FMT_ISO8601 = 0x01;

/// @brief Select fixed format timestamp
///
/// The timestamp is in the form "YYYY-MM-DD hh:mm:ss <TZ>".
/// The <TZ> part is the timezone: "UTC+/-hh:mm"
static constexpr uint32_t TIME_FMT_FIXED   = 0x02;

/// @brief Select packed format timestamp
///
/// The timestamp is in the form "YYYYMMDD hhmmss <TZ>".
/// The <TZ> part is the timezone: "+/-hhmm"
static constexpr uint32_t TIME_FMT_PACKED  = 0x04;


//--------------------------------------------------
// TIME FLAGS
//--------------------------------------------------
/// @brief Enable milliseconds
static constexpr uint32_t TIME_MILLIS     = 0x0010;

/// @brief Enable microseconds (overrides TIME_MILLIS)
static constexpr uint32_t TIME_MICROS     = 0x0020;

/// @brief Enable the timezone
static constexpr uint32_t TIME_TZ         = 0x0040;

/// @brief Enable the week day
static constexpr uint32_t TIME_WEEKDAY    = 0x0080;

/// @brief Use underscores instead of all other kinds of separators
static constexpr uint32_t TIME_UNDERSCORE = 0x0100;

/// @brief Default settings
static constexpr uint32_t TIME_DEFAULT    = (TIME_FMT_ISO8601|TIME_MILLIS|TIME_TZ);


/// @brief Timezone object
struct timezone {
	timezone(int32_t tz=0): m_tz(tz) {}
	int32_t m_tz = 0;

	/// @brief Set the timezone to UTC
	void setUTC() {m_tz=0;}

	/// @brief Print the timezone according to the selected options
	/// @param out Output stream
	/// @param time_print_opt A combination of `TIME_xxx` flags
	void print(std::ostream& out, uint32_t time_print_opt = TIME_DEFAULT) const;
};

/// @brief Print a time zone
DASTD_DEF_OSTREAM(timezone);


/// @brief Structure used to decompose date and time
struct decomposed_time {
	/// @brief Empty constructor
	decomposed_time() {clear();}

	/// @brief Parametric constructor
	decomposed_time(uint32_t year, uint32_t month, uint32_t day) {clear(); m_year=year; m_month=month; m_day=day;}

	/// @brief Parametric constructor
	decomposed_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t mins, uint32_t secs, timezone tz) {clear(); m_year=year; m_month=month; m_day=day; m_hours=hours; m_mins=mins; m_secs=secs; m_tz=tz;}

	// RESET
	void clear() {
		m_year = 0;
		m_month = 0;
		m_day = 0;
		m_hours = 0;
		m_mins = 0;
		m_secs = 0;
		m_microsecs = 0;
		m_tz = 0;
		m_day_of_year = 0;
		m_day_of_week = 0;
	}

	/// @brief Four digit year
	uint32_t m_year;
	/// @brief Month, 1..12
	uint32_t m_month;
	/// @brief Day, 1..31
	uint32_t m_day;
	/// @brief Hours, 0..23
	uint32_t m_hours;
	/// @brief Minutes, 0..59
	uint32_t m_mins;
	/// @brief Seconds, 0..59
	uint32_t m_secs;
	/// @brief Microseconds, 0..999999
	uint32_t m_microsecs;
	/// @brief Timezone, in steps of 15 minutes (1=15', 4=+1h, -1=-15' etc.)
	timezone m_tz;

	/// @brief No. of day in the year. 0=first day of the year, 364=last for non leaps
	///
	/// This value is only calculated by `time::compose` but ignored by
	/// `time::decompose`.
	uint32_t m_day_of_year;

	/// @brief No. of day in the week, where 0=Sunday, 1=Monday, ..., 6=Saturday.
	///
	/// This value is only calculated by `time::compose` but ignored by
	/// `time::decompose`.
	uint32_t m_day_of_week;

	/// @brief Print the timestamp according to the selected options
	/// @param out Output stream
	/// @param time_print_opt A combination of `TIME_xxx` flags
	void print(std::ostream& out, uint32_t time_print_opt = TIME_DEFAULT) const;

	/// @brief Length of the string to be allocated to host a timestamp.
	///
	/// Note that the timestamp can be shorter.
	static constexpr size_t TIMESTAMP_LENGTH = 64;

	/// @brief Write a timestamp using only numbers and underscores.
	///
	/// @param out    The output stream that receives the string. It must
	///               must be able to accept at least TIMESTAMP_LENGTH bytes.
	void write_clean_timestamp(std::ostream& out, bool include_microsec=false) const;
};

DASTD_DEF_OSTREAM(decomposed_time)

/*------------------------------------------------------------------------------
	It stores a date and time.
------------------------------------------------------------------------------*/
class time {
	public:
		/// @brief Not set
		static constexpr uint64_t TIME_NOT_SET = 0;

		/// @brief Constructor
		time () {clear();}

		/// @brief Constructor
		time(const decomposed_time* decomposed) {clear(); compose(decomposed);}
		time(const decomposed_time& decomposed) {clear(); compose(decomposed);}

		/// @brief Constructor
		time(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0, timezone tz=timezone());

		/// @brief Print the timestamp according to the selected options
		/// @param out Output stream
		/// @param time_print_opt A combination of `TIME_xxx` flags
		void print(std::ostream& out, uint32_t time_print_opt = TIME_DEFAULT) const;

		/// @brief Clear the time to an invalid value
		void clear() {m_date_time=TIME_NOT_SET;}

		/// @brief Returns true if it is valid
		bool is_valid() const;

		/// @brief Returns now in UTC
		static time nowUTC() {time now; now.set_current_timeUTC(); return now;}

		/// @brief Returns now in local time
		static time now_local() {time now; now.set_current_time_local(); return now;}

		/// @brief Compare
		bool operator == (time Other) const {return compare(Other) == 0;}
		bool operator != (time Other) const {return compare(Other) != 0;}
		bool operator <  (time Other) const {return compare(Other) <  0;}
		bool operator >  (time Other) const {return compare(Other) >  0;}
		bool operator <= (time Other) const {return compare(Other) <= 0;}
		bool operator >= (time Other) const {return compare(Other) >= 0;}

		/// @brief Given a date in internal format, it returns day, month, etc.
		void decompose(decomposed_time* decomposed) const;
		void decompose(decomposed_time& decomposed) const {decompose(&decomposed);}

		/// @brief Given a date in day, month, etc. format, it updates the internal value
		/// @return Returns true if ok, false in case of failure.
		bool compose(const decomposed_time* decomposed);
		bool compose(const decomposed_time& decomposed) {return compose(&decomposed);}

		/// @brief Set from year, month, day, hours, minutes, seconds and timezone
		/// @return Returns true if ok, false in case of error.
		bool set(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0, timezone tz=timezone());

		/// @brief Set from year, month, day, hours, minutes, seconds and timezone
		///
		/// Defaults on the local timezone returned by get_system_time_zone()
		///
		/// @return Returns true if ok, false in case of error.
		bool set_localTZ(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0) {return set(year, month, day, hours, mins, secs, get_system_time_zone());}

		/// @brief Set from year, month, day, hours, minutes, seconds and timezone
		static epoch32_t make_epoch32(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0, timezone tz=timezone());

		/// @brief Set from year, month, day, hours, minutes, seconds and timezone
		static epoch64_t make_epoch64(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0, timezone tz=timezone());

		/// @brief Set from year, month, day, hours, minutes, seconds and timezone
		///
		/// Defaults on the local timezone returned by get_system_time_zone()
		static epoch32_t make_epoch32_localTZ(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0) {return make_epoch32(year, month, day, hours, mins, secs, get_system_time_zone());}

		/// @brief Set from year, month, day, hours, minutes, seconds and timezone
		///
		/// Defaults on the local timezone returned by get_system_time_zone()
		static epoch64_t make_epoch64_localTZ(uint32_t year, uint32_t month, uint32_t day, uint32_t hours=0, uint32_t mins=0, uint32_t secs=0) {return make_epoch64(year, month, day, hours, mins, secs, get_system_time_zone());}

		/// @brief Returns the current system timezone, that includes DST (Daylight Saving Time)
		static timezone get_system_time_zone();

		/// @brief Returns the difference of the current timezone in seconds from UTC
		///
		/// This value can be added to an UTC epoch to obtain the local time or
		/// subtracted from a local time to get UTC.
		static int32_t seconds_to_be_added_toUTC_to_obtain_local_time();

		/// @brief Set the current time in UTC form.
		void set_current_timeUTC ();

		/// @brief Set the current time in Local time form.
		void set_current_time_local();

		/// @brief et the timezone also updating the internal time.
		void set_timezone(timezone tz);

		/// @brief Return the timezone of the time here stored.
		timezone get_timezone() const;

		/// @brief Compare two times. Returns -1 if this<other, ==0 if this==other, 1 if this>other
		int compare(const time& other) const;

		/// @brief Set from EPOCH UTC
		void set_from_epochUTC(epoch64_t epoch);

		/// @brief Set from EPOCH UTC microseconds
		void set_from_epochUTC_micros(uint64_t micros);

		/// @brief Return the date as EPOCH (seconds from Jan 1st, 1970)
		///
		/// The time is returned it whatever timezone it is.
		/// So if it is 12:34 UTC+1, it returns 12:34.
		/// The value can be converted to 32-bit if dates are within the 32-bit
		/// limit of epoch 18th jan, 2038.
		///
		/// @return If the date is not valid, returns 0.
		epoch64_t get_epoch64() const;

		/// @brief Return the epoch as Epoch32
		epoch32_t get_epoch32() const {return (epoch32_t)get_epoch64();}

		/// @brief Return "now" in local time as epoch
		static epoch64_t epoch_now_local64() {time now; now.set_current_time_local(); return now.get_epoch64();}
		static epoch32_t epoch_now_local32() {return (epoch32_t)epoch_now_local64();}

		/// @brief Return "now" in local time as UTC
		static epoch64_t epoch_nowUTC64() {time now; now.set_current_timeUTC(); return now.get_epoch64();}
		static epoch32_t epoch_nowUTC32() {return (epoch32_t)epoch_nowUTC64();}

		/// @brief Returns current time UTC in microseconds
		static epoch64us_t epoch_nowUTC64_micros() {return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();}

		/// @brief Round microseconds to the nearest second
		void round_to_nearest_second();

		/// @brief Return the time in microseconds. Useful to calculate time difference.
		int64_t getU_sec() const {return m_date_time>>8;}

		/// @brief set the year ((PCTIME_YEAR_ZERO+1) .. PCTIME_YEAR_LAST)
		void set_year(uint32_t year);

		/// @brief Set the month (1 .. 12)
		void set_month(uint32_t month);

		/// @brief Set the day (1 .. 31)
		void set_day(uint32_t day);

		/// @brief Set the hours (0 .. 23)
		void set_hours(uint32_t hours);

		/// @brief Set the mins (0 .. 59)
		void set_mins(uint32_t mins);

		/// @brief Set the secs (0 .. 59)
		void set_secs(uint32_t secs);

		/// @brief Set the microsecs (0 .. 999999)
		void set_microsecs(uint32_t microsecs);

	private:
		/// @brief Time stored in the format "DATE FORMAT" described above
		uint64_t m_date_time;
};

DASTD_DEF_OSTREAM (time)

} // namespace dastd


//    ___ _   _ _     ___ _   _ _____
//   |_ _| \ | | |   |_ _| \ | | ____|
//    | ||  \| | |    | ||  \| |  _|
//    | || |\  | |___ | || |\  | |___
//   |___|_| \_|_____|___|_| \_|_____|
//


#include "fmt.hpp"
#include "ostream_charbuf.hpp"
#include <vector>

#if !USE_CPP_20_TIME

#if defined DASTD_WINANY
#define DASTD_USE_WINDOWS_TIME
#endif

#ifdef DASTD_USE_WINDOWS_TIME
#define VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif
#endif

namespace dastd {


/// @brief Table with the week days
constexpr const char* time_week_days [7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/// @brief Values in microseconds
constexpr uint64_t dastd_time_MICROSEC  = 256;
constexpr uint64_t dastd_time_SEC       = (dastd_time_MICROSEC * 1000000);
constexpr uint64_t dastd_time_MIN       = (dastd_time_SEC * 60);
constexpr uint64_t dastd_time_HOUR      = (dastd_time_MIN * 60);
constexpr uint64_t dastd_time_DAY       = (dastd_time_HOUR * 24);

constexpr int32_t dastd_time_TZ_MIN = -95;
constexpr int32_t dastd_time_TZ_MAX = 95;

constexpr uint64_t dastd_time_MIN_PCTIME = (UINT64_C(0x0000016E) * dastd_time_DAY);
constexpr uint64_t dastd_time_MAX_PCTIME = (((UINT64_C(0x0005914E)+UINT64_C(365))*dastd_time_DAY) - dastd_time_MICROSEC + dastd_time_TZ_MAX);

constexpr uint32_t dastd_time_YEAR_ZERO = 1600;
constexpr uint32_t dastd_time_YEAR_LAST = 2599;
constexpr size_t dastd_time_NO_YEARS = (dastd_time_YEAR_LAST+1-dastd_time_YEAR_ZERO);

/* Day number of UNIX epoch, i.e. Jan 1st, 1970 */
constexpr uint64_t dastd_time_EPOCH_DAY = UINT64_C(0x00020FE4);

/* Day of week of day N.0, i.e. January 1st, dastd_time_YEAR_ZERO */
/* 0=Sunday, 1=Monday, ..., 6=Saturday */
constexpr uint32_t dastd_time_DAY_OF_WEEK_0 = 6;

/* Return the UTC difference in steps of 15 minutes (i.e. 4 = 1 hour) */
constexpr int32_t dastd_time_TZ(uint64_t datetime) {return ((int32_t)((int8_t)((datetime)&0xFF)));}

/* Invalid time*/
constexpr uint64_t dastd_time_INVALID = 0;

/* set timezone value in microseconds; tz is is 15 min steps (1=15min, 2=30min, etc.) */
/* This value can be added directly to the upper 56 bits without shifting */
constexpr int64_t dastd_time_TZ_usec(int32_t tz) {return (int64_t)((dastd_time_MIN) * 15 * (tz));}

/* The table below contains the first day number of each year starting from */
/* dastd_time_YEAR_ZERO. The most significant bit is 1 if the year is leap. */
static inline constexpr std::array<uint32_t,1000> dastd_time_days = {
	/* YEAR 1600 */ 0x80000000, 0x0000016E, 0x000002DB, 0x00000448, 0x800005B5, 0x00000723, 0x00000890, 0x000009FD, 0x80000B6A, 0x00000CD8,
	/* YEAR 1610 */ 0x00000E45, 0x00000FB2, 0x8000111F, 0x0000128D, 0x000013FA, 0x00001567, 0x800016D4, 0x00001842, 0x000019AF, 0x00001B1C,
	/* YEAR 1620 */ 0x80001C89, 0x00001DF7, 0x00001F64, 0x000020D1, 0x8000223E, 0x000023AC, 0x00002519, 0x00002686, 0x800027F3, 0x00002961,
	/* YEAR 1630 */ 0x00002ACE, 0x00002C3B, 0x80002DA8, 0x00002F16, 0x00003083, 0x000031F0, 0x8000335D, 0x000034CB, 0x00003638, 0x000037A5,
	/* YEAR 1640 */ 0x80003912, 0x00003A80, 0x00003BED, 0x00003D5A, 0x80003EC7, 0x00004035, 0x000041A2, 0x0000430F, 0x8000447C, 0x000045EA,
	/* YEAR 1650 */ 0x00004757, 0x000048C4, 0x80004A31, 0x00004B9F, 0x00004D0C, 0x00004E79, 0x80004FE6, 0x00005154, 0x000052C1, 0x0000542E,
	/* YEAR 1660 */ 0x8000559B, 0x00005709, 0x00005876, 0x000059E3, 0x80005B50, 0x00005CBE, 0x00005E2B, 0x00005F98, 0x80006105, 0x00006273,
	/* YEAR 1670 */ 0x000063E0, 0x0000654D, 0x800066BA, 0x00006828, 0x00006995, 0x00006B02, 0x80006C6F, 0x00006DDD, 0x00006F4A, 0x000070B7,
	/* YEAR 1680 */ 0x80007224, 0x00007392, 0x000074FF, 0x0000766C, 0x800077D9, 0x00007947, 0x00007AB4, 0x00007C21, 0x80007D8E, 0x00007EFC,
	/* YEAR 1690 */ 0x00008069, 0x000081D6, 0x80008343, 0x000084B1, 0x0000861E, 0x0000878B, 0x800088F8, 0x00008A66, 0x00008BD3, 0x00008D40,
	/* YEAR 1700 */ 0x00008EAD, 0x0000901A, 0x00009187, 0x000092F4, 0x80009461, 0x000095CF, 0x0000973C, 0x000098A9, 0x80009A16, 0x00009B84,
	/* YEAR 1710 */ 0x00009CF1, 0x00009E5E, 0x80009FCB, 0x0000A139, 0x0000A2A6, 0x0000A413, 0x8000A580, 0x0000A6EE, 0x0000A85B, 0x0000A9C8,
	/* YEAR 1720 */ 0x8000AB35, 0x0000ACA3, 0x0000AE10, 0x0000AF7D, 0x8000B0EA, 0x0000B258, 0x0000B3C5, 0x0000B532, 0x8000B69F, 0x0000B80D,
	/* YEAR 1730 */ 0x0000B97A, 0x0000BAE7, 0x8000BC54, 0x0000BDC2, 0x0000BF2F, 0x0000C09C, 0x8000C209, 0x0000C377, 0x0000C4E4, 0x0000C651,
	/* YEAR 1740 */ 0x8000C7BE, 0x0000C92C, 0x0000CA99, 0x0000CC06, 0x8000CD73, 0x0000CEE1, 0x0000D04E, 0x0000D1BB, 0x8000D328, 0x0000D496,
	/* YEAR 1750 */ 0x0000D603, 0x0000D770, 0x8000D8DD, 0x0000DA4B, 0x0000DBB8, 0x0000DD25, 0x8000DE92, 0x0000E000, 0x0000E16D, 0x0000E2DA,
	/* YEAR 1760 */ 0x8000E447, 0x0000E5B5, 0x0000E722, 0x0000E88F, 0x8000E9FC, 0x0000EB6A, 0x0000ECD7, 0x0000EE44, 0x8000EFB1, 0x0000F11F,
	/* YEAR 1770 */ 0x0000F28C, 0x0000F3F9, 0x8000F566, 0x0000F6D4, 0x0000F841, 0x0000F9AE, 0x8000FB1B, 0x0000FC89, 0x0000FDF6, 0x0000FF63,
	/* YEAR 1780 */ 0x800100D0, 0x0001023E, 0x000103AB, 0x00010518, 0x80010685, 0x000107F3, 0x00010960, 0x00010ACD, 0x80010C3A, 0x00010DA8,
	/* YEAR 1790 */ 0x00010F15, 0x00011082, 0x800111EF, 0x0001135D, 0x000114CA, 0x00011637, 0x800117A4, 0x00011912, 0x00011A7F, 0x00011BEC,
	/* YEAR 1800 */ 0x00011D59, 0x00011EC6, 0x00012033, 0x000121A0, 0x8001230D, 0x0001247B, 0x000125E8, 0x00012755, 0x800128C2, 0x00012A30,
	/* YEAR 1810 */ 0x00012B9D, 0x00012D0A, 0x80012E77, 0x00012FE5, 0x00013152, 0x000132BF, 0x8001342C, 0x0001359A, 0x00013707, 0x00013874,
	/* YEAR 1820 */ 0x800139E1, 0x00013B4F, 0x00013CBC, 0x00013E29, 0x80013F96, 0x00014104, 0x00014271, 0x000143DE, 0x8001454B, 0x000146B9,
	/* YEAR 1830 */ 0x00014826, 0x00014993, 0x80014B00, 0x00014C6E, 0x00014DDB, 0x00014F48, 0x800150B5, 0x00015223, 0x00015390, 0x000154FD,
	/* YEAR 1840 */ 0x8001566A, 0x000157D8, 0x00015945, 0x00015AB2, 0x80015C1F, 0x00015D8D, 0x00015EFA, 0x00016067, 0x800161D4, 0x00016342,
	/* YEAR 1850 */ 0x000164AF, 0x0001661C, 0x80016789, 0x000168F7, 0x00016A64, 0x00016BD1, 0x80016D3E, 0x00016EAC, 0x00017019, 0x00017186,
	/* YEAR 1860 */ 0x800172F3, 0x00017461, 0x000175CE, 0x0001773B, 0x800178A8, 0x00017A16, 0x00017B83, 0x00017CF0, 0x80017E5D, 0x00017FCB,
	/* YEAR 1870 */ 0x00018138, 0x000182A5, 0x80018412, 0x00018580, 0x000186ED, 0x0001885A, 0x800189C7, 0x00018B35, 0x00018CA2, 0x00018E0F,
	/* YEAR 1880 */ 0x80018F7C, 0x000190EA, 0x00019257, 0x000193C4, 0x80019531, 0x0001969F, 0x0001980C, 0x00019979, 0x80019AE6, 0x00019C54,
	/* YEAR 1890 */ 0x00019DC1, 0x00019F2E, 0x8001A09B, 0x0001A209, 0x0001A376, 0x0001A4E3, 0x8001A650, 0x0001A7BE, 0x0001A92B, 0x0001AA98,
	/* YEAR 1900 */ 0x0001AC05, 0x0001AD72, 0x0001AEDF, 0x0001B04C, 0x8001B1B9, 0x0001B327, 0x0001B494, 0x0001B601, 0x8001B76E, 0x0001B8DC,
	/* YEAR 1910 */ 0x0001BA49, 0x0001BBB6, 0x8001BD23, 0x0001BE91, 0x0001BFFE, 0x0001C16B, 0x8001C2D8, 0x0001C446, 0x0001C5B3, 0x0001C720,
	/* YEAR 1920 */ 0x8001C88D, 0x0001C9FB, 0x0001CB68, 0x0001CCD5, 0x8001CE42, 0x0001CFB0, 0x0001D11D, 0x0001D28A, 0x8001D3F7, 0x0001D565,
	/* YEAR 1930 */ 0x0001D6D2, 0x0001D83F, 0x8001D9AC, 0x0001DB1A, 0x0001DC87, 0x0001DDF4, 0x8001DF61, 0x0001E0CF, 0x0001E23C, 0x0001E3A9,
	/* YEAR 1940 */ 0x8001E516, 0x0001E684, 0x0001E7F1, 0x0001E95E, 0x8001EACB, 0x0001EC39, 0x0001EDA6, 0x0001EF13, 0x8001F080, 0x0001F1EE,
	/* YEAR 1950 */ 0x0001F35B, 0x0001F4C8, 0x8001F635, 0x0001F7A3, 0x0001F910, 0x0001FA7D, 0x8001FBEA, 0x0001FD58, 0x0001FEC5, 0x00020032,
	/* YEAR 1960 */ 0x8002019F, 0x0002030D, 0x0002047A, 0x000205E7, 0x80020754, 0x000208C2, 0x00020A2F, 0x00020B9C, 0x80020D09, 0x00020E77,
	/* YEAR 1970 */ 0x00020FE4, 0x00021151, 0x800212BE, 0x0002142C, 0x00021599, 0x00021706, 0x80021873, 0x000219E1, 0x00021B4E, 0x00021CBB,
	/* YEAR 1980 */ 0x80021E28, 0x00021F96, 0x00022103, 0x00022270, 0x800223DD, 0x0002254B, 0x000226B8, 0x00022825, 0x80022992, 0x00022B00,
	/* YEAR 1990 */ 0x00022C6D, 0x00022DDA, 0x80022F47, 0x000230B5, 0x00023222, 0x0002338F, 0x800234FC, 0x0002366A, 0x000237D7, 0x00023944,
	/* YEAR 2000 */ 0x80023AB1, 0x00023C1F, 0x00023D8C, 0x00023EF9, 0x80024066, 0x000241D4, 0x00024341, 0x000244AE, 0x8002461B, 0x00024789,
	/* YEAR 2010 */ 0x000248F6, 0x00024A63, 0x80024BD0, 0x00024D3E, 0x00024EAB, 0x00025018, 0x80025185, 0x000252F3, 0x00025460, 0x000255CD,
	/* YEAR 2020 */ 0x8002573A, 0x000258A8, 0x00025A15, 0x00025B82, 0x80025CEF, 0x00025E5D, 0x00025FCA, 0x00026137, 0x800262A4, 0x00026412,
	/* YEAR 2030 */ 0x0002657F, 0x000266EC, 0x80026859, 0x000269C7, 0x00026B34, 0x00026CA1, 0x80026E0E, 0x00026F7C, 0x000270E9, 0x00027256,
	/* YEAR 2040 */ 0x800273C3, 0x00027531, 0x0002769E, 0x0002780B, 0x80027978, 0x00027AE6, 0x00027C53, 0x00027DC0, 0x80027F2D, 0x0002809B,
	/* YEAR 2050 */ 0x00028208, 0x00028375, 0x800284E2, 0x00028650, 0x000287BD, 0x0002892A, 0x80028A97, 0x00028C05, 0x00028D72, 0x00028EDF,
	/* YEAR 2060 */ 0x8002904C, 0x000291BA, 0x00029327, 0x00029494, 0x80029601, 0x0002976F, 0x000298DC, 0x00029A49, 0x80029BB6, 0x00029D24,
	/* YEAR 2070 */ 0x00029E91, 0x00029FFE, 0x8002A16B, 0x0002A2D9, 0x0002A446, 0x0002A5B3, 0x8002A720, 0x0002A88E, 0x0002A9FB, 0x0002AB68,
	/* YEAR 2080 */ 0x8002ACD5, 0x0002AE43, 0x0002AFB0, 0x0002B11D, 0x8002B28A, 0x0002B3F8, 0x0002B565, 0x0002B6D2, 0x8002B83F, 0x0002B9AD,
	/* YEAR 2090 */ 0x0002BB1A, 0x0002BC87, 0x8002BDF4, 0x0002BF62, 0x0002C0CF, 0x0002C23C, 0x8002C3A9, 0x0002C517, 0x0002C684, 0x0002C7F1,
	/* YEAR 2100 */ 0x0002C95E, 0x0002CACB, 0x0002CC38, 0x0002CDA5, 0x8002CF12, 0x0002D080, 0x0002D1ED, 0x0002D35A, 0x8002D4C7, 0x0002D635,
	/* YEAR 2110 */ 0x0002D7A2, 0x0002D90F, 0x8002DA7C, 0x0002DBEA, 0x0002DD57, 0x0002DEC4, 0x8002E031, 0x0002E19F, 0x0002E30C, 0x0002E479,
	/* YEAR 2120 */ 0x8002E5E6, 0x0002E754, 0x0002E8C1, 0x0002EA2E, 0x8002EB9B, 0x0002ED09, 0x0002EE76, 0x0002EFE3, 0x8002F150, 0x0002F2BE,
	/* YEAR 2130 */ 0x0002F42B, 0x0002F598, 0x8002F705, 0x0002F873, 0x0002F9E0, 0x0002FB4D, 0x8002FCBA, 0x0002FE28, 0x0002FF95, 0x00030102,
	/* YEAR 2140 */ 0x8003026F, 0x000303DD, 0x0003054A, 0x000306B7, 0x80030824, 0x00030992, 0x00030AFF, 0x00030C6C, 0x80030DD9, 0x00030F47,
	/* YEAR 2150 */ 0x000310B4, 0x00031221, 0x8003138E, 0x000314FC, 0x00031669, 0x000317D6, 0x80031943, 0x00031AB1, 0x00031C1E, 0x00031D8B,
	/* YEAR 2160 */ 0x80031EF8, 0x00032066, 0x000321D3, 0x00032340, 0x800324AD, 0x0003261B, 0x00032788, 0x000328F5, 0x80032A62, 0x00032BD0,
	/* YEAR 2170 */ 0x00032D3D, 0x00032EAA, 0x80033017, 0x00033185, 0x000332F2, 0x0003345F, 0x800335CC, 0x0003373A, 0x000338A7, 0x00033A14,
	/* YEAR 2180 */ 0x80033B81, 0x00033CEF, 0x00033E5C, 0x00033FC9, 0x80034136, 0x000342A4, 0x00034411, 0x0003457E, 0x800346EB, 0x00034859,
	/* YEAR 2190 */ 0x000349C6, 0x00034B33, 0x80034CA0, 0x00034E0E, 0x00034F7B, 0x000350E8, 0x80035255, 0x000353C3, 0x00035530, 0x0003569D,
	/* YEAR 2200 */ 0x0003580A, 0x00035977, 0x00035AE4, 0x00035C51, 0x80035DBE, 0x00035F2C, 0x00036099, 0x00036206, 0x80036373, 0x000364E1,
	/* YEAR 2210 */ 0x0003664E, 0x000367BB, 0x80036928, 0x00036A96, 0x00036C03, 0x00036D70, 0x80036EDD, 0x0003704B, 0x000371B8, 0x00037325,
	/* YEAR 2220 */ 0x80037492, 0x00037600, 0x0003776D, 0x000378DA, 0x80037A47, 0x00037BB5, 0x00037D22, 0x00037E8F, 0x80037FFC, 0x0003816A,
	/* YEAR 2230 */ 0x000382D7, 0x00038444, 0x800385B1, 0x0003871F, 0x0003888C, 0x000389F9, 0x80038B66, 0x00038CD4, 0x00038E41, 0x00038FAE,
	/* YEAR 2240 */ 0x8003911B, 0x00039289, 0x000393F6, 0x00039563, 0x800396D0, 0x0003983E, 0x000399AB, 0x00039B18, 0x80039C85, 0x00039DF3,
	/* YEAR 2250 */ 0x00039F60, 0x0003A0CD, 0x8003A23A, 0x0003A3A8, 0x0003A515, 0x0003A682, 0x8003A7EF, 0x0003A95D, 0x0003AACA, 0x0003AC37,
	/* YEAR 2260 */ 0x8003ADA4, 0x0003AF12, 0x0003B07F, 0x0003B1EC, 0x8003B359, 0x0003B4C7, 0x0003B634, 0x0003B7A1, 0x8003B90E, 0x0003BA7C,
	/* YEAR 2270 */ 0x0003BBE9, 0x0003BD56, 0x8003BEC3, 0x0003C031, 0x0003C19E, 0x0003C30B, 0x8003C478, 0x0003C5E6, 0x0003C753, 0x0003C8C0,
	/* YEAR 2280 */ 0x8003CA2D, 0x0003CB9B, 0x0003CD08, 0x0003CE75, 0x8003CFE2, 0x0003D150, 0x0003D2BD, 0x0003D42A, 0x8003D597, 0x0003D705,
	/* YEAR 2290 */ 0x0003D872, 0x0003D9DF, 0x8003DB4C, 0x0003DCBA, 0x0003DE27, 0x0003DF94, 0x8003E101, 0x0003E26F, 0x0003E3DC, 0x0003E549,
	/* YEAR 2300 */ 0x0003E6B6, 0x0003E823, 0x0003E990, 0x0003EAFD, 0x8003EC6A, 0x0003EDD8, 0x0003EF45, 0x0003F0B2, 0x8003F21F, 0x0003F38D,
	/* YEAR 2310 */ 0x0003F4FA, 0x0003F667, 0x8003F7D4, 0x0003F942, 0x0003FAAF, 0x0003FC1C, 0x8003FD89, 0x0003FEF7, 0x00040064, 0x000401D1,
	/* YEAR 2320 */ 0x8004033E, 0x000404AC, 0x00040619, 0x00040786, 0x800408F3, 0x00040A61, 0x00040BCE, 0x00040D3B, 0x80040EA8, 0x00041016,
	/* YEAR 2330 */ 0x00041183, 0x000412F0, 0x8004145D, 0x000415CB, 0x00041738, 0x000418A5, 0x80041A12, 0x00041B80, 0x00041CED, 0x00041E5A,
	/* YEAR 2340 */ 0x80041FC7, 0x00042135, 0x000422A2, 0x0004240F, 0x8004257C, 0x000426EA, 0x00042857, 0x000429C4, 0x80042B31, 0x00042C9F,
	/* YEAR 2350 */ 0x00042E0C, 0x00042F79, 0x800430E6, 0x00043254, 0x000433C1, 0x0004352E, 0x8004369B, 0x00043809, 0x00043976, 0x00043AE3,
	/* YEAR 2360 */ 0x80043C50, 0x00043DBE, 0x00043F2B, 0x00044098, 0x80044205, 0x00044373, 0x000444E0, 0x0004464D, 0x800447BA, 0x00044928,
	/* YEAR 2370 */ 0x00044A95, 0x00044C02, 0x80044D6F, 0x00044EDD, 0x0004504A, 0x000451B7, 0x80045324, 0x00045492, 0x000455FF, 0x0004576C,
	/* YEAR 2380 */ 0x800458D9, 0x00045A47, 0x00045BB4, 0x00045D21, 0x80045E8E, 0x00045FFC, 0x00046169, 0x000462D6, 0x80046443, 0x000465B1,
	/* YEAR 2390 */ 0x0004671E, 0x0004688B, 0x800469F8, 0x00046B66, 0x00046CD3, 0x00046E40, 0x80046FAD, 0x0004711B, 0x00047288, 0x000473F5,
	/* YEAR 2400 */ 0x80047562, 0x000476D0, 0x0004783D, 0x000479AA, 0x80047B17, 0x00047C85, 0x00047DF2, 0x00047F5F, 0x800480CC, 0x0004823A,
	/* YEAR 2410 */ 0x000483A7, 0x00048514, 0x80048681, 0x000487EF, 0x0004895C, 0x00048AC9, 0x80048C36, 0x00048DA4, 0x00048F11, 0x0004907E,
	/* YEAR 2420 */ 0x800491EB, 0x00049359, 0x000494C6, 0x00049633, 0x800497A0, 0x0004990E, 0x00049A7B, 0x00049BE8, 0x80049D55, 0x00049EC3,
	/* YEAR 2430 */ 0x0004A030, 0x0004A19D, 0x8004A30A, 0x0004A478, 0x0004A5E5, 0x0004A752, 0x8004A8BF, 0x0004AA2D, 0x0004AB9A, 0x0004AD07,
	/* YEAR 2440 */ 0x8004AE74, 0x0004AFE2, 0x0004B14F, 0x0004B2BC, 0x8004B429, 0x0004B597, 0x0004B704, 0x0004B871, 0x8004B9DE, 0x0004BB4C,
	/* YEAR 2450 */ 0x0004BCB9, 0x0004BE26, 0x8004BF93, 0x0004C101, 0x0004C26E, 0x0004C3DB, 0x8004C548, 0x0004C6B6, 0x0004C823, 0x0004C990,
	/* YEAR 2460 */ 0x8004CAFD, 0x0004CC6B, 0x0004CDD8, 0x0004CF45, 0x8004D0B2, 0x0004D220, 0x0004D38D, 0x0004D4FA, 0x8004D667, 0x0004D7D5,
	/* YEAR 2470 */ 0x0004D942, 0x0004DAAF, 0x8004DC1C, 0x0004DD8A, 0x0004DEF7, 0x0004E064, 0x8004E1D1, 0x0004E33F, 0x0004E4AC, 0x0004E619,
	/* YEAR 2480 */ 0x8004E786, 0x0004E8F4, 0x0004EA61, 0x0004EBCE, 0x8004ED3B, 0x0004EEA9, 0x0004F016, 0x0004F183, 0x8004F2F0, 0x0004F45E,
	/* YEAR 2490 */ 0x0004F5CB, 0x0004F738, 0x8004F8A5, 0x0004FA13, 0x0004FB80, 0x0004FCED, 0x8004FE5A, 0x0004FFC8, 0x00050135, 0x000502A2,
	/* YEAR 2500 */ 0x0005040F, 0x0005057C, 0x000506E9, 0x00050856, 0x800509C3, 0x00050B31, 0x00050C9E, 0x00050E0B, 0x80050F78, 0x000510E6,
	/* YEAR 2510 */ 0x00051253, 0x000513C0, 0x8005152D, 0x0005169B, 0x00051808, 0x00051975, 0x80051AE2, 0x00051C50, 0x00051DBD, 0x00051F2A,
	/* YEAR 2520 */ 0x80052097, 0x00052205, 0x00052372, 0x000524DF, 0x8005264C, 0x000527BA, 0x00052927, 0x00052A94, 0x80052C01, 0x00052D6F,
	/* YEAR 2530 */ 0x00052EDC, 0x00053049, 0x800531B6, 0x00053324, 0x00053491, 0x000535FE, 0x8005376B, 0x000538D9, 0x00053A46, 0x00053BB3,
	/* YEAR 2540 */ 0x80053D20, 0x00053E8E, 0x00053FFB, 0x00054168, 0x800542D5, 0x00054443, 0x000545B0, 0x0005471D, 0x8005488A, 0x000549F8,
	/* YEAR 2550 */ 0x00054B65, 0x00054CD2, 0x80054E3F, 0x00054FAD, 0x0005511A, 0x00055287, 0x800553F4, 0x00055562, 0x000556CF, 0x0005583C,
	/* YEAR 2560 */ 0x800559A9, 0x00055B17, 0x00055C84, 0x00055DF1, 0x80055F5E, 0x000560CC, 0x00056239, 0x000563A6, 0x80056513, 0x00056681,
	/* YEAR 2570 */ 0x000567EE, 0x0005695B, 0x80056AC8, 0x00056C36, 0x00056DA3, 0x00056F10, 0x8005707D, 0x000571EB, 0x00057358, 0x000574C5,
	/* YEAR 2580 */ 0x80057632, 0x000577A0, 0x0005790D, 0x00057A7A, 0x80057BE7, 0x00057D55, 0x00057EC2, 0x0005802F, 0x8005819C, 0x0005830A,
	/* YEAR 2590 */ 0x00058477, 0x000585E4, 0x80058751, 0x000588BF, 0x00058A2C, 0x00058B99, 0x80058D06, 0x00058E74, 0x00058FE1, 0x0005914E
};

/* These tables indicate the day-0 offset for each month, for nonleap and leap months */
inline const std::vector<uint32_t> time_mon = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
inline const std::vector<uint32_t> time_mon_leap = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

/* This table stores the number of days for each month */
inline const std::vector<uint32_t> time_mon_days = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Windows function - Set from a FILETIME
#ifdef DASTD_USE_WINDOWS_TIME
inline uint64_t convertFILETIME_to_time(FILETIME ft)
{
	return ((((uint64_t)(ft).dwHighDateTime) << 32) | ((uint64_t)(ft).dwLowDateTime)) / UINT64_C(10) * dastd_time_MICROSEC + UINT64_C(0x0000016E) * dastd_time_DAY;
}
#endif



//------------------------------------------------------------------------------
// (brief) Print the timezone according to the selected options
// (param) out Output stream
// (param) time_print_opt A combination of `TIME_xxx` flags
//------------------------------------------------------------------------------
inline void timezone::print(std::ostream& out, uint32_t time_print_opt) const
{
	bool use_underscores = DASTD_ISSET(time_print_opt, TIME_UNDERSCORE);
	#define SEP(p) (use_underscores ? '_' : p)
	switch(time_print_opt & TIME_FMT_MASK) {
		case TIME_FMT_ISO8601: {
			if (m_tz == 0) {out << "Z"; return;}
			out << (m_tz < 0 ? '-' : '+') << fmt((m_tz < 0 ? -m_tz : m_tz)/4, 10, 2);
			if ((m_tz % 4) != 0) out << SEP(':') << fmt(((m_tz < 0 ? -m_tz : m_tz)%4) * 15, 10, 2);
			break;
		}
		case TIME_FMT_PACKED: {
			out << (m_tz < 0 ? '-' : '+') << fmt((m_tz < 0 ? -m_tz : m_tz)/4, 10, 2) << fmt(((m_tz < 0 ? -m_tz : m_tz)%4) * 15, 10, 2);
			break;
		}
		default: {
			out << "UTC" << (m_tz < 0 ? '-' : '+') << fmt((m_tz < 0 ? -m_tz : m_tz)/4, 10, 2) << SEP(':') << fmt(((m_tz < 0 ? -m_tz : m_tz)%4) * 15, 10, 2);
			break;
		}
	}
	#undef SEP
}

/*------------------------------------------------------------------------------
	print
------------------------------------------------------------------------------*/
inline void decomposed_time::print(std::ostream& out, uint32_t time_print_opt) const
{
	bool use_underscores = DASTD_ISSET(time_print_opt, TIME_UNDERSCORE);
	#define SEP(p) (use_underscores ? '_' : p)

	switch(time_print_opt & TIME_FMT_MASK) {
		case TIME_FMT_ISO8601: {
			out << fmt(m_year, 10, 4) << SEP('-') << fmt(m_month, 10, 2) << SEP('-') << fmt(m_day, 10, 2)
				<< SEP('T')
				<< fmt(m_hours, 10, 2) << SEP(':') << fmt(m_mins, 10, 2) << SEP(':') << fmt(m_secs, 10, 2)
			;
			break;
		}
		case TIME_FMT_PACKED: {
			out << fmt(m_year, 10, 4) << fmt(m_month, 10, 2) << fmt(m_day, 10, 2)
				<< SEP(' ')
				<< fmt(m_hours, 10, 2) << fmt(m_mins, 10, 2) << fmt(m_secs, 10, 2)
			;
			break;
		}
		default: {
			out << fmt(m_year, 10, 4) << SEP('-') << fmt(m_month, 10, 2) << SEP('-') << fmt(m_day, 10, 2)
				<< SEP(' ')
				<< fmt(m_hours, 10, 2) << SEP(':') << fmt(m_mins, 10, 2) << SEP(':') << fmt(m_secs, 10, 2)
			;
			break;
		}
	}

	if (DASTD_ISSET(time_print_opt, TIME_MICROS)) out << SEP('.') << fmt(m_microsecs, 10, 6);
	else if (DASTD_ISSET(time_print_opt, TIME_MILLIS)) out << SEP('.') << fmt(m_microsecs/1000, 10, 3);

	if (DASTD_ISSET(time_print_opt, TIME_TZ)) {
		if ((time_print_opt & TIME_FMT_MASK) != TIME_FMT_ISO8601) out << SEP(' ');
		m_tz.print(out, time_print_opt);
	}

	if (DASTD_ISSET(time_print_opt, TIME_WEEKDAY)) {
		out << SEP(' ') << time_week_days[m_day_of_week];
	}
	#undef SEP
}

//------------------------------------------------------------------------------
// (brief) Write a timestamp using only numbers and underscores.
//
// (param) out   The output stream that receives the string. It must
//               must be able to accept at least TIMESTAMP_LENGTH bytes.
//------------------------------------------------------------------------------
inline void decomposed_time::write_clean_timestamp(std::ostream& out, bool include_microsec) const
{
	out << fmt(m_year, 10, 4) << fmt(m_month, 10, 2) << fmt(m_day, 10, 2) << '_' << fmt(m_hours, 10, 2) << fmt(m_mins, 10, 2) << fmt(m_secs, 10, 2);
	if (include_microsec) {
		out << "_" << fmt(m_microsecs, 10, 6);
	}
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
inline time::time(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t mins, uint32_t secs, timezone tz)
{
	clear();
	if (!set(year, month, day, hours, mins, secs, tz)) clear();
}

/*---------------------------------------------------------------------------
	Returns true if it is valid
---------------------------------------------------------------------------*/
inline bool time::is_valid() const
{
	if (m_date_time < dastd_time_MIN_PCTIME) return false;
	if (m_date_time > dastd_time_MAX_PCTIME) return false;
	return true;
}

/*---------------------------------------------------------------------------
	Given a date in internal format, it returns day, month, etc.
---------------------------------------------------------------------------*/
inline void time::decompose(decomposed_time* decomposed) const
{
	uint32_t day_no;
	uint64_t microsecs;
	uint32_t day_zero;
	uint32_t day_in_year;
	size_t position;
	const std::vector<uint32_t>* months;

	if (!is_valid()) {
		decomposed->clear();
	}
	else {
		day_no = (uint32_t)(m_date_time / dastd_time_DAY);

		// From the day number, find the year
		auto year_iter = std::upper_bound(dastd_time_days.begin(), dastd_time_days.end(), day_no, [](uint32_t a, uint32_t b) {return (a & 0x7FFFFFFF) < (b & 0x7FFFFFFF);});
		position = std::distance(dastd_time_days.begin(), year_iter)-1;

		decomposed->m_year = (uint32_t)(dastd_time_YEAR_ZERO + position);

		assert(position < dastd_time_NO_YEARS);
		day_zero = (dastd_time_days[position] & ~0x80000000U);
		assert(day_zero <= day_no);
		day_in_year = day_no - day_zero;

		if ((dastd_time_days[position] & 0x80000000U) != 0) {
			/* LEAP YEARS */
			assert(day_zero + 366 > day_no);
			assert(day_in_year < 366);
			months = &time_mon_leap;
		}
		else {
			/* NON LEAP YEARS */
			assert(day_zero + 365 > day_no);
			assert(day_in_year < 365);
			months = &time_mon;
		}

		// Find the month knowing the day of the year
		auto month_iter = std::upper_bound(months->begin(), months->end(), day_in_year);
		position = std::distance(months->begin(), month_iter)-1;
		assert(position < 12);

		decomposed->m_month = (uint32_t)(position+1);
		decomposed->m_day = day_in_year - (*months)[position] + 1;

		/* Calculate time */
		microsecs = m_date_time / dastd_time_MICROSEC;

		decomposed->m_microsecs = (uint32_t)(microsecs % UINT64_C(1000000));
		assert(decomposed->m_microsecs < UINT64_C(1000000));
		microsecs /= UINT64_C(1000000);

		decomposed->m_secs = (uint32_t)(microsecs % UINT64_C(60));
		assert(decomposed->m_secs < UINT64_C(60));
		microsecs /= UINT64_C(60);

		decomposed->m_mins = (uint32_t)(microsecs % UINT64_C(60));
		assert(decomposed->m_mins < UINT64_C(60));
		microsecs /= UINT64_C(60);

		decomposed->m_hours = (uint32_t)(microsecs % UINT64_C(24));
		assert(decomposed->m_hours < UINT64_C(24));

		decomposed->m_tz = dastd_time_TZ(m_date_time);

		decomposed->m_day_of_week = (day_no + dastd_time_DAY_OF_WEEK_0) % 7;
		decomposed->m_day_of_year = day_in_year;
	}
}

/*---------------------------------------------------------------------------
	Given a date in day, month, etc. format, it returns the internal format.
---------------------------------------------------------------------------*/
inline bool time::compose (const decomposed_time* decomposed)
{
	m_date_time = UINT64_C(0);
	uint32_t day0;
	uint32_t is_leap;

	/* Check the ranges and return invalid immediately */
	if (decomposed->m_year <= dastd_time_YEAR_ZERO || decomposed->m_year > dastd_time_YEAR_LAST
		|| decomposed->m_month < 1 || decomposed->m_month > 12 || decomposed->m_day < 1
		|| decomposed->m_hours >= 24 || decomposed->m_mins >= 60 || decomposed->m_secs >= 60
		|| decomposed->m_microsecs >= 1000000
		|| decomposed->m_tz.m_tz < dastd_time_TZ_MIN || decomposed->m_tz.m_tz > dastd_time_TZ_MAX
		) {
		clear();
		return false;
	}

	day0 = dastd_time_days [decomposed->m_year - dastd_time_YEAR_ZERO] & ~0x80000000;
	is_leap = ((dastd_time_days [decomposed->m_year - dastd_time_YEAR_ZERO] & 0x80000000) != 0);

	if (decomposed->m_day  > time_mon_days [decomposed->m_month-1]) {
		/* Allow only one special case: february 29th on leap years. */
		if (decomposed->m_month != 2 || decomposed->m_day != 29 || !is_leap) {
			clear();
			return false;
		}
	}

	m_date_time =
		((((uint64_t)day0) + ((uint64_t)((is_leap ? time_mon_leap : time_mon)[decomposed->m_month-1]))
		+ ((uint64_t)(decomposed->m_day - 1))) * dastd_time_DAY
		+ ((uint64_t)(decomposed->m_hours)) * dastd_time_HOUR
		+ ((uint64_t)(decomposed->m_mins)) * dastd_time_MIN
		+ ((uint64_t)(decomposed->m_secs)) * dastd_time_SEC
		+ ((uint64_t)(decomposed->m_microsecs)* dastd_time_MICROSEC))
		| (uint64_t)((uint8_t)((int8_t)(decomposed->m_tz.m_tz)))
	;

	return true;
}

//------------------------------------------------------------------------------
// set from year, month, day, hours, minutes, seconds and timezone
//------------------------------------------------------------------------------
inline bool time::set(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t mins, uint32_t secs, timezone tz)
{
	decomposed_time decomp_time(year, month, day, hours, mins, secs, tz);
	return compose(decomp_time);
}


//------------------------------------------------------------------------------
// set from year, month, day, hours, minutes, seconds and timezone
//------------------------------------------------------------------------------
inline epoch32_t time::make_epoch32(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t mins, uint32_t secs, timezone tz)
{
	return (epoch32_t)make_epoch64(year, month, day, hours, mins, secs, tz);
}

//------------------------------------------------------------------------------
// set from year, month, day, hours, minutes, seconds and timezone
//------------------------------------------------------------------------------
inline epoch64_t time::make_epoch64(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t mins, uint32_t secs, timezone tz)
{
	time t;
	if (!t.set(year, month, day, hours, mins, secs, tz)) return 0;
	return t.get_epoch64();
}

/*---------------------------------------------------------------------------
	Returns the current timezone, that includes DST (Daylight Saving Time)
---------------------------------------------------------------------------*/
inline timezone time::get_system_time_zone ()
{
	return seconds_to_be_added_toUTC_to_obtain_local_time()/900;
}

//---------------------------------------------------------------------------
// Returns the difference of the current timezone in seconds from UTC
// This value can be added to an UTC epoch to obtain the local time or
// subtracted from a local time to get UTC.
//---------------------------------------------------------------------------
inline int32_t time::seconds_to_be_added_toUTC_to_obtain_local_time()
{
	// UPGRADE TO STDC++ 2020
	#if USE_CPP_20_TIME
		return (int32_t)std::chrono::current_zone()->get_info(std::chrono::system_clock::now()).offset.count();
	#else
		#ifdef DASTD_USE_WINDOWS_TIME
		TIME_ZONE_INFORMATION tz_info;
		DWORD ret;
		ret = GetTimeZoneInformation(&tz_info);
		return (- (tz_info.Bias + (ret == TIME_ZONE_ID_DAYLIGHT ? tz_info.DaylightBias : tz_info.StandardBias))) * 60;
		#else
			#ifdef DASTD_ESP32
				// EspressIF does not support the tm_gmtoff field
				return 0;
			#else
				struct tm tm;
				time_t curtime;
				curtime = ::time(nullptr);
				localtime_r(&curtime, &tm);

				return (int32_t)(tm.tm_gmtoff);
			#endif
		#endif
	#endif
}

/*---------------------------------------------------------------------------
	Returns the current time in UTC form.
---------------------------------------------------------------------------*/
inline void time::set_current_timeUTC ()
{
	#if USE_CPP_20_TIME
	const std::chrono::utc_clock::duration utc_epoch = std::chrono::utc_clock::now().time_since_epoch();
	const std::chrono::seconds utc_s = std::chrono::duration_cast<std::chrono::seconds>(utc_epoch);
	set_from_epochUTC((dastd::epoch64_t)(utc_s.count()));
	#else
	#ifdef DASTD_USE_WINDOWS_TIME
	FILETIME ft;
	GetSystemTimeAsFileTime (&ft);
	m_date_time = convertFILETIME_to_time(ft);
	#else
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	m_date_time = dastd_time_SEC * tv.tv_sec + dastd_time_MICROSEC * tv.tv_usec + dastd_time_EPOCH_DAY * dastd_time_DAY;
	#endif
	#endif
	assert(is_valid());
}

//---------------------------------------------------------------------------
// Set the current time in Local time form.
//---------------------------------------------------------------------------
inline void time::set_current_time_local()
{
	set_current_timeUTC();
	set_timezone(get_system_time_zone());
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Return the timezone of the time here stored.
// Timezone 'tz' is in steps of 15'.
//------------------------------------------------------------------------------
inline timezone time::get_timezone() const
{
	if (!is_valid()) return 0;
	return dastd_time_TZ(m_date_time);
}

//------------------------------------------------------------------------------
// Set the timezone also updating the internal time.
// Timezone 'tz' is in steps of 15'.
//------------------------------------------------------------------------------
inline void time::set_timezone(timezone tz)
{
	if (!is_valid()) return;

	// Get the current time zone
	timezone oldTZ = dastd_time_TZ(m_date_time);

	uint64_t usec = (m_date_time & ~0xFF);

	// Subtract the old timezone and add the new one
	int64_t t1 = dastd_time_TZ_usec(oldTZ.m_tz);
	int64_t t2 = dastd_time_TZ_usec(tz.m_tz);
	usec = (uint64_t)(usec - t1 + t2);

	m_date_time = (usec) | (uint64_t)((uint8_t)((int8_t)(tz.m_tz)));

}

/*------------------------------------------------------------------------------
	print
------------------------------------------------------------------------------*/
inline void time::print (std::ostream& o, uint32_t print_parts) const
{
	if (is_valid ()) {
		decomposed_time comp;
		decompose(&comp);
		comp.print(o, print_parts);
	}
	else {
		o << "INVALID-TIME";
	}
}

/*------------------------------------------------------------------------------
	Set from EPOCH UTC
------------------------------------------------------------------------------*/
inline void time::set_from_epochUTC(epoch64_t epoch)
{
	m_date_time = ((epoch) * dastd_time_SEC + dastd_time_EPOCH_DAY * dastd_time_DAY);
}

/*------------------------------------------------------------------------------
	Set from EPOCH UTC microseconds
------------------------------------------------------------------------------*/
inline void time::set_from_epochUTC_micros(uint64_t micros)
{
	m_date_time = ((micros) * dastd_time_MICROSEC + dastd_time_EPOCH_DAY * dastd_time_DAY);
}

/*---------------------------------------------------------------------------
 Return the date as EPOCH (seconds from Jan 1st, 1970)
 The time is returned it whatever timezone it is.
 So if it is 12:34 UTC+1, it returns 12:34.
 The value can be converted to 32-bit if dates are within the 32-bit
 limit of epoch 18th jan, 2038.
---------------------------------------------------------------------------*/
inline epoch64_t time::get_epoch64() const
{
	if (!is_valid()) return UINT64_C(0);

	return ((m_date_time & ~0xFF) - dastd_time_EPOCH_DAY * dastd_time_DAY) / dastd_time_SEC;
}

/*---------------------------------------------------------------------------
	Compare two times. Returns -1 if t1<t2, ==0 if t1==t2, 1 if t1>t2
---------------------------------------------------------------------------*/
inline int time::compare (const time& other) const
{
	uint64_t t1=m_date_time;
	uint64_t t2=other.m_date_time;
	if (!is_valid()) t1 = dastd_time_INVALID;
	if (!other.is_valid ()) t2 = dastd_time_INVALID;
	return DASTD_NUMCMP (t1, t2);
}

/*---------------------------------------------------------------------------
	Round microseconds to the nearest second
---------------------------------------------------------------------------*/
inline void time::round_to_nearest_second()
{
	uint64_t us = (m_date_time >> 8);
	bool add_sec = (us % UINT64_C(1000000)) >= UINT64_C(500000);
	us = (us / UINT64_C(1000000) + (add_sec ? 1 : 0)) * UINT64_C(1000000);
	m_date_time = (us << 8) | (m_date_time & 0xFF);
}

//------------------------------------------------------------------------------
// Set the year ((dastd_time_YEAR_ZERO+1) .. dastd_time_YEAR_LAST)
//------------------------------------------------------------------------------
inline void time::set_year(uint32_t year)
{
	assert(is_valid());
	assert((year >= (dastd_time_YEAR_ZERO+1)) && (year <= dastd_time_YEAR_LAST));
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_year = year;
	compose(decomposed);
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Set the month (1 .. 12)
//------------------------------------------------------------------------------
inline void time::set_month(uint32_t month)
{
	assert(is_valid());
	assert((month >= 1) && (month <= 12));
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_month = month;
	compose(decomposed);
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Set the day (1 .. 31)
//------------------------------------------------------------------------------
inline void time::set_day(uint32_t day)
{
	assert(is_valid());
	assert((day >= 1) && (day <= 31));
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_day = day;
	compose(decomposed);
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Set the hours (0 .. 23)
//------------------------------------------------------------------------------
inline void time::set_hours(uint32_t hours)
{
	assert(is_valid());
	assert(hours <= 23);
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_hours = hours;
	compose(decomposed);
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Set the mins (0 .. 59)
//------------------------------------------------------------------------------
inline void time::set_mins(uint32_t mins)
{
	assert(is_valid());
	assert(mins <= 59);
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_mins = mins;
	compose(decomposed);
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Set the secs (0 .. 59)
//------------------------------------------------------------------------------
inline void time::set_secs(uint32_t secs)
{
	assert(is_valid());
	assert(secs <= 59);
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_secs = secs;
	compose(decomposed);
	assert(is_valid());
}

//------------------------------------------------------------------------------
// Set the microsecs (0 .. 999999)
//------------------------------------------------------------------------------
inline void time::set_microsecs(uint32_t microsecs)
{
	assert(is_valid());
	assert(microsecs <= 999999);
	decomposed_time decomposed;
	decompose(decomposed);
	decomposed.m_microsecs = microsecs;
	compose(decomposed);
	assert(is_valid());
}


} // namespace dastd
