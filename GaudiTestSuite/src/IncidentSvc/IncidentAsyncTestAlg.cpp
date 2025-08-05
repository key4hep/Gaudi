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
#include "IncidentAsyncTestAlg.h"

#include "IIncidentAsyncTestSvc.h"

#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/Incident.h>

// Static Factory declaration
DECLARE_COMPONENT( IncidentAsyncTestAlg )

//=============================================================================
StatusCode IncidentAsyncTestAlg::initialize() {
  StatusCode sc = Algorithm::initialize();
  if ( sc.isFailure() ) return sc;
  // get service containing event data
  m_service = service( m_serviceName, true );
  // Copied from CPUCruncher.cpp
  for ( auto k : m_inpKeys ) {
    debug() << "adding input key " << k << endmsg;
    m_inputObjHandles.emplace_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Reader, this ) );
    declare( *m_inputObjHandles.back() );
  }

  for ( auto k : m_outKeys ) {
    debug() << "adding output key " << k << endmsg;
    m_outputObjHandles.emplace_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this ) );
    declare( *m_outputObjHandles.back() );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentAsyncTestAlg::execute() {
  uint64_t  data = 0;
  MsgStream logstream( msgSvc(), name() );
  for ( auto& inputHandle : m_inputObjHandles ) {
    if ( !inputHandle->isValid() ) continue;

    DataObject const* obj = inputHandle->get();
    if ( !obj ) logstream << MSG::ERROR << "A read object was a null pointer." << endmsg;
  }

  m_service->getData( &data );
  for ( auto& outputHandle : m_outputObjHandles ) {
    if ( !outputHandle->isValid() ) continue;
    outputHandle->put( std::make_unique<DataObject>() );
  }
  info() << "Read data " << data << endmsg;
  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentAsyncTestAlg::finalize() {
  info() << "Finalizing " << endmsg;
  return Algorithm::finalize();
}
