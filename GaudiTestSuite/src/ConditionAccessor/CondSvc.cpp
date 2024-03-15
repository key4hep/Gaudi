/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/TestSuite/Conditions/CondSvc.h>

namespace Gaudi::TestSuite::Conditions {

  StatusCode CondSvc::initialize() {

    // Convert alg names vector to a set for easier search
    std::set<std::string> algNameSet( m_algNames.begin(), m_algNames.end() );

    // Get conditions alg pointers
    m_algResourcePool = serviceLocator()->service( "AlgResourcePool" );
    for ( auto& alg : m_algResourcePool->getFlatAlgList() ) {
      if ( algNameSet.find( alg->name() ) != algNameSet.end() ) { m_condAlgs.insert( alg ); }
    }

    // Get conditions data ids
    for ( auto& name : m_dataNames ) { m_condData.insert( DataObjID( name ) ); }

    return StatusCode::SUCCESS;
  }
} // namespace Gaudi::TestSuite::Conditions
