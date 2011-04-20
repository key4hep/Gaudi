// $Id: Time.cpp,v 1.1 2006/01/26 09:24:03 hmd Exp $
// Include files
#include "GaudiKernel/Time.h"

#include "GaudiKernel/time_r.h"

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <cstring>

// local
using namespace Gaudi;

// architecture dependent includes
#ifdef WIN32

// this turns off a lot of useless Win stuff which conflicts with Gaudi. (found empirically)
#define NOATOM
#define NOGDI
#define NOGDICAPMASKS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NORASTEROPS
#define NOSCROLL
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOCRYPT
#define NOMCX

#include <windows.h>

#else
// Linux

#include <sys/time.h>

#endif

//-----------------------------------------------------------------------------
// Implementation file for class : Gaudi::Time
// Based on seal::Time
// 2005-12-15 : Marco Clemencic
//-----------------------------------------------------------------------------

#ifdef WIN32
/** Seconds between 1601 and 1970, the Windows time base and UNIX time
    base.  1 January 1601, 00:00:00 to 1 January 1970, 00:00:00 is 369
    years.  To this we add the leap years from 1604 to 1968, excluding
    1700, 1800, 1900.  This makes (1968 - 1600) / 4 - 3 = 89 leap
    days, and a total of 134774 days.  Each day in the period had 24 *
    60 * 60 = 86400 seconds (no leap seconds).  Thus the difference is
    in total 134774 * 86400 = 11644473600 seconds.  For the actual
    bias this needs to be multiplied by 10000000 since Windows time
    uses 100ns resolution, not seconds.  */
//# define SECS_1601_TO_1970	((369 * 365 + 89) * SECS_PER_DAY)
# define SECS_1601_TO_1970	((369 * 365 + 89) * 86400ui64)
#endif

#ifdef WIN32
static time_t timegm (struct tm *t) {
  // This code is adapted from wine, samba
  time_t t1 = mktime (t);
  struct tm gmt;
  gmtime_s(&gmt, &t1);
  time_t t2 = mktime(&gmt);
  return t1 + (t1 - t2);
}
#endif

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
Time::Time( int year, int month, int day,
            int hour, int min, int sec,
            ValueType nsecs,
            bool local /* = true */ ) {
  tm val;
  memset (&val, 0, sizeof (val));
  val.tm_sec = sec;
  val.tm_min = min;
  val.tm_hour = hour;
  val.tm_mday = day;
  val.tm_mon = month;
  val.tm_year = year > 1900 ? year - 1900 : year;
  val.tm_isdst = -1; // FIXME?

  m_nsecs = build (local, val, nsecs).m_nsecs;
}

#ifdef WIN32
/** Windows-specific function to convert system time representation
    to a #Time class.  */
Time Time::from (const FILETIME *systime) {
  ValueType t = ((ValueType) systime->dwHighDateTime << 32)
    + (ValueType) systime->dwLowDateTime;

  if (t)
    // Subtract bias (1970--1601 in 100ns), then convert to nanoseconds.
    t = (t - SECS_1601_TO_1970 * (SEC_NSECS/100)) * 100;

  return Time (t);
}
#endif

/** Return the current system time.  */
Time Time::current (void) {
#ifdef WIN32
  FILETIME ftime;
  GetSystemTimeAsFileTime (&ftime);
  return from (&ftime);
#else
  timeval tv;
  if (gettimeofday (&tv, 0) != 0) {
    char buf[256];
    std::ostringstream tag,msg;
    tag << "errno=" << errno;
    if( strerror_r(errno, buf, 256) == 0 ) {
      msg << buf;
    } else {
      msg << "Unknown error retrieving current time";
    }
    throw GaudiException(msg.str(),tag.str(),StatusCode::FAILURE);
  }
  return Time (tv.tv_sec, tv.tv_usec * 1000);
#endif
}

/** Construct a time from local time @a base and a delta @a diff. */
Time Time::build (bool local, const tm &base, TimeSpan diff /* = 0 */) {
  tm tmp (base);
  return Time (local ? mktime(&tmp) : timegm(&tmp), 0) + diff;
}

/** Break up the time to the standard representation, either in UTC
    (if @a local is @c false) or local time (if @a local is @c true).
    If @a nsecpart is non-null, it is set to the nanosecond part that
    cannot be stored into @c tm.  */
