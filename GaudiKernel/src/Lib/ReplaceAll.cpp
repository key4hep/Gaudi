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
#include <boost/algorithm/string/replace.hpp>
#include <string>
#include <string_view>

namespace details {
  // External implementation using boost not to have to include
  // boost in a header file
  std::string replace_all( std::string str, std::string_view from, std::string_view to ) {
    if ( from.empty() ) return str;
    boost::replace_all( str, from, to );
    return str;
  }
} // namespace details
