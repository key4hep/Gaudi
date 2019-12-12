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
#include "HelloWorld.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"

// Static Factory declaration

DECLARE_COMPONENT( HelloWorld )

// Constructor
//------------------------------------------------------------------------------
HelloWorld::HelloWorld( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ) {
  //------------------------------------------------------------------------------
  m_initialized = false;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::initialize() {
  //------------------------------------------------------------------------------
  // avoid calling initialize more than once
  if ( m_initialized ) return StatusCode::SUCCESS;

  info() << "initializing...." << endmsg;

  Gaudi::Property<int>         m_int;
  Gaudi::Property<double>      m_double;
  Gaudi::Property<std::string> m_string;

  declareInfo( "int_value", t_int, "description" );
  declareInfo( "double_value", t_double, "description" );
  declareInfo( "string_value", t_string, "description" );

  m_initialized = true;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  m_initialized = false;
  return StatusCode::SUCCESS;
}
