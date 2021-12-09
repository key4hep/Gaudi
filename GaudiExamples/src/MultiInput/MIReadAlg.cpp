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
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

// Example related include files
#include "MIHelpers.h"
#include "MIReadAlg.h"

// Event Model related classes
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

#include "GaudiKernel/System.h"

#include <fstream>

using namespace Gaudi::Examples::MultiInput;

DECLARE_COMPONENT( ReadAlg )

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode ReadAlg::initialize() {
  StatusCode sc = Algorithm::initialize();
  if ( sc.isFailure() ) return sc;

  if ( msgLevel( MSG::DEBUG ) ) debug() << "Reading " << m_addressfile.value() << endmsg;
  m_addresses.clear();
  std::ifstream input{ m_addressfile };
  while ( input.good() ) {
    RootAddressArgs addr;
    input >> addr;
    if ( input.eof() ) break;
    m_addresses.push_back( addr );
  }
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Read " << m_addresses.size() << " addresses" << endmsg;

  m_count = 0;

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadAlg::execute() {

  if ( m_count < m_addresses.size() ) {
    // register the entry "Extra/Tracks" in the TES so that it is loaded
    // from the other file
    StatusCode sc = eventSvc()->registerObject( "Extra", new DataObject );
    if ( sc.isFailure() ) {
      error() << "Cannot add entry 'Extra' to the TES" << endmsg;
      return sc;
    }
    sc =
        SmartIF<IDataManagerSvc>( eventSvc() )->registerAddress( "Extra/Tracks", make_address( m_addresses[m_count] ) );
    if ( sc.isFailure() ) {
      error() << "Failed to register the address to the extra data" << endmsg;
      return sc;
    }
    ++m_count;
  }

  SmartDataPtr<MyTrackVector> trks1( eventSvc(), "Tracks" );
  SmartDataPtr<MyTrackVector> trks2( eventSvc(), "Extra/Tracks" );

  if ( trks1 )
    info() << "Base event tracks: " << trks1->size() << endmsg;
  else
    warning() << "No tracks container in base event" << endmsg;
  if ( trks2 )
    info() << "Extra event tracks: " << trks2->size() << endmsg;
  else
    warning() << "No tracks container in extra event" << endmsg;

  return StatusCode::SUCCESS;
}
