/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/StreamBuffer.h>

/** @class TimeException Time.h GaudiKernel/Time.h
 *
 *  Exception thrown by Gaudi::Time.
 *
 *  @see GaudiException
 *  @see Gaudi::Time
 *
 *  @author Marco Clemencic
 *  @date   2005-12-14
 */
struct GAUDI_API TimeException : GaudiException {
  // Standard constructor
  TimeException( std::string Message = "unspecified exception", std::string Tag = "*Gaudi::Time*",
                 StatusCode Code = StatusCode::FAILURE )
      : GaudiException( std::move( Message ), std::move( Tag ), std::move( Code ) ) {}
};

struct tm;
#ifdef WIN32
typedef struct _FILETIME FILETIME;
#endif

namespace Gaudi {

  class Time;
  class TimeSpan;

  /** @class TimeSpan Time.h GaudiKernel/Time.h
   *
   *  Based on seal::TimeSpan.
   *

   A difference between two #Time values.  In addition to supporting
   normal integer artihmetic and comparisons, the span can also be
   converted to a number useful units.

   @sa #Time.

   *  (Documentation taken from original SEAL class)
   *  @author Marco Clemencic
   *  @date   2005-12-15
   */
  class GAUDI_API TimeSpan {
    friend class Time;

  public:
    typedef long long ValueType;

    /** Initialize an empty (zero) time difference.  */
    TimeSpan() = default;

    TimeSpan( Time t );
    TimeSpan( ValueType nsecs );
    TimeSpan( ValueType secs, int nsecs );
    TimeSpan( int days, int hours, int mins, int secs, int nsecs );

    int       days() const;
    int       hours() const;
    int       minutes() const;
    ValueType seconds() const;

    int lastHours() const;
    int lastMinutes() const;
    int lastSeconds() const;
    int lastNSeconds() const;

    TimeSpan& operator+=( const TimeSpan& x );
    TimeSpan& operator-=( const TimeSpan& x );
    TimeSpan& operator*=( const TimeSpan& n );
    TimeSpan& operator/=( const TimeSpan& n );
    TimeSpan& operator%=( const TimeSpan& n );

    ValueType ns() const;

    friend bool operator==( const Gaudi::TimeSpan& t1, const Gaudi::TimeSpan& t2 ) { return t1.ns() == t2.ns(); }

    friend bool operator!=( const Gaudi::TimeSpan& t1, const Gaudi::TimeSpan& t2 ) { return t1.ns() != t2.ns(); }

    friend bool operator<( const Gaudi::TimeSpan& t1, const Gaudi::TimeSpan& t2 ) { return t1.ns() < t2.ns(); }

    friend bool operator<=( const Gaudi::TimeSpan& t1, const Gaudi::TimeSpan& t2 ) { return t1.ns() <= t2.ns(); }

    friend bool operator>( const Gaudi::TimeSpan& t1, const Gaudi::TimeSpan& t2 ) { return t1.ns() > t2.ns(); }

    friend bool operator>=( const Gaudi::TimeSpan& t1, const Gaudi::TimeSpan& t2 ) { return t1.ns() >= t2.ns(); }

    friend Gaudi::TimeSpan operator+( const Gaudi::TimeSpan& ts1, const Gaudi::TimeSpan& ts2 ) {
      return Gaudi::TimeSpan( ts1.ns() + ts2.ns() );
    }

    friend Gaudi::TimeSpan operator-( const Gaudi::TimeSpan& ts1, const Gaudi::TimeSpan& ts2 ) {
      return Gaudi::TimeSpan( ts1.ns() - ts2.ns() );
    }

  private:
    ValueType m_nsecs = 0; //< The span length.
  };

