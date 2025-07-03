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

#include "SystemWin32.h"
#include <cstring>
#include <sstream>
#include <windows.h>

// Disable warning C4996 triggered by C standard library calls
#pragma warning( disable : 4996 )

namespace System {
  namespace Win32 {

    std::vector<std::string> cmdLineArgs() {

      // The result object:
      std::vector<std::string> result;

      // For compatibility with UNIX we CANNOT use strtok!
      // If we would use strtok, options like -g="My world" at
      // the command line level would result on NT in TWO options
      // instead in one as in UNIX.
      char  exe[1024];
      char *next, *tmp1, *tmp2;
      for ( LPTSTR cmd = ::GetCommandLine(); *cmd; cmd = next ) {
        ::memset( exe, 0, sizeof( exe ) );
        while ( *cmd == ' ' ) cmd++;
        next = ::strchr( cmd, ' ' );
        if ( !next ) next = cmd + ::strlen( cmd );
        if ( ( tmp1 = ::strchr( cmd, '\"' ) ) > 0 && tmp1 < next ) {
          tmp2 = ::strchr( ++tmp1, '\"' );
          if ( tmp2 > 0 ) {
            next = ++tmp2;
            if ( cmd < tmp1 ) ::strncpy( exe, cmd, tmp1 - cmd - 1 );
            ::strncpy( &exe[strlen( exe )], tmp1, tmp2 - tmp1 - 1 );
          } else {
            std::cout << "Mismatched \" in command line arguments" << std::endl;
            return std::vector<std::string>();
          }
        } else {
          ::strncpy( exe, cmd, next - cmd );
        }
        result.push_back( exe );
      }

      return result;
    }

    std::string typeinfoName( const char* class_name ) {

      // The result variable:
      std::string result;

      long off = 0;
      if ( ::strncmp( class_name, "class ", 6 ) == 0 ) {
        // The returned name is prefixed with "class "
        off = 6;
      }
      if ( ::strncmp( class_name, "struct ", 7 ) == 0 ) {
        // The returned name is prefixed with "struct "
        off = 7;
      }
      if ( off > 0 ) {
        std::string tmp = class_name + off;
        long        loc = 0;
        while ( ( loc = tmp.find( "class " ) ) > 0 ) { tmp.erase( loc, 6 ); }
        loc = 0;
        while ( ( loc = tmp.find( "struct " ) ) > 0 ) { tmp.erase( loc, 7 ); }
        result = tmp;
      } else {
        result = class_name;
      }
      // Change any " *" to "*"
      while ( ( off = result.find( " *" ) ) != std::string::npos ) { result.replace( off, 2, "*" ); }
      // Change any " &" to "&"
      while ( ( off = result.find( " &" ) ) != std::string::npos ) { result.replace( off, 2, "&" ); }

      return result;
    }

    std::string hostName() {

      static const size_t STRING_SIZE = 512;
      char                hname[STRING_SIZE];
      size_t              strlen = STRING_SIZE;
      if ( !::GetComputerName( hname, &strlen ) ) { return "UNKNOWN"; }
      return std::string( hname );
    }

    std::string osName() { return "Windows"; }

    std::string osVersion() {

      OSVERSIONINFO ut;
      ut.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
      if ( !::GetVersionEx( &ut ) ) { return "UNKNOWN"; }
      std::ostringstream ver;
      ver << ut.dwMajorVersion << '.' << ut.dwMinorVersion;
      return ver.str();
    }

    std::string machineType() {

      SYSTEM_INFO ut;
      ::GetSystemInfo( &ut );
      std::ostringstream arch;
      arch << ut.wProcessorArchitecture;
      return arch.str();
    }

    std::string accountName() {

      static const size_t STRING_SIZE = 512;
      char                uname[STRING_SIZE];
      size_t              strlen = STRING_SIZE;
      if ( !::GetUserName( uname, &strlen ) ) { return "UNKNOWN"; }
      return std::string( uname );
    }

  } // namespace Win32
} // namespace System
