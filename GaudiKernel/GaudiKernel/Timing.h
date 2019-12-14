/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	Timing.h
//--------------------------------------------------------------------
//
//	Package    : Gaudi/System (The LHCb System service)
//
//  Description: Definition of Systems internals
//
//	Author     : M.Frank
//      Created    : 13/1/99
//====================================================================
#ifndef GAUDIKERNEL_TIMING_H
#define GAUDIKERNEL_TIMING_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/SystemBase.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

/** Note: OS specific details for process timing

    Entrypoints:
      - remainingTime returns the time the process could still execute
      - ellapsedTime: returns elapsed time since program start
      - kernelTime:   returns the amount of time the process has spent in kernel mode
      - userTime:     returns the amount of time the process has spent in user mode
      - cpuTime:      returns kernel+user time


      On Windows NT Time is expressed as
      the amount of time that has elapsed since midnight on
      January 1, 1601 at Greenwich, England.

      On Unix time is expressed as
      the amount of time that has elapsed since midnight on
      January 1, 1970 at Greenwich, England.

    <P> History    :

    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 11/11/00| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    @author:  M.Frank
    @version: 1.0
*/
namespace System {
  /// Time type for conversion
  enum TimeType { Year, Month, Day, Hour, Min, Sec, milliSec, microSec, nanoSec, Native };

  /// Convert time from OS native time to requested representation (Experts only)
  GAUDI_API long long adjustTime( TimeType typ, long long timevalue );

  /// Convert the time from OS native time to requested representation (Experts only)
  template <TimeType T>
  inline long long adjustTime( long long timevalue );

  /** Elapsed time since start of process in milliseconds.
        @param typ       Indicator or the unit the time will be returned.
        @param timevalue Time value to be converted.
        @return          Requested value in the indicated units.
    */
  GAUDI_API long long ellapsedTime( TimeType typ = milliSec, InfoType fetch = Times, long pid = -1 );
  /** CPU kernel mode time of process in milliseconds.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long kernelTime( TimeType typ = milliSec, InfoType fetch = Times, long pid = -1 );
  /** CPU user mode time of process in milliseconds.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long userTime( TimeType typ = milliSec, InfoType fetch = Times, long pid = -1 );
  /** Consumed CPU time of process in milliseconds.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long cpuTime( TimeType typ = milliSec, InfoType fetch = Times, long pid = -1 );
  /** Maximum processing time left for this process.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long remainingTime( TimeType typ = milliSec, InfoType fetch = Quota, long pid = -1 );
  /** Process Creation time.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long creationTime( TimeType typ = milliSec, InfoType fetch = Times, long pid = -1 );
  /** Maximum processing time left for this process.
      @param typ     Indicator or the unit the time will be returned.
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long systemStart( TimeType typ = Sec );
  /** Maximum processing time left for this process.
      @param typ     Indicator or the unit the time will be returned.
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long upTime( TimeType typ = Hour );
  /** Retrieve absolute system time
      @param typ     Indicator or the unit the time will be returned.
      @return        Requested value in the indicated units.
  */

  /// Get current time in specificed units via template parameter (inlined)
  template <TimeType T>
  GAUDI_API long long currentTime();

  /// Get current time in specificed units
  GAUDI_API long long currentTime( TimeType typ = milliSec );

  /** Retrieve the number of ticks since system startup
      @return        Requested value in the indicated units.
  */
  GAUDI_API long long tickCount();

  /** Simple class to hold the time information of a process.
   *
   * Simplify the simultaneous handling of kernel, user and elapsed times of a
   * process.
   *
   * \see {<a href="http://savannah.cern.ch/bugs/?87341">bug #87341</a>}
   */
  class ProcessTime {
  public:
    typedef long long TimeValueType;

    /// Constructor
    ProcessTime() : i_kernel( 0 ), i_user( 0 ), i_elapsed( 0 ) {}

    /// Constructor
    ProcessTime( TimeValueType k, TimeValueType u, TimeValueType e ) : i_kernel( k ), i_user( u ), i_elapsed( e ) {}