tm Time::split (bool local, int *nsecpart /* = 0 */) const {
  if (nsecpart)
    *nsecpart = (int)(m_nsecs % SEC_NSECS);

  time_t val = (time_t)(m_nsecs / SEC_NSECS);

  tm retval;
  if (local)
	localtime_r(&val, &retval);
  else
	gmtime_r(&val, &retval);

  return retval;
}

/** Break up the time to the standard library representation, keeping
    it in UTC.  If @a nsecpart is non-null, it is set to the
    nanosecond part that cannot be stored into @c tm.  */
tm Time::utc (int *nsecpart /* = 0 */) const {
  return split (false, nsecpart);
}

/** Break up the time to the standard library representation,
    converting it first to local time.  If @a nsecpart is non-null, it
    is set to the nanosecond part that cannot be stored into @c tm.  */
tm Time::local (int *nsecpart /* = 0 */) const {
  return split (true, nsecpart);
}

/** Get the year.  */
int Time::year (bool local) const {
  return split (local).tm_year + 1900;
}

/** Get the month, numbered [0,11].  */
int Time::month (bool local) const {
  return split (local).tm_mon;
}

/** Get the day of month, numbered [1,31].  */
int Time::day (bool local) const {
  return split (local).tm_mday;
}

/** Get the hour, numbered [0, 23].  */
int Time::hour (bool local) const {
  return split (local).tm_hour;
}

/** Get the minute, numbered [0, 59].  */
int Time::minute (bool local) const {
  return split (local).tm_min;
}

/** Get the seconds, numbered [0,61] (allowing one or two leap
    seconds, years with leap seconds can have the time Dec 31,
    23:59:60 (or :61).)  */
int Time::second (bool local) const {
  return split (local).tm_sec;
}

/** Get the nanoseconds.  There is no @c local argument since
    time zone and daylight savings never affects the value at
    the subsecond granularity. */
int Time::nsecond (void) const {
  return (int)(m_nsecs % SEC_NSECS);
}

/** Get the day of week, numbered [0,6] and starting from Sunday.  */
int Time::weekday (bool local) const {
  return split (local).tm_wday;
}

/** Check whether daylight savings is in effect.  This really only
    makes sense if @a local is true since daylight savings is never
    in effect for UTC time.  */
bool Time::isdst (bool local) const {
  return split (local).tm_isdst > 0;
}

/** Return the number of nanoseconds that needs to be added to UTC to
    translate this time to the local time (= nanoseconds east of
    UTC).  This accounts for the time zone and daylight savings
    settings of the local time as of the current value.  If
    @a daylight is non-null, it is set to indicate daylight savings
    status (that is, tm.tm_isdst for the effective local time).  */
Time::ValueType Time::utcoffset (int *daylight /* = 0 */) const {
  ValueType n = 0;

#ifndef WIN32
  tm localtm = local ();
  n = localtm.tm_gmtoff;
  if (daylight) *daylight = localtm.tm_isdst;
#else
  // Adapted from WINE.
  time_t	utctime = (time_t)(m_nsecs / SEC_NSECS);
  tm		localtm;
  localtime_s(&localtm, &utctime);
  int		savedaylight = localtm.tm_isdst;
  tm		gmt;
  gmtime_s(&gmt, &utctime);

  gmt.tm_isdst = savedaylight;
  n = utctime - mktime (&gmt);

  if (daylight) *daylight = savedaylight;
#endif
  return n * SEC_NSECS;
}

#ifdef WIN32
// disable warning
// C4996: 'tzname': This function or variable may be unsafe.
#pragma warning(push)
#pragma warning(disable:4996)
#endif
/** Return the local timezone name that applies at this time value.
    On some platforms returns the most recent timezone name (dst or
    non-dst one depending on the time value), not the one that applies
    at the time value.  */
const char * Time::timezone (int *daylight /* = 0 */) const {
  tm localtm = local ();
  if (daylight) *daylight = localtm.tm_isdst;
  // extern "C" { extern char *tzname [2]; }
  return tzname [localtm.tm_isdst > 0 ? 1 : 0];
}
#ifdef WIN32
#pragma warning(pop)
#endif

/** Format the time using @c strftime.
 *  The additional conversion specifier %f can be used to display
 *  milliseconds (extension for compatibility with MessageSvc time format).
 */
