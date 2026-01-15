/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "SystemLinux.h"
#include "TypeNormalization.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace System {
  namespace Linux {

    std::vector<std::string> cmdLineArgs() {

      // Open the file that we can get this information from:
      char fname[1024];
      ::snprintf( fname, sizeof( fname ), "/proc/%d/cmdline", ::getpid() );
      FILE* cmdLine = ::fopen( fname, "r" );

      // The result object:
      std::vector<std::string> result;

      // Read the command line arguments from there:
      char cmd[1024];
      if ( cmdLine ) {
        long len = ::fread( cmd, sizeof( char ), sizeof( cmd ), cmdLine );
        if ( len > 0 ) {
          cmd[len] = 0;
          for ( char* token = cmd; token - cmd < len; token += ::strlen( token ) + 1 ) { result.push_back( token ); }
        }
        ::fclose( cmdLine );
      }

      return result;
    }

    std::string typeinfoName( const char* class_name ) { return Detail::normalizeTypeName( class_name ); }

    std::string hostName() {

      static const size_t           STRING_SIZE = 512;
      std::array<char, STRING_SIZE> hname;
      if ( ::gethostname( hname.data(), STRING_SIZE ) ) { return ""; }

      // According to the gethostname documentation, if a host name is too long
      // to fit into the array provided by the user, the call will truncate the
      // name to fit into the array, without adding a terminating null
      // character at the end, and will not signal an error to the caller.
      // While SUSv2 guarantees that "Host names are limited to 255 bytes", and
      // the limit to host names is in practice 64 characters on Linux, just to
      // be safe, the last character of the returned array is set to null
      // forecfully.
      hname.back() = '\0';

      return std::string( hname.data() );
    }

    std::string osName() {

      struct utsname ut;
      if ( ::uname( &ut ) ) { return "UNKNOWN Linux"; }
      return std::string( ut.sysname );
    }

    std::string osVersion() {

      struct utsname ut;
      if ( ::uname( &ut ) ) { return "UNKNOWN version"; }
      return std::string( ut.release );
    }

    std::string machineType() {

      struct utsname ut;
      if ( ::uname( &ut ) ) { return "UNKNOWN"; }
      return std::string( ut.machine );
    }

    std::string accountName() {

      const char* acct = ::getlogin();
      if ( !acct ) acct = ::getenv( "LOGNAME" );
      if ( !acct ) acct = ::getenv( "USER" );

      return ( acct ? acct : "UNKNOWN" );
    }

  } // namespace Linux
} // namespace System
