/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "AlgContextAuditor.h"

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IAlgContextSvc.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/ThreadLocalContext.h>

#include <cassert>

/** @file
 *  Implementation file for class AlgContexAuditor
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
namespace {
  template <StatusCode ( IAlgContextSvc::*fun )( IAlgorithm*, const EventContext& )>
  void call( IAlgContextSvc* ctx, INamedInterface* a ) {
    if ( ctx ) {
      // make a safe cast using "smart interface"
      SmartIF<IAlgorithm> alg{ a };
      const EventContext& ectx = Gaudi::Hive::currentContext();
      if ( alg ) ( ctx->*fun )( alg.get(), ectx ).ignore();
    }
  }
} // namespace

// mandatory auditor factory, needed for instantiation
DECLARE_COMPONENT( AlgContextAuditor )

StatusCode AlgContextAuditor::initialize() {
  return Auditor::initialize().andThen( [&] {
    m_svc = service( "AlgContextSvc", true );
    if ( !m_svc ) {
      error() << "Invalid pointer to IAlgContextSvc" << endmsg;
      return StatusCode::FAILURE; // RETURN
    }
    return StatusCode::SUCCESS;
  } );
}

StatusCode AlgContextAuditor::finalize() {
  m_svc.reset();
  return Auditor::finalize();
}

void AlgContextAuditor::before( StandardEventType evt, INamedInterface* a ) {
  switch ( evt ) {
  case Initialize:
  case Execute:
  case Finalize:
    return call<&IAlgContextSvc::setCurrentAlg>( m_svc, a );
  default:
    return;
  }
}

void AlgContextAuditor::after( StandardEventType evt, INamedInterface* a, const StatusCode& ) {
  switch ( evt ) {
  case Initialize:
  case Execute:
  case Finalize:
    return call<&IAlgContextSvc::unSetCurrentAlg>( m_svc, a );
  default:
    return;
  }
}
