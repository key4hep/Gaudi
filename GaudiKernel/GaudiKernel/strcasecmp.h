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
#ifndef GAUDIKERNEL_STRCASECMP_H
#define GAUDIKERNEL_STRCASECMP_H

#ifdef _WIN32
#  include <cstring>
inline int strcasecmp( const char* s1, const char* s2 ) { return ::_stricmp( s1, s2 ); }
inline int strncasecmp( const char* s1, const char* s2, size_t n ) { return ::_strnicmp( s1, s2, n ); }
#else // Unix uses string.h
#  include <string.h>
#endif

#endif // GAUDIKERNEL_STRCASECMP_H
