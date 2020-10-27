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
#if !defined( _WIN32 )
/* It's called _swab(...) on NT, but swab(...) on egcs....
   And in some version it's not even defined
*/
#  include <unistd.h>
//  extern "C" void swab( const char*, char*, size_t);
#  define _swab( source, target, radix ) swab( source, target, radix )
#endif /* WIN32 */