std::string Time::format (bool local, std::string spec) const {
  /// @FIXME: This doesn't account for nsecs part!
  std::string            result;
  tm                     time = split (local);
  std::string::size_type length = 0;

  // handle the special case of "%f"
  std::string::size_type pos = spec.find("%f");
  if (std::string::npos != pos) {
    // Get the milliseconds string
    std::string ms = nanoformat(3,3);
    // Replace all the occurrences of '%f' (if not preceded by '%')
    while (std::string::npos != pos) {
      if (pos != 0 && spec[pos-1] != '%') {
        spec.replace(pos, 2, ms);
      }
      pos = spec.find("%f", pos + 1); // search for the next occurrence
    }
  }
  const int MIN_BUF_SIZE = 128;
  do
  {
    // Guess how much we'll expand.  If we go wrong, we'll expand again. (with a minimum)
    result.resize(std::max<std::string::size_type>(result.size()*2,
                    std::max<std::string::size_type>(spec.size()*2, MIN_BUF_SIZE))
                  , 0);
    length = ::strftime (&result[0], result.size(), spec.c_str(), &time);
  } while (! length);

  result.resize (length);
  return result;
}

/** Format the nanosecond fractional part of the time as a string.
    The arguments control the representation of the resulting value.
    The nanosecond part is printed as fixed nine-character-wide number
    and then excess zeroes are stripped off at the right end.  Use @a
    minwidth to force a specific number number of them to be left
    intact: the resulting number will have at least that many digits.
    Use @a maxwidth to truncate the value: the resulting number will
    have at most that many digits.  Both @a minwidth and @a maxwidth
    must be between one and nine inclusive and @a minwidth must be
    less or equal to @a maxwidth.  */
std::string Time::nanoformat (size_t minwidth /* = 1 */, size_t maxwidth /* = 9 */) const {
  TimeAssert( (minwidth >= 1) && (minwidth <= maxwidth) && (maxwidth <= 9),
              "nanoformat options do not satisfy: 1 <= minwidth <= maxwidth <= 9");

  // Calculate the nanosecond fraction.  This will be < 1000000000.
  int value = (int)(m_nsecs % SEC_NSECS);

  std::ostringstream buf;
  buf.fill('0');
  buf.width(9);
  buf << value;
  std::string out = buf.str();
  // Find the last non-0 char before maxwidth, but after minwidth
  // (Note: -1 and +1 are to account for difference between position and size.
  //        moreover, npos + 1 == 0, so it is correct to say that 'not found' means size of 0)
  size_t len = out.find_last_not_of('0', maxwidth - 1) + 1;
  // Truncate the output string to at least minwidth chars
  out.resize(std::max(len, minwidth));
  return out;
}

//////////////////////////////////////////////////////////////////////
/** Convert the #Time @a t into a MS-DOS date format.  */
unsigned Time::toDosDate (Time time) {
  // Use local time since DOS does too.
  struct tm localtm = time.local ();

  unsigned mday = localtm.tm_mday;
  unsigned mon  = localtm.tm_mon + 1;
  unsigned year = (localtm.tm_year > 80 ? localtm.tm_year - 80 : 0);
  unsigned sec  = localtm.tm_sec / 2;
  unsigned min  = localtm.tm_min;
  unsigned hour = localtm.tm_hour;
  return (mday << 16 | mon << 21 | year << 25
          | sec      | min << 5  | hour << 11);
}

/** Convert the MS-DOS date @a dosDate into a #Time.  */
Time Time::fromDosDate (unsigned dosDate) {
  // DOS times are generally local; treat it as UTC.  This avoids
  // any round-trip conversion and leaves only a presentation as an
  // issue.  Since not much can be known about the origin of the DOS
  // times, it's generally best to present them as such (= in UTC).
  struct tm localtm;
  memset (&localtm, 0, sizeof (localtm));
  localtm.tm_mday  = (dosDate >> 16) & 0x1f;
  localtm.tm_mon   = ((dosDate >> 21) & 0xf) - 1;
  localtm.tm_year  = ((dosDate >> 25) & 0x7f) + 80;
  localtm.tm_hour  = (dosDate >> 11) & 0x1f;
  localtm.tm_min   = (dosDate >> 5) & 0x3f;
  localtm.tm_sec   = (dosDate & 0x1f) * 2;
  localtm.tm_isdst = -1;

  return Time (mktime (&localtm), 0);
}

//=============================================================================
