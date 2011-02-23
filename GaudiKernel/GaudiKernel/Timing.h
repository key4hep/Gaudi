// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/Timing.h,v 1.2 2005/07/18 16:34:05 hmd Exp $
//====================================================================
//	Timing.h
//--------------------------------------------------------------------
//
//	Package    : Gaudi/System (The LHCb System service)
//
//  Description: Definition of Systems internals
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    :
//====================================================================
#ifndef GAUDIKERNEL_TIMING_H
#define GAUDIKERNEL_TIMING_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/SystemBase.h"

/** Note: OS specific details for process timing

    Entrypoints:
      - remainingTime returns the time the process could still execute
      - ellapsedTime: returns ellapsed time since program start
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
namespace System  {
  /// Time type for conversion
  enum TimeType   { Year, Month, Day, Hour, Min, Sec, milliSec, microSec, nanoSec };
  /// Convert time from OS native time to requested representation (Experts only)
  GAUDI_API longlong adjustTime(TimeType typ, longlong timevalue);
  /** Ellapsed time since start of process in milli seconds.
      @param typ       Indicator or the unit the time will be returned.
      @param timevalue Time value to be converted.
      @return          Requested value in the indicated units.
  */
  GAUDI_API longlong ellapsedTime(TimeType typ = milliSec, InfoType fetch = Times, long pid = -1);
  /** CPU kernel mode time of process in milli seconds.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong kernelTime(TimeType typ = milliSec, InfoType fetch = Times, long pid = -1);
  /** CPU user mode time of process in milli seconds.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong userTime(TimeType typ = milliSec, InfoType fetch = Times, long pid = -1);
  /** Consumed CPU time of process in milli seconds.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong cpuTime(TimeType typ = milliSec, InfoType fetch = Times, long pid = -1);
  /** Maximum processing time left for this process.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong remainingTime(TimeType typ = milliSec, InfoType fetch = Quota, long pid = -1);
  /** Process Creation time.
      @param typ     Indicator or the unit the time will be returned.
      @param fetch   Indicator of the information to be fetched.
                     If Fetch_None, the information will not be updated.
      @param pid     Process ID of which the information will be returned
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong creationTime(TimeType typ = milliSec, InfoType fetch = Times, long pid = -1);
  /** Maximum processing time left for this process.
      @param typ     Indicator or the unit the time will be returned.
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong systemStart(TimeType typ = Sec);
  /** Maximum processing time left for this process.
      @param typ     Indicator or the unit the time will be returned.
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong upTime(TimeType typ = Hour);
  /** Retrieve absolute system time
      @param typ     Indicator or the unit the time will be returned.
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong currentTime(TimeType typ = milliSec);
  /** Retrieve the number of ticks since system startup
      @return        Requested value in the indicated units.
  */
  GAUDI_API longlong tickCount();
}
#endif    // GAUDIKERNEL_TIMING_H
