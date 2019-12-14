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
// Framework include files
#include "RecordStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( RecordStream )

// Standard Constructor
RecordStream::RecordStream( const std::string& name, ISvcLocator* pSvcLocator ) : OutputStream( name, pSvcLocator ) {
  /// in the baseclass, always fire the incidents by default
  /// in RecordStream this will be set to false, and configurable
  declareProperty( "FireIncidents", m_fireIncidents = false, "if to fire incidents for writing opening/closing etc" );
}

StatusCode RecordStream::finalize() {
  info() << "Set up File Summary Record" << endmsg;
  if ( !m_fireIncidents && msgLevel( MSG::VERBOSE ) ) verbose() << "will not fire incidents" << endmsg;
  StatusCode sc = OutputStream::execute();
  if ( !sc.isSuccess() ) { warning() << "Error writing run summary record....." << endmsg; }
  return OutputStream::finalize();
}
