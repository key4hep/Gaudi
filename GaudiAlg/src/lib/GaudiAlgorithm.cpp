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
#define GAUDIALG_GAUDIALGORITHM_CPP 1
// ============================================================================
// include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
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
GaudiAlgorithm::GaudiAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
    : GaudiCommon<Algorithm>( name, pSvcLocator ) {
  setProperty( "RegisterForContextService", true ).ignore();
}
// ============================================================================
// standard initialization method
// ============================================================================
StatusCode GaudiAlgorithm::initialize() {
  // initialize the base class
  const StatusCode sc = GaudiCommon<Algorithm>::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Add any customisations here, that cannot go in GaudiCommon

  // return
  return sc;
}
// ============================================================================
// standard finalization method
// ============================================================================
StatusCode GaudiAlgorithm::finalize() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Finalize base class GaudiAlgorithm" << endmsg;

  // reset pointers
  m_evtColSvc.reset();

  // finalize the base class and return
  return GaudiCommon<Algorithm>::finalize();
}
// ============================================================================
// standard execution method
// ============================================================================
StatusCode GaudiAlgorithm::execute() { return Error( "Default GaudiAlgorithm execute method called !!" ); }
// ============================================================================
// The standard event collection service
// ============================================================================
SmartIF<INTupleSvc>& GaudiAlgorithm::evtColSvc() const {
  if ( !m_evtColSvc ) { m_evtColSvc = svc<INTupleSvc>( "EvtTupleSvc", true ); }
  //
  return m_evtColSvc;
}
// ============================================================================
/*  The generic actions for the execution.
 *  @see  Algorithm
 *  @see IAlgorithm
 *  @see Algorithm::sysExecute
 *  @return status code
 */
// ============================================================================
StatusCode GaudiAlgorithm::sysExecute( const EventContext& ctx ) {
  StatusCode sc{StatusCode::SUCCESS};

  IAlgContextSvc* algCtx = nullptr;
  if ( registerContext() ) { algCtx = contextSvc(); }
  // Lock the context
  Gaudi::Utils::AlgContext cnt( this, algCtx, ctx ); ///< guard/sentry

  // Do not execute if one or more of the m_vetoObjs exist in TES
  const auto it = find_if( begin( m_vetoObjs ), end( m_vetoObjs ),
                           [&]( const std::string& loc ) { return this->exist<DataObject>( loc ); } );
  if ( it != end( m_vetoObjs ) ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << *it << " found, skipping event " << endmsg;
    return sc;
  }

  // Execute if m_requireObjs is empty
  // or if one or more of the m_requireObjs exist in TES
  bool doIt =
      m_requireObjs.empty() || any_of( begin( m_requireObjs ), end( m_requireObjs ),
                                       [&]( const std::string& loc ) { return this->exist<DataObject>( loc ); } );

  // execute the generic method:
  if ( doIt ) sc = Algorithm::sysExecute( ctx );
  return sc;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
