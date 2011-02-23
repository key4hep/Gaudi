// $Id: GaudiAlgorithm.cpp,v 1.12 2008/11/04 22:49:25 marcocle Exp $
// ============================================================================
#define  GAUDIALG_GAUDIALGORITHM_CPP 1
// ============================================================================
// include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/DataObject.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
/** @file
 *
 *  Implementation file for the class GaudiAlgorithm
 *
 *  @see GaudiAlgorithm
 *  @see Algorithm
 *  @see IAlgorithm
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Christopher Jones  Christopher.Rob.Jones@cern.ch
 *  @date 30/06/2001
 */
// ============================================================================
// templated methods
// ============================================================================
#include "GaudiCommon.icpp"
// ============================================================================
template class GaudiCommon<Algorithm>;
// ============================================================================
// Standard algorithm like constructor
// ============================================================================
GaudiAlgorithm::GaudiAlgorithm ( const std::string&  name        ,
                                 ISvcLocator*        pSvcLocator )
  : GaudiCommon<Algorithm> ( name , pSvcLocator )
  //
  , m_evtColSvc  ()  // pointer to Event Tag Collection Service
{
  m_vetoObjs.clear();
  m_requireObjs.clear();
  
  setProperty ( "RegisterForContextService" , true ).ignore() ;
  
  declareProperty( "VetoObjects", m_vetoObjs,
                   "Skip execute if one or more of these TES objects exists" );
  declareProperty( "RequireObjects", m_requireObjs,
                   "Execute only if one or more of these TES objects exists" );
}
// ============================================================================
// Destructor
// ============================================================================
GaudiAlgorithm::~GaudiAlgorithm() { }
// ============================================================================
// standard initialization method
// ============================================================================
StatusCode GaudiAlgorithm::initialize()
{
  // initialize the base class
  const StatusCode sc = GaudiCommon<Algorithm>::initialize() ;
  if ( sc.isFailure() ) { return sc; }

  // Add any customisations here, that cannot go in GaudiCommon

  // return
  return sc;
}
// ============================================================================
// standard finalization method
// ============================================================================
StatusCode GaudiAlgorithm::finalize()
{
  if ( msgLevel(MSG::DEBUG) )
    debug() << "Finalize base class GaudiAlgorithm" << endmsg;
  
  // reset pointers
  m_evtColSvc.reset() ;
  
  // finalize the base class and return
  return GaudiCommon<Algorithm>::finalize() ;
}
// ============================================================================
// standard execution method
// ============================================================================
StatusCode GaudiAlgorithm::execute()
{
  return Error ( "Default GaudiAlgorithm execute method called !!" ) ;
}
// ============================================================================
// The standard event collection service
// ============================================================================
SmartIF<INTupleSvc>& GaudiAlgorithm::evtColSvc() const
{
  if ( !m_evtColSvc.isValid() )
  { m_evtColSvc = svc< INTupleSvc > ( "EvtTupleSvc" , true ) ; }
  //
  return m_evtColSvc ;
}
// ============================================================================
/*  The generic actions for the execution.
 *  @see  Algorithm
 *  @see IAlgorithm
 *  @see Algorithm::sysExecute
 *  @return status code
 */
// ============================================================================
StatusCode GaudiAlgorithm::sysExecute ()
{
  IAlgContextSvc* ctx = 0 ;
  if ( registerContext() ) { ctx = contextSvc() ; }
  // Lock the context
  Gaudi::Utils::AlgContext cnt ( ctx , this ) ;  ///< guard/sentry

  // Do not execute if one or more of the m_vetoObjs exist in TES
  for( std::vector<std::string>::iterator it  = m_vetoObjs.begin();
                                          it != m_vetoObjs.end(); it++ ) {
    if( exist<DataObject>(*it) ) {
      debug() << *it << " found, skipping event " << endmsg;
      return StatusCode::SUCCESS;
    }
  }

  // Execute if m_requireObjs is empty
  bool doIt = m_requireObjs.empty() ? true : false;

  // Execute also if one or more of the m_requireObjs exist in TES
  for( std::vector<std::string>::iterator it  = m_requireObjs.begin();
                                          it != m_requireObjs.end(); it++ ) {
    if( exist<DataObject>(*it) ) {
      doIt = true;
      break;
    }
  }

  if( doIt )
    // execute the generic method:
    return Algorithm::sysExecute() ;
  else
    return StatusCode::SUCCESS;
}
// ============================================================================




// ============================================================================
// The END
// ============================================================================

