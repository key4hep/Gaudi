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
#include "PropertyProxy.h"

#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IChronoStatSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/SmartIF.h>

DECLARE_COMPONENT( PropertyProxy )

// Constructor
//------------------------------------------------------------------------------
PropertyProxy::PropertyProxy( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ) {
  //------------------------------------------------------------------------------
  // Declare remote properties at this moment
  auto               algMgr = serviceLocator()->as<IAlgManager>();
  SmartIF<IProperty> rAlgP( algMgr->algorithm( "PropertyAlg" ) );
  if ( rAlgP ) {
    m_remAlg = rAlgP; // remember it for later
    declareRemoteProperty( "RInt", rAlgP, "Int" );
    declareRemoteProperty( "String", rAlgP );
  } else {
    warning() << " The 'remote' Algorithm PropertyAlg is not found" << endmsg;
  }
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::initialize() {
  if ( !Algorithm::initialize() ) return StatusCode::FAILURE;

  std::string value( "empty" );
  std::string value1( "empty" );

  if ( !this->getProperty( "RInt", value ) ) { warning() << "failed to get property RInt" << endmsg; }
  info() << " Got property this.RInt = " << value << ";" << endmsg;

  info() << " Set property this.RInt = 1001;" << endmsg;
  if ( !this->setProperty( "RInt", 1001 ) ) { warning() << "failed to set property RInt" << endmsg; }

  this->getProperty( "RInt", value ).ignore();
  info() << " Got property this.RInt = " << value << ";" << endmsg;
  if ( value != "1001" ) { error() << "RInt value not what expected" << endmsg; }

  this->getProperty( "String", value ).ignore();
  m_remAlg->getProperty( "String", value1 ).ignore();
  if ( value == value1 ) {
    info() << " Got property this.String = " << value << ";" << endmsg;
  } else {
    error() << " Local property [" << value1 << "] not equal [" << value << "]" << endmsg;
  }

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  return StatusCode::SUCCESS;
}