    /// Retrieve the kernel time in the requested unit.
    template <TimeType T>
    inline TimeValueType kernelTime() const {
      return adjustTime<T>( i_kernel );
    }

    /// Retrieve the user time in the requested unit.
    template <TimeType T>
    inline TimeValueType userTime() const {
      return adjustTime<T>( i_user );
    }

    /// Retrieve the elapsed time in the requested unit.
    template <TimeType T>
    inline TimeValueType elapsedTime() const {
      return adjustTime<T>( i_elapsed );
    }

    /// Retrieve the CPU (user+kernel) time in the requested unit.
    template <TimeType T>
    inline TimeValueType cpuTime() const {
      return adjustTime<T>( i_user + i_kernel );
    }

    /// Return the delta between two \c ProcessTime objects.
    inline ProcessTime operator-( const ProcessTime& rhs ) const {
      return ProcessTime( i_kernel - rhs.i_kernel, i_user - rhs.i_user, i_elapsed - rhs.i_elapsed );
    }
    /// Add the timings to the current objects
    inline ProcessTime& operator+=( const ProcessTime& rhs ) {
      i_kernel += rhs.i_kernel;
      i_user += rhs.i_user;
      i_elapsed += rhs.i_elapsed;
      return *this;
    }

  private:
    /// Internal storage.
    TimeValueType i_kernel, i_user, i_elapsed;
  };

  /** Retrieve the process time data for a process.
   *
   * Get the process time data for a process (by default the current) as a
   * \c ProcessTime object.
   */
  GAUDI_API ProcessTime getProcessTime( long pid = -1 );
} // namespace System

// implementation of the templated functions
namespace System {
  template <>
  inline long long adjustTime<Year>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 1LL * 365 * 24 * 60 * 60 * 1000 * 1000 * 10 );
  }
  template <>
  inline long long adjustTime<Day>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 1LL * 24 * 60 * 60 * 1000 * 1000 * 10 );
  }
  template <>
  inline long long adjustTime<Hour>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 1LL * 60 * 60 * 1000 * 1000 * 10 );
  }
  template <>
  inline long long adjustTime<Min>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 60 * 1000 * 1000 * 10 );
  }
  template <>
  inline long long adjustTime<Sec>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 1000 * 1000 * 10 );
  }
  template <>
  inline long long adjustTime<milliSec>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 1000 * 10 );
  }
  template <>
  inline long long adjustTime<microSec>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 10LL );
  }
  template <>
  inline long long adjustTime<nanoSec>( long long t ) {
    return ( t == -1 ) ? t : t *= 100LL;
  }
  template <>
  inline long long adjustTime<Month>( long long t ) {
    return ( t == -1 ) ? t : t /= ( 1LL * 30 * 24 * 60 * 60 * 1000 * 1000 * 10 );
  }
  template <>
  inline long long adjustTime<Native>( long long t ) {
    return t;
  }

  // This is frequently used and thus we inline it if possible
  template <TimeType T>
  inline long long currentTime() {
#ifdef _WIN32
    long long current = 0;
    ::GetSystemTimeAsFileTime( (FILETIME*)&current );
    return adjustTime<T>( current - UNIX_BASE_TIME );
#else
    struct timeval tv;
    ::gettimeofday( &tv, 0 );
    return adjustTime<T>( ( tv.tv_sec * 1000000 + tv.tv_usec ) * 10 );
#endif
  }

  // Define all template versions here to avoid code bloat
  template long long currentTime<Year>();
  template long long currentTime<Month>();
  template long long currentTime<Day>();
  template long long currentTime<Hour>();
  template long long currentTime<Min>();
  template long long currentTime<Sec>();
  template long long currentTime<milliSec>();
  template long long currentTime<microSec>();
  template long long currentTime<nanoSec>();
  template long long currentTime<Native>();
} // namespace System

#endif // GAUDIKERNEL_TIMING_H
