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
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/IDataHandleHolder.h>
#include <GaudiKernel/SerializeSTL.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace Gaudi::Functional::details {
  template <typename Handles>
  [[deprecated]] Handles make_vector_of_handles( IDataHandleHolder* owner, const std::vector<DataObjID>& init ) {
    Handles handles;
    handles.reserve( init.size() );
    std::transform( init.begin(), init.end(), std::back_inserter( handles ),
                    [&]( const auto& loc ) -> typename Handles::value_type {
                      return { loc, owner };
                    } );
    return handles;
  }

  template <typename OutputSpec, typename InputSpec, typename Traits_>
  class DataHandleMixin;

  template <typename Out, typename In, typename Tr>
  [[deprecated]] void updateHandleLocation( DataHandleMixin<Out, In, Tr>& parent, const std::string& prop,
                                            const std::string& newLoc ) {
    parent.setProperty( prop, newLoc ).orThrow( "Could not set Property", prop + " -> " + newLoc );
  }

  template <typename Out, typename In, typename Tr>
  [[deprecated]] void updateHandleLocations( DataHandleMixin<Out, In, Tr>& parent, const std::string& prop,
                                             const std::vector<std::string>& newLocs ) {
    std::ostringstream ss;
    GaudiUtils::details::ostream_joiner( ss << '[', newLocs, ", ", []( std::ostream& os, const auto& i ) -> auto& {
      return os << "'" << i << "'";
    } ) << ']';
    parent.setProperty( prop, ss.str() ).orThrow( "Could not set Property", prop + " -> " + ss.str() );
  }
} // namespace Gaudi::Functional::details
