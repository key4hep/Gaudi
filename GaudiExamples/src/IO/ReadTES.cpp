// Include files

// local
#include "ReadTES.h"

//-----------------------------------------------------------------------------
// Implementation file for class : ReadTES
//
// 2008-11-03 : Marco Cattaneo
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( ReadTES )

//=============================================================================
// Initialization
//=============================================================================
StatusCode ReadTES::initialize()
{
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;              // error printed already by GaudiAlgorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;

  if ( m_locations.empty() ) return Error( "You must define at least one TES Location" );

  return StatusCode::SUCCESS;
}
//=============================================================================
// Main execution
//=============================================================================
StatusCode ReadTES::execute()
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  for ( auto& loc : m_locations ) {
    DataObject* pTES = get<DataObject>( loc );
    info() << "Found object " << loc << " at " << pTES << endmsg;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
