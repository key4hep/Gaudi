// Include files

// from Gaudi
#include "GaudiAlg/GaudiAtomicSequencer.h"

//-----------------------------------------------------------------------------
// Implementation file for class : GaudiAtomicSequencer
//
// 2014-05-26 : Daniel Funke
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GaudiAtomicSequencer::GaudiAtomicSequencer( const std::string& name,
                                ISvcLocator* pSvcLocator)
  : GaudiSequencer ( name , pSvcLocator )
{}

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode GaudiAtomicSequencer::initialize() {

	//in the GaudiSequencer::initalize the sysInitialize of all contained algorithms is called
	//thereafter we have correct DataObjectHandles in the Algorithms
	StatusCode status = GaudiSequencer::initialize();

  if (msgLevel(MSG::DEBUG)) debug() << "==> Initialise" << endmsg;
  if ( !status.isSuccess() ) return status;

  //  addSubAlgorithmDataObjectHandles();

  return status;
}


