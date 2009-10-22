// $Id: AuditorTestAlg.cpp,v 1.1 2008/04/03 14:42:59 marcocle Exp $

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class AuditorTestAlg AuditorTestAlg.cpp
 *  
 *
 *  @author Marco Clemencic
 *  @date   Apr 2, 2008
 */
class AuditorTestAlg : public GaudiAlgorithm {
public: 
  /// Standard constructor
  AuditorTestAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~AuditorTestAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:

};

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

//-----------------------------------------------------------------------------
// Implementation for class : AuditorTestAlg
//
// Apr 2, 2008 : Marco Clemencic
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( AuditorTestAlg );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AuditorTestAlg::AuditorTestAlg( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  
}
//=============================================================================
// Destructor
//=============================================================================
AuditorTestAlg::~AuditorTestAlg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode AuditorTestAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;
  
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AuditorTestAlg::execute() {

  debug() << "==> Execute" << endmsg;
  
  const IAuditor::CustomEventType evt("loop");
  auditorSvc()->before(evt,name());
  for (long i = 0; i < 1000000; ++i) {}
  auditorSvc()->after(evt,name());
  
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode AuditorTestAlg::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
