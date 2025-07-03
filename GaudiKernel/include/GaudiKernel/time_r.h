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

#include <ctime>

/*
  This header file provides the functions localtime_r and time_r (available on Linux)
  to the Win32 platform.

  Marco Clemencic
*/

#ifdef _WIN32

extern "C" {
inline struct tm* localtime_r( const time_t* sec, struct tm* result ) {
  localtime_s( result, sec );
  return result;
}
inline struct tm* gmtime_r( const time_t* sec, struct tm* result ) {
  gmtime_s( result, sec );
  return result;
}
}

#endif
