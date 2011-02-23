// $Id: ReadTES.cpp,v 1.1 2008/11/04 22:49:24 marcocle Exp $
// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// local
#include "ReadTES.h"

//-----------------------------------------------------------------------------
// Implementation file for class : ReadTES
//
// 2008-11-03 : Marco Cattaneo
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( ReadTES )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ReadTES::ReadTES( const std::string& name,
                  ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  m_locations.clear();
  declareProperty( "Locations", m_locations, "Locations to read" );
}
//=============================================================================
// Destructor
//=============================================================================
ReadTES::~ReadTES() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode ReadTES::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  if ( m_locations.empty() )
    return Error( "You must define at least one TES Location" );

  return StatusCode::SUCCESS;
}
//=============================================================================
// Main execution
//=============================================================================
StatusCode ReadTES::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  for ( std::vector<std::string>::iterator it  = m_locations.begin();
                                           it != m_locations.end(); it++ ) {
    DataObject* pTES = get<DataObject>( *it );
    info() << "Found object " << *it << " at " << pTES << endmsg;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
