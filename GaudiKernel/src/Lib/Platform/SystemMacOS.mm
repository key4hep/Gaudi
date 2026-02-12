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
// Dear emacs, this is -*- objc -*-

// System import(s):
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <sys/utsname.h>

// Local import(s):
#import "SystemMacOS.h"
#import "TypeNormalization.h"

namespace System
{
  namespace MacOS
  {

    std::vector<std::string> cmdLineArgs() {

      // Get the process's command line arguments in a very MacOS specific way:
      id args = [ [ NSProcessInfo processInfo ] arguments ];

      // Convert the arguments to a vector<string>:
      std::vector< std::string > result;
      for ( id arg in args ) {
        result.push_back( [ arg cStringUsingEncoding :
                                   [ NSString defaultCStringEncoding ] ] );
      }

      // Return the newly created value:
      return result;
    }

    std::string typeinfoName( const char* class_name ) { return Detail::normalizeTypeName( class_name, true ); }

    std::string hostName() {

      // Get the host name in a very MacOS specific way:
      id hname = [ [ NSProcessInfo processInfo ] hostName ];

      // Translate it into an std::string on return:
      return std::string( [ hname cStringUsingEncoding :
                                     [ NSString defaultCStringEncoding ] ] );
    }

    std::string osName() {

      return "MacOS X";
    }

    std::string osVersion() {

      // Get the operating system's version in a very MacOS specific way:
      id oname = [ [ NSProcessInfo processInfo ] operatingSystemVersionString ];

      // Translate it into an std::string on return:
      return std::string( [ oname cStringUsingEncoding :
                                     [ NSString defaultCStringEncoding ] ] );
    }

    std::string machineType() {

      struct utsname ut;
      if ( ::uname( &ut ) ) {
        return "UNKNOWN";
      }
      return std::string( ut.machine );
    }

    std::string accountName() {

      // Get the user's account name in a very MacOS specific way:
      id uname = [ [ NSProcessInfo processInfo ] userName ];

      // Translate it into an std::string on return:
      return std::string( [ uname cStringUsingEncoding :
                                     [ NSString defaultCStringEncoding ] ] );
    }

  } // namespace MacOS
} // namespace System
