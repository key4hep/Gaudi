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
#include "AlgContextAuditor.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <cassert>

// ============================================================================
/** @file
 *  Implementation file for class AlgContexAuditor
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
// ============================================================================
namespace {
  template <StatusCode ( IAlgContextSvc::*fun )( IAlgorithm*, const EventContext& )>
  void call( IAlgContextSvc* ctx, INamedInterface* a ) {
    if ( ctx ) {
      // make a safe cast using "smart interface"
      SmartIF<IAlgorithm> alg{a};
      const EventContext& ectx = Gaudi::Hive::currentContext();
      if ( alg ) ( ctx->*fun )( alg.get(), ectx ).ignore();
    }
  }
} // namespace
// ============================================================================
// mandatory auditor factory, needed for instantiation
// ============================================================================
DECLARE_COMPONENT( AlgContextAuditor )
// ============================================================================
// standard constructor @see Auditor
// ============================================================================
AlgContextAuditor::AlgContextAuditor( const std::string& name, ISvcLocator* pSvc ) : Auditor( name, pSvc ) {}
// ============================================================================
// standard initialization, see @IAuditor
// ============================================================================
StatusCode AlgContextAuditor::initialize() {
  // initialize the base class
  StatusCode sc = Auditor::initialize();
  if ( sc.isFailure() ) { return sc; } // RETURN
  m_svc = service( "AlgContextSvc", true );
  if ( !m_svc ) {
    error() << "Invalid pointer to IAlgContextSvc" << endmsg;
    return StatusCode::FAILURE; // RETURN
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
// standard finalization, see @IAuditor
// ============================================================================
StatusCode AlgContextAuditor::finalize() {
  m_svc.reset();
  // finalize the base class
  return Auditor::finalize();
}
// ============================================================================
void AlgContextAuditor::beforeInitialize( INamedInterface* a ) { call<&IAlgContextSvc::setCurrentAlg>( m_svc, a ); }
// ============================================================================
void AlgContextAuditor::afterInitialize( INamedInterface* a ) { call<&IAlgContextSvc::unSetCurrentAlg>( m_svc, a ); }
// ============================================================================
void AlgContextAuditor::beforeFinalize( INamedInterface* a ) { call<&IAlgContextSvc::setCurrentAlg>( m_svc, a ); }
// ============================================================================
void AlgContextAuditor::afterFinalize( INamedInterface* a ) { call<&IAlgContextSvc::unSetCurrentAlg>( m_svc, a ); }
// ============================================================================
void AlgContextAuditor::beforeExecute( INamedInterface* a ) { call<&IAlgContextSvc::setCurrentAlg>( m_svc, a ); }
// ============================================================================
void AlgContextAuditor::afterExecute( INamedInterface* a, const StatusCode& /* s */ ) {
  call<&IAlgContextSvc::unSetCurrentAlg>( m_svc, a );
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
