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
// Dear emacs, this is -*- c++ -*-
#pragma once

#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#include <regex>
#include <string>

namespace System {
  namespace Detail {

    /// Normalize a demangled C++ type name for cross-platform consistency.
    /// Handles differences between libstdc++ and libc++:
    /// - Removes std::__cxx11:: and std::__1:: inline namespace prefixes
    /// - Converts basic_string<char,...> to std::string
    /// - Normalizes ">>" to "> >" (C++03 style spacing)
    /// @param normalize_commas If true, also converts ", " to "," (legacy behavior)
    inline std::string normalizeTypeName( const char* mangled_name, bool normalize_commas = false ) {
      int  status;
      auto realname = std::unique_ptr<char, decltype( free )*>(
          abi::__cxa_demangle( mangled_name, nullptr, nullptr, &status ), std::free );
      if ( !realname ) return mangled_name;

      std::string result = realname.get();

      // Normalize libstdc++ (Linux) std::string representation
      static const std::regex cxx11_string{
          "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >( (?=>))?" };
      result = std::regex_replace( result, cxx11_string, "std::string" );

      // Normalize libc++ inline namespace - remove std::__1:: prefix
      static const std::regex libc_ns{ "std::__1::" };
      result = std::regex_replace( result, libc_ns, "std::" );

      // Normalize std::basic_string (after removing __1::)
      static const std::regex basic_string{
          "std::basic_string<char, std::char_traits<char>, std::allocator<char>>( (?=>))?" };
      result = std::regex_replace( result, basic_string, "std::string" );

      // Normalize closing angle brackets: >> to > > (C++03 style)
      static const std::regex angle_brackets{ ">>" };
      result = std::regex_replace( result, angle_brackets, "> >" );
      result = std::regex_replace( result, angle_brackets, "> >" ); // twice for >>>

      // Optionally normalize commas: ", " to "," (legacy behavior)
      if ( normalize_commas ) {
        static const std::regex comma_space{ ", " };
        result = std::regex_replace( result, comma_space, "," );
      }

      return result;
    }

  } // namespace Detail
} // namespace System
