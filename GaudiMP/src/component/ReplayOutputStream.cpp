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
// Include files

// From Gaudi
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IDataManagerSvc.h"
// local
#include "RecordOutputStream.h"
#include "ReplayOutputStream.h"

#include <algorithm>
#include <functional>
#include <list>

// ----------------------------------------------------------------------------
// Implementation file for class: ReplayOutputStream
//
// 30/08/2013: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_COMPONENT( ReplayOutputStream )

namespace {

  /// Helper class to call trigger the transitions in an OutputStream
  template <Gaudi::StateMachine::Transition TR>
  class OutStreamTransition {
  public:
    typedef ReplayOutputStream::OutStreamsMapType::value_type ItemType;
    OutStreamTransition( MsgStream& msg ) : m_msg( msg ), m_code( StatusCode::SUCCESS ) {}

    void operator()( ItemType& item );

    StatusCode result() const { return m_code; }

  private:
    MsgStream& m_msg;
    StatusCode m_code;
  };

  template <>
  void OutStreamTransition<Gaudi::StateMachine::INITIALIZE>::operator()( ItemType& item ) {
    const StatusCode sc = item.second->sysInitialize();
    if ( sc.isFailure() ) {
      m_msg << MSG::WARNING << "Failed to initialize " << item.first << endmsg;
      m_code = sc;
    }
  }
  template <>
  void OutStreamTransition<Gaudi::StateMachine::START>::operator()( ItemType& item ) {
    const StatusCode sc = item.second->sysStart();
    if ( sc.isFailure() ) {
      m_msg << MSG::WARNING << "Failed to start " << item.first << endmsg;
      m_code = sc;
    }
  }
  template <>
  void OutStreamTransition<Gaudi::StateMachine::STOP>::operator()( ItemType& item ) {
    const StatusCode sc = item.second->sysStop();
    if ( sc.isFailure() ) {
      m_msg << MSG::WARNING << "Failed to stop " << item.first << endmsg;
      m_code = sc;
    }
  }
  template <>
  void OutStreamTransition<Gaudi::StateMachine::FINALIZE>::operator()( ItemType& item ) {
    const StatusCode sc = item.second->sysFinalize();
    if ( sc.isFailure() ) {
      m_msg << MSG::WARNING << "Failed to finalize " << item.first << endmsg;
      m_code = sc;
    }
  }
} // namespace

template <Gaudi::StateMachine::Transition TR>
StatusCode ReplayOutputStream::i_outStreamTransition() {
  OutStreamTransition<TR> trans( msg() );
  std::for_each( m_outputStreams.begin(), m_outputStreams.end(), trans );
  return trans.result();
}

// ============================================================================
// Initialization
// ============================================================================
StatusCode ReplayOutputStream::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;              // error printed already by GaudiAlgorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;

  m_algMgr = service( "ApplicationMgr" );
  if ( UNLIKELY( !m_algMgr ) ) return Error( "cannot retrieve IAlgManager" );

  m_evtMgr = evtSvc();
  if ( UNLIKELY( !m_evtMgr ) ) return Error( "cannot retrieve IDataManagerSvc " );

  std::for_each( m_outputStreamNames.begin(), m_outputStreamNames.end(), OutStreamAdder( this ) );

  return i_outStreamTransition<Gaudi::StateMachine::INITIALIZE>();
}

StatusCode ReplayOutputStream::start() {
  StatusCode sc = GaudiAlgorithm::start(); // must be executed first
  if ( sc.isFailure() ) return sc;         // error printed already by GaudiAlgorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Start" << endmsg;

  return i_outStreamTransition<Gaudi::StateMachine::START>();
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode ReplayOutputStream::execute() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  std::vector<std::string> names;
  if ( auto sc = m_evtMgr->traverseSubTree( RecordOutputStream::locationRoot(),
                                            [&names]( IRegistry* pReg, int lvl ) {
                                              if ( lvl > 0 ) names.push_back( pReg->name() );
                                              return true;
                                            } );
       !sc )
    return sc;

  std::for_each( names.begin(), names.end(), [this]( const std::string& name ) {
    SmartIF<IAlgorithm>& alg = this->m_outputStreams[name];
    if ( alg ) {
      const auto& ctx = Gaudi::Hive::currentContext();
      if ( alg->execState( ctx ).state() != AlgExecState::State::Done ) {
        alg->sysExecute( ctx ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      } else {
        this->warning() << name << " already executed for the current event" << endmsg;
      }
    } else {
      this->warning() << "invalid OuputStream " << name << endmsg;
    }
  } );

  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode ReplayOutputStream::finalize() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  StatusCode sc = i_outStreamTransition<Gaudi::StateMachine::FINALIZE>();

  // release interfaces
  m_outputStreams.clear();
  m_algMgr.reset();
  m_evtMgr.reset();

  StatusCode fsc = GaudiAlgorithm::finalize(); // must be called after all other actions
  if ( sc.isSuccess() ) sc = fsc;
  return sc;
}

StatusCode ReplayOutputStream::stop() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Stop" << endmsg;

  StatusCode sc = i_outStreamTransition<Gaudi::StateMachine::STOP>();

  StatusCode ssc = GaudiAlgorithm::stop(); // must be called after all other actions
  if ( sc.isSuccess() ) sc = ssc;
  return sc;
}

void ReplayOutputStream::i_addOutputStream( const Gaudi::Utils::TypeNameString& outStream ) {
  // we prepend '/' to the name of the algorithm to simplify the handling in
  // OutputStreamsCollector
  const std::string algId = "/" + outStream.name();
  if ( m_outputStreams.find( algId ) == m_outputStreams.end() ) {
    m_outputStreams[algId] = m_algMgr->algorithm( outStream );
    if ( !m_outputStreams[algId] ) {
      throw GaudiException( name(), "Could not get algorithm " + outStream.name(), StatusCode::FAILURE );
    }
  } else {
    warning() << "OutputStream instance " << outStream.name() << " already added, ignoring " << outStream << endmsg;
  }
}

// ============================================================================
