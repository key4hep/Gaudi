/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Algorithm.h>
#include <algorithm>

namespace Gaudi::Examples {

  /// Simple algorithm useful as base class for tests.
  struct TestAlg : ::Algorithm {
    using Algorithm::Algorithm;
    StatusCode initialize() override {
      return Algorithm::initialize().andThen( [&]() {
        if ( m_propsPrint && msgLevel( MSG::DEBUG ) ) {
          auto& log        = debug();
          auto  properties = getProperties();
          log << "List of ALL properties of " << System::typeinfoName( typeid( *this ) ) << "/" << this->name()
              << "  #properties = " << properties.size() << endmsg;
          std::sort( begin( properties ), end( properties ),
                     []( const auto& a, const auto& b ) { return a->name() < b->name(); } );
          for ( const auto& property : properties ) { log << "Property ['Name': Value] = " << *property << endmsg; }
        }
      } );
    }
    Gaudi::Property<bool> m_propsPrint{ this, "PropertiesPrint", false,
                                        "print the properties of the component at initialize" };
  };
} // namespace Gaudi::Examples
