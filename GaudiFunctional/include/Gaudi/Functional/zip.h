/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <GaudiKernel/System.h>
#ifndef NDEBUG
#  include <GaudiKernel/GaudiException.h>
#  include <sstream>
#endif
#include <utility>

#if defined( __cpp_lib_ranges_zip ) && defined( __cpp_lib_ranges_as_const )
#  define GAUDI_FUNCTIONAL_USES_STD_RANGES 1
#  include <ranges>
#else
#  include <range/v3/view/const.hpp>
#  include <range/v3/view/zip.hpp>
#endif

namespace Gaudi::Functional::details {
  // CRJ : Stuff for zipping
  namespace zip {

    /// Print the parameter
    template <typename OS, typename Arg>
    OS& printSizes( OS& out, Arg&& arg ) {
      out << "SizeOf'" << System::typeinfoName( typeid( Arg ) ) << "'=" << arg.size();
      return out;
    }

    /// Print the parameters
    template <typename OS, typename Arg, typename... Args>
      requires( sizeof...( Args ) > 0 )
    OS& printSizes( OS& out, Arg&& arg, Args&&... args ) {
      printSizes( out, arg ) << ", ";
      return printSizes( out, args... );
    }

    /// Compare sizes of 1 or more containers
    template <typename A, typename... Rest>
    bool check_sizes( const A& first, const Rest&... rest ) noexcept {
      return ( ( first.size() == rest.size() ) && ... );
    }

#ifndef NDEBUG
    /// Verify the data container sizes have the same sizes
    template <typename... Args>
    inline decltype( auto ) verifySizes( Args&... args ) {
      if ( !check_sizes( args... ) ) {
        std::ostringstream mess;
        mess << "Zipped containers have different sizes : ";
        printSizes( mess, args... );
        throw GaudiException( mess.str(), "Gaudi::Functional::details::zip::verifySizes", StatusCode::FAILURE );
      }
    }
#endif

    /// Zips multiple containers together to form a single range
    template <typename... Args>
    inline decltype( auto ) range( Args&&... args ) {
#ifndef NDEBUG
      verifySizes( args... );
#endif
#if defined( GAUDI_FUNCTIONAL_USES_STD_RANGES )
      return std::ranges::zip_view( std::forward<Args>( args )... );
#else
      return ranges::views::zip( std::forward<Args>( args )... );
#endif
    }

    /// Zips multiple containers together to form a single const range
    template <typename... Args>
    inline decltype( auto ) const_range( Args&&... args ) {
#ifndef NDEBUG
      verifySizes( args... );
#endif
#if defined( GAUDI_FUNCTIONAL_USES_STD_RANGES )
      return std::ranges::as_const_view( std::ranges::zip_view( std::forward<Args>( args )... ) );
#else
      return ranges::views::const_( ranges::views::zip( std::forward<Args>( args )... ) );
#endif
    }
  } // namespace zip
} // namespace Gaudi::Functional::details