  /** @class Time Time.h GaudiKernel/Time.h
   *
   *  Based on seal::Time.
   *
   Calendar time in nanoseconds since 00:00:00 on January 1, 1970,
   Coordinated Universal Time (UTC).

   #Time is represented internally as UTC time, but it can also be
   converted to the local time as necessary.  Most methods take an
   argument flag @c local to indicate which time interpretation is
   desired by the client, and automatically perform the necessary
   adjustments.  The client can also find out about the difference
   between UTC time and local time using the #utcoffset() method,
   and the time zone name with #timezone() method.  Both allow the
   client to discover whether daylight savings is in effect.

   The native representation of #Time is not well suited for human
   handling of time.  #Time provides access in more convenient terms
   such as #year(), #month() and #day(); more are available through
   conversion into a #TimeSpan.  #Time can also be converted to and
   from ISO C standard @c tm structure.  Note however that unlike C's
   @c mktime() which always assumes @c tm in local time, #Time fully
   supports all conversions between local and universal time.  Thus
   it is possible for example to #build() a UTC time directly from a
   @c tm.

   #Time behaves as an integral type.  Differences in time values are
   represented as a #TimeSpan.  Usual integral arithmetic works with
   both types.  Output works in general as any other integral type,
   however since the #ValueType can be a wide type, it may be poorly
   supported by the @c iostream; if so, including the @c LongLong.h
   header will help.  Note that the output value will usually be very
   large as #Time is represented in nanoseconds, not seconds!  When
   constructing #Time values in seconds, such as when reading in, do
   remember to use the two-argument constructor taking seconds and
   nanoseconds instead of the default single-argument one.

   #Time can be formatted into a string using the #format() method,
   which uses the versatile @c strftime() function.  Since the latter
   works on seconds at best (through a struct @c tm), the subsecond
   part cannot be formatted; the #nanoformat() method is provided to
   overcome this limitation.  To combine #format() and #nanoformat()
   output use a suitable #StringFormat pattern.

   #Time is linked to the system's concept of calendar time and is
   therefore may not be linear nor monotonic.  System time can jump
   arbitrarily in either direction as real time clock is corrected or
   the system is suspended.  The local time may also jump due to
   daylight savings.  The process' ability to sample system time can
   be limited for reasons such as getting swapped out.  #TimeInfo
   provides an alternative time measurement facility not linked to
   calendar and guaranteed to grow monotonically -- though not always
   linearly.  Note that few systems actually provide wall-clock time
   in nanosecond resolution.  Not all system provide an interface to
   get time at that resolution, let alone track it so precisely.

   Because of the time warp issues, scheduling events using #Time is
   not straightforward.  Application code should understand whether
   it is dealing with concrete or abstract calendar calculations, and
   how the events it schedules are linked to wall clock time.

   For calculations on concrete calendar as perceived by people use
   #local() after plain #Time and #TimeSpan integer arithmetic.  The
   method accounts for timezone and daylight savings definitions.  To
   schedule events use #build() to derive times from #local() time to
   get values comparable to the system time returned by #current().
   The applications should know whether events are scheduled in UTC
   or local time---"meeting at 9:00 on Wednesday morning" when the
   device switches timezones may be known to be at 9:00 in the new
   timezone (= locked to local time), or in the timezone where the
   event was created (= locked to UTC).  The #build() and #split()
   methods allow either format to be used, the application just needs
   to know which one to use.  It is also easy to convert between the
   two using #utcoffset().

   For calculations using an abstract calendar, without timezone or
   daylight savings, use #Time in its native UTC representation and
   integer arithmetic with #Time and #TimeSpan.  Do note however that
   "T + 24 hours" may not be the same hour the next day in the local
   calendar time -- timezone changes and daylight savings make a
   difference.  This may require the application to accept as user
   input exception rules to its usual calendar calculations.

   To schedule events, one should choose between three choices: UTC
   time, local time, or delta time.  For the first two cases system
   time should be polled regularly to see if any of the recorded
   events have expired.  It is not a good idea to sleep until the
   next scheduled event, as the system time may jump during the nap;
   instead sleep small increments, recheck the current time after
   each nap and trigger the events that have expired.  A policy must
   be applied when the system time warps; this can happen both
   forwards and backwards with both local and UTC time (daylight
   savings or timezone changes for mobile devices are common local
   time change reasons, but the system time can be updated for any
   reason, e.g. when the real time clock is wrong, or if the system
   is suspended for a long time).  Some events should be executed
   only once in case of time warps backwards.  If the time jumps
   forwards, several events may need to be dealt with in one go.  In
   either case the application should guard against major time
   changes: long system suspends, moving mobile devices and major
   time updates may result in a large number of "missed" events.  One
   possibility is to provide a user-configurable "excessive time
   drift limit" (e.g. N hours): if time changes by more than that,
   missed events are not triggered.

   For the final case of using delta times, sort upcoming events by
   their deltas from the previous event---not by the time they are
   anticipated to occur.  Capture current time before and after the
   sleep and pull events off the queue based on the difference (the
   sleep time may exceed the requested time).  Either guard against
   long time warps like suspends or schedule timer events cautiously.
   Using #TimeInfo as schedule base solves such issues simply.  To
   cope with backward system time jumps when using #Time as schedule
   base, assume that sleeps always last at least the requested time;
   if the time delta over the nap is less than the requested, assume
   time warp (this is not foolproof against interrupted system calls
   but works for many event scheduling situations).

   @sa #TimeInfo for monotonic time not related to the calendar.
   *  (Documentation taken from original SEAL class)
   *  @author Marco Clemencic
   *  @date   2005-12-15
   */
  class GAUDI_API Time {
    friend class TimeSpan;

