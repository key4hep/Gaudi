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
#include "ParentAlg.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"

DECLARE_COMPONENT( ParentAlg )

StatusCode ParentAlg::initialize() {
  info() << "creating sub-algorithms...." << endmsg;

  auto sc = createSubAlgorithm( "SubAlg", "SubAlg1", m_subalg1 );
  if ( !sc ) {
    error() << "Error creating Sub-Algorithm SubAlg1" << endmsg;
    return sc;
  }

  sc = createSubAlgorithm( "SubAlg", "SubAlg2", m_subalg2 );
  if ( !sc ) {
    error() << "Error creating Sub-Algorithm SubAlg2" << endmsg;
    return sc;
  }

  // the base class must be invoked _after_ the subalgorithms are created
  return Sequence::initialize();
}

StatusCode ParentAlg::execute( const EventContext& ctx ) const {
  info() << "executing...." << endmsg;

  for ( auto alg : ( *subAlgorithms() ) ) {
    if ( !alg->sysExecute( ctx ) ) { error() << "Error executing Sub-Algorithm" << alg->name() << endmsg; }
  }
  return StatusCode::SUCCESS;
}

StatusCode ParentAlg::finalize() {
  info() << "finalizing...." << endmsg;
  return Sequence::finalize();
}
