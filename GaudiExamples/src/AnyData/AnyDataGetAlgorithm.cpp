/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include <vector>

// from Gaudi
#include "GaudiKernel/AnyDataWrapper.h"

// local
#include "AnyDataGetAlgorithm.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AnyDataGetAlgorithm
//
// 2016-05-26 : Roel Aaij
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT_WITH_ID( AnyDataGetAlgorithm<int>, "AnyDataGetAlgorithm_Int" )
DECLARE_COMPONENT_WITH_ID( AnyDataGetAlgorithm<std::vector<int>>, "AnyDataGetAlgorithm_VectorInt" )

namespace {
  using std::vector;
}

//=============================================================================
// Main execution
//=============================================================================
template <class T>
StatusCode AnyDataGetAlgorithm<T>::execute() {

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  auto base = getIfExists<AnyDataWrapperBase>( m_location );
  if ( base ) { info() << "Got base from " << m_location.value() << endmsg; }
  const auto i = dynamic_cast<const AnyDataWrapper<T>*>( base );
  if ( i ) {
    info() << "Got " << System::typeinfoName( typeid( T ) ) << " from " << m_location.value() << ": " << i->getData()
           << endmsg;
  }

  const auto& ids = m_ids.get();
  info() << "AnyDataHandler holds:";
  for ( const auto& i : *ids ) info() << i << " ";
  info() << endmsg;
  return StatusCode::SUCCESS;
}
