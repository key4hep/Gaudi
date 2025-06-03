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

#include <Gaudi/Auditor.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IAlgContextSvc.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>

class IAlgContextSvc;

/** @class AlgContextAuditor
 *  Description:  Register/Unregister the AlgContext of each
 *  algorithm before entering the algorithm and after leaving it
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
class AlgContextAuditor : public Gaudi::Auditor {
public:
  // IAuditor implementation
  void before( std::string const&, std::string const&, EventContext const& ) override;
  void after( std::string const&, std::string const&, EventContext const&, StatusCode const& ) override;

public:
  using Auditor::Auditor;
  StatusCode initialize() override;
  StatusCode finalize() override;

private:
  /// delete the default/copy constructor and assignment
  AlgContextAuditor()                                      = delete;
  AlgContextAuditor( const AlgContextAuditor& )            = delete;
  AlgContextAuditor& operator=( const AlgContextAuditor& ) = delete;

private:
  /// the pointer to Algorithm Context Service
  SmartIF<IAlgContextSvc> m_svc;
  /// the pointer to Algorithm Manager
  SmartIF<IAlgManager> m_algMgr;
};

// mandatory auditor factory, needed for instantiation
DECLARE_COMPONENT( AlgContextAuditor )

StatusCode AlgContextAuditor::initialize() {
  return Auditor::initialize().andThen( [&] {
    m_svc = service( "AlgContextSvc", true );
    if ( !m_svc ) {
      error() << "Invalid pointer to IAlgContextSvc" << endmsg;
      return StatusCode::FAILURE; // RETURN
    }
    m_algMgr = serviceLocator();
    if ( !m_algMgr ) {
      error() << "Invalid pointer to IAlgManager" << endmsg;
      return StatusCode::FAILURE; // RETURN
    }
    return StatusCode::SUCCESS;
  } );
}

StatusCode AlgContextAuditor::finalize() {
  m_svc.reset();
  m_algMgr.reset();
  return Auditor::finalize();
}

void AlgContextAuditor::before( std::string const& event, std::string const& caller, EventContext const& context ) {
  if ( m_svc && ( event == Gaudi::IAuditor::Initialize || event == Gaudi::IAuditor::Execute ||
                  event == Gaudi::IAuditor::Finalize ) ) {
    SmartIF<IAlgorithm>& alg = m_algMgr->algorithm( caller, false );
    if ( alg ) m_svc->setCurrentAlg( alg, context ).ignore();
  }
}

void AlgContextAuditor::after( std::string const& event, std::string const& caller, EventContext const& context,
                               StatusCode const& ) {
  if ( m_svc && ( event == Gaudi::IAuditor::Initialize || event == Gaudi::IAuditor::Execute ||
                  event == Gaudi::IAuditor::Finalize ) ) {
    SmartIF<IAlgorithm>& alg = m_algMgr->algorithm( caller, false );
    if ( alg ) m_svc->unSetCurrentAlg( alg, context ).ignore();
  }
}
