/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "DataCreator.h"
#include <GaudiKernel/DataObject.h>

// Static Factory declaration
DECLARE_COMPONENT( DataCreator )

//------------------------------------------------------------------------------
StatusCode DataCreator::initialize() {
  //------------------------------------------------------------------------------
  info() << "initializing...." << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode DataCreator::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;
  return eventSvc()->registerObject( m_data, new DataObject() );
}

//------------------------------------------------------------------------------
StatusCode DataCreator::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}
