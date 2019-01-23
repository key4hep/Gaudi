// Dear emacs, this is -*- objc -*-

// System import(s):
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <sys/utsname.h>
#import <cxxabi.h>
#import <memory>
#import <regex>

// Local import(s):
#import "SystemMacOS.h"

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

    std::string typeinfoName( const char* class_name ) {

      // Demangle the name:
      int status;
      auto realname = std::unique_ptr<char, decltype( free )*>(
        abi::__cxa_demangle( class_name, nullptr, nullptr, &status ), std::free );
      if ( !realname ) return class_name;

      // Substitute the full type of std::string with "std::string"
      static const std::regex cxx11_string{
       "std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >( (?=>))?"};
      std::string result = std::regex_replace( realname.get(), cxx11_string, "std::string" );

      // Substitute ', ' with ','
      static const std::regex comma_space{", "};
      result = std::regex_replace( result, comma_space, "," );

      return result;
    }

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