  public:
    typedef long long ValueType;

    /** Symbolic names for months */
    enum Months {
      January   = 0,
      February  = 1,
      March     = 2,
      April     = 3,
      May       = 4,
      June      = 5,
      July      = 6,
      August    = 7,
      September = 8,
      October   = 9,
      November  = 10,
      December  = 11
    };

    /** Seconds in 24 hours.  */
    static const int SECS_PER_DAY = 86400;

    /** Seconds in one hour hour.  */
    static const int SECS_PER_HOUR = 3600;

    /** Nanoseconds in one second.  */
    static const ValueType SEC_NSECS = 1000000000;

    /** Initialize an empty (zero) time value.  */
    Time() = default;

    Time( TimeSpan ts );
    Time( ValueType nsecs );
    Time( ValueType secs, int nsecs );
    Time( int year, int month, int day, int hour, int min, int sec, ValueType nsecs, bool local = true );
    // implicit copy constructor
    // implicit assignment operator
    // implicit destructor

    /// Returns the minimum time.
    static Time epoch();
    /// Returns the maximum time.
    static Time max();
    /// Returns the current time.
    static Time current();
#ifdef WIN32
    static Time from( const FILETIME* systime );
#endif
    static Time build( bool local, const tm& base, TimeSpan diff = 0 );

    tm split( bool local, int* nsecpart = 0 ) const;
    tm utc( int* nsecpart = 0 ) const;
    tm local( int* nsecpart = 0 ) const;

    int  year( bool local ) const;
    int  month( bool local ) const;
    int  day( bool local ) const;
    int  hour( bool local ) const;
    int  minute( bool local ) const;
    int  second( bool local ) const;
    int  nsecond() const;
    int  weekday( bool local ) const;
    bool isdst( bool local ) const;

    ValueType   utcoffset( int* daylight = 0 ) const;
    const char* timezone( int* daylight = 0 ) const;

    Time& operator+=( const TimeSpan& x );
    Time& operator-=( const TimeSpan& x );

    ValueType ns() const;

    std::string format( bool local, std::string spec = "%c" ) const;
    std::string nanoformat( size_t minwidth = 1, size_t maxwidth = 9 ) const;

    static bool isLeap( int year );

    // Conversion helpers
    static unsigned toDosDate( Time time );
    static Time     fromDosDate( unsigned dosDate );

    friend bool operator==( const Gaudi::Time& t1, const Gaudi::Time& t2 ) { return t1.ns() == t2.ns(); }

    friend bool operator!=( const Gaudi::Time& t1, const Gaudi::Time& t2 ) { return t1.ns() != t2.ns(); }

    friend bool operator<( const Gaudi::Time& t1, const Gaudi::Time& t2 ) { return t1.ns() < t2.ns(); }

    friend bool operator<=( const Gaudi::Time& t1, const Gaudi::Time& t2 ) { return t1.ns() <= t2.ns(); }

    friend bool operator>( const Gaudi::Time& t1, const Gaudi::Time& t2 ) { return t1.ns() > t2.ns(); }

    friend bool operator>=( const Gaudi::Time& t1, const Gaudi::Time& t2 ) { return t1.ns() >= t2.ns(); }

  private:
    ValueType m_nsecs = 0; //< Time value as nsecs from #epoch().

    // Taking string_view means there will never be any dynamic allocation if cond == true
    inline void TimeAssert( bool cond, std::string_view msg = "time assertion failed" ) const {
      if ( !cond ) throw TimeException( std::string{ msg } );
    }
  };
} // namespace Gaudi

#include <GaudiKernel/Time.icpp>
