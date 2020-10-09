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
#include <initializer_list>
#include <sstream>
#include <tuple>

// from Gaudi
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/ISequencerTimerTool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "GaudiCommon.icpp"
template class GaudiCommon<Gaudi::Sequence>;

namespace {

  bool isDefault( const std::string& s ) { return s.empty(); }

} // namespace

//-----------------------------------------------------------------------------
// Implementation file for class : GaudiSequencer
//
// 2004-05-13 : Olivier Callot
//-----------------------------------------------------------------------------

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode GaudiSequencer::initialize() {
  // Note: not calling base class initialize because we want to reimplement the loop over sub algs
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialise" << endmsg;

  auto status = decodeNames();
  if ( !status.isSuccess() ) return status;

  m_timerTool = tool<ISequencerTimerTool>( "SequencerTimerTool" );

  if ( m_timerTool->globalTiming() ) m_measureTime = true;

  if ( m_measureTime ) {
    m_timer = m_timerTool->addTimer( name() );
    m_timerTool->increaseIndent();
  } else {
    release( m_timerTool ).ignore();
    m_timerTool = nullptr;
  }

  //== Initialize the algorithms
  for ( auto& entry : m_entries ) {
    if ( m_measureTime ) { entry.setTimer( m_timerTool->addTimer( entry.algorithm()->name() ) ); }

    status = entry.algorithm()->sysInitialize();
    if ( !status.isSuccess() ) { return Error( "Can not initialize " + entry.algorithm()->name(), status ); }
  }
  if ( m_measureTime ) m_timerTool->decreaseIndent();

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode GaudiSequencer::execute( const EventContext& ctx ) const {

  if ( m_measureTime ) m_timerTool->start( m_timer );

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  StatusCode result = StatusCode( StatusCode::SUCCESS, true );

  bool seqPass = !m_modeOR; //  for OR, result will be false, unless (at least) one is true
                            //  for AND, result will be true, unless (at least) one is false
                            //    also see comment below ....

  for ( auto& entry : m_entries ) {
    Gaudi::Algorithm* myAlg = entry.algorithm();
    if ( !myAlg->isEnabled() ) continue;
    if ( myAlg->execState( ctx ).state() != AlgExecState::State::Done ) {

      if ( m_measureTime ) m_timerTool->start( entry.timer() );
      result = myAlg->sysExecute( ctx );
      if ( m_measureTime ) m_timerTool->stop( entry.timer() );
      if ( !result.isSuccess() ) break; //== Abort and return bad status
    }
    //== Check the returned status
    if ( !m_ignoreFilter ) {
      bool passed = myAlg->execState( ctx ).filterPassed();
      if ( msgLevel( MSG::VERBOSE ) )
        verbose() << "Algorithm " << myAlg->name() << " returned filter passed " << ( passed ? "true" : "false" )
                  << endmsg;
      if ( entry.reverse() ) passed = !passed;

      //== indicate our own result. For OR, exit as soon as true.
      //        If no more, will exit with false.
      //== for AND, exit as soon as false. Else, will be true (default)

      // if not short-circuiting, make sure we latch iPass to 'true' in
      // OR mode (i.e. it is sufficient for one item to be true in order
      // to be true at the end, and thus we start out at 'false'), and latch
      // to 'false' in AND mode (i.e. it is sufficient for one item to
      // be false to the false in the end, and thus we start out at 'true')
      // -- i.e. we should not just blindly return the 'last' passed status!

      // or to put it another way: in OR mode, we don't care about things
      // which are false, as they leave our current state alone (provided
      // we stared as 'false'!), and in AND mode, we keep our current
      // state until someone returns 'false' (provided we started as 'true')
      if ( m_modeOR ? passed : !passed ) {
        seqPass = passed;
        if ( msgLevel( MSG::VERBOSE ) ) verbose() << "SeqPass is now " << ( seqPass ? "true" : "false" ) << endmsg;
        if ( m_shortCircuit ) break;
      }
    }
  }
  if ( msgLevel( MSG::VERBOSE ) ) verbose() << "SeqPass is " << ( seqPass ? "true" : "false" ) << endmsg;
  auto& state = execState( ctx );
  if ( !m_ignoreFilter && !m_entries.empty() ) state.setFilterPassed( m_invert ? !seqPass : seqPass );
  state.setState( AlgExecState::State::Done );

  if ( m_measureTime ) m_timerTool->stop( m_timer );

  return m_returnOK ? ( result.ignore(), StatusCode::SUCCESS ) : result;
}

//=========================================================================
//  Decode the input names and fills the m_algs vector.
//=========================================================================
StatusCode GaudiSequencer::decodeNames() {
  StatusCode final = StatusCode::SUCCESS;
  m_entries.clear();

  //== Get the "Context" option if in the file...

  //= Get the Application manager, to see if algorithm exist
  auto appMgr = service<IAlgManager>( "ApplicationMgr" );
  for ( const auto& item : m_names.value() ) {
    const Gaudi::Utils::TypeNameString typeName( item );
    const std::string&                 theName = typeName.name();
    const std::string&                 theType = typeName.type();

    //== Check wether the specified algorithm already exists. If not, create it
    StatusCode          result = StatusCode::SUCCESS;
    SmartIF<IAlgorithm> myIAlg = appMgr->algorithm( typeName, false ); // do not create it now
    if ( !myIAlg ) {
      Gaudi::Algorithm* myAlg = nullptr;
      result                  = createSubAlgorithm( theType, theName, myAlg );
      if ( myAlg ) {
        // Override the default values of the special properties Context and RootInTES,
        // which will be superseded by the explicit value in options (if present).
        if ( !isDefault( context() ) && myAlg->hasProperty( "Context" ) ) {
          myAlg->setProperty( "Context", context() ).ignore();
        }
        if ( !isDefault( rootInTES() ) && myAlg->hasProperty( "RootInTES" ) ) {
          myAlg->setProperty( "RootInTES", rootInTES() ).ignore();
        }
      }
      myIAlg = myAlg; // ensure that myIAlg.isValid() from here onwards!
    } else {
      Gaudi::Algorithm* myAlg = dynamic_cast<Gaudi::Algorithm*>( myIAlg.get() );
      if ( myAlg ) {
        subAlgorithms()->push_back( myAlg );
        // when the algorithm is not created, the ref count is short by one, so we have to fix it.
        myAlg->addRef();
      }
    }

    // propagate the sub-algorithm into own state.
    if ( result.isSuccess() && Gaudi::StateMachine::INITIALIZED <= FSMState() && myIAlg &&
         Gaudi::StateMachine::INITIALIZED > myIAlg->FSMState() ) {
      StatusCode sc = myIAlg->sysInitialize();
      if ( sc.isFailure() ) result = sc;
    }

    // propagate the sub-algorithm into own state.
    if ( result.isSuccess() && Gaudi::StateMachine::RUNNING <= FSMState() && myIAlg &&
         Gaudi::StateMachine::RUNNING > myIAlg->FSMState() ) {
      StatusCode sc = myIAlg->sysStart();
      if ( sc.isFailure() ) result = sc;
    }

    //== Is it an Algorithm ?  Strange test...
    if ( result.isSuccess() ) {
      // TODO: (MCl) it is possible to avoid the dynamic_cast in most of the
      //             cases by keeping the result of createSubAlgorithm.
      Gaudi::Algorithm* myAlg = dynamic_cast<Gaudi::Algorithm*>( myIAlg.get() );
      if ( myAlg ) {
        // Note: The reference counting is kept by the system of sub-algorithms
        m_entries.emplace_back( myAlg );
        if ( msgLevel( MSG::DEBUG ) ) debug() << "Added algorithm " << theName << endmsg;
      } else {
        warning() << theName << " is not a Gaudi::Algorithm - failed dynamic_cast" << endmsg;
        final = StatusCode::FAILURE;
      }
    } else {
      warning() << "Unable to find or create " << theName << endmsg;
      final = result;
    }
  }

  //== Print the list of algorithms
  MsgStream& msg = info();
  if ( m_modeOR ) msg << "OR ";
  msg << "Member list: ";
  GaudiUtils::details::ostream_joiner( msg, m_entries, ", ",
                                       []( auto& os, const AlgorithmEntry& e ) -> decltype( auto ) {
                                         Gaudi::Algorithm* alg = e.algorithm();
                                         std::string       typ = alg->type();
                                         os << typ;
                                         if ( alg->name() != typ ) os << "/" << alg->name();
                                         return os;
                                       } );
  if ( !isDefault( context() ) ) msg << ", with context '" << context() << "'";
  if ( !isDefault( rootInTES() ) ) msg << ", with rootInTES '" << rootInTES() << "'";
  msg << endmsg;

  return final;
}

//=========================================================================
//  Interface for the Property manager
//=========================================================================
void GaudiSequencer::membershipHandler() {
  // no action for not-yet initialized sequencer
  if ( Gaudi::StateMachine::INITIALIZED > FSMState() ) return; // RETURN

  decodeNames().ignore();

  if ( !m_measureTime ) return; // RETURN

  // add the entries into timer table:

  if ( !m_timerTool ) { m_timerTool = tool<ISequencerTimerTool>( "SequencerTimerTool" ); }

  if ( m_timerTool->globalTiming() ) m_measureTime = true;

  m_timer = m_timerTool->addTimer( name() );
  m_timerTool->increaseIndent();

  for ( auto& entry : m_entries ) { entry.setTimer( m_timerTool->addTimer( entry.algorithm()->name() ) ); }

  m_timerTool->decreaseIndent();
}

std::ostream& GaudiSequencer::toControlFlowExpression( std::ostream& os ) const {
  if ( m_invert ) os << "~";
  // the default filterpass value for an empty sequencer depends on ModeOR
  if ( m_entries.empty() ) return os << ( ( !m_modeOR ) ? "CFTrue" : "CFFalse" );

  // if we have only one element, we do not need a name
  if ( m_entries.size() > 1 ) os << "seq(";

  const auto op    = m_modeOR ? " | " : " & ";
  const auto last  = end( m_entries );
  const auto first = begin( m_entries );
  for ( auto iterator = first; iterator != last; ++iterator ) {
    if ( iterator != first ) os << op;
    if ( iterator->reverse() ) os << "~";
    iterator->algorithm()->toControlFlowExpression( os );
  }

  if ( m_entries.size() > 1 ) os << ")";
  return os;
}

// ============================================================================
StatusCode GaudiSequencer::sysExecute( const EventContext& ctx ) {
  StatusCode sc{StatusCode::SUCCESS};

  IAlgContextSvc* algCtx = nullptr;
  if ( registerContext() ) { algCtx = contextSvc(); }
  // Lock the context
  Gaudi::Utils::AlgContext cnt( this, algCtx, ctx ); ///< guard/sentry

  // Do not execute if one or more of the m_vetoObjs exist in TES
  const auto it = find_if( begin( m_vetoObjs ), end( m_vetoObjs ),
                           [&]( const std::string& loc ) { return this->exist<DataObject>( evtSvc(), loc ); } );
  if ( it != end( m_vetoObjs ) ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << *it << " found, skipping event " << endmsg;
    return sc;
  }

  // Execute if m_requireObjs is empty
  // or if one or more of the m_requireObjs exist in TES
  bool doIt = m_requireObjs.empty() ||
              any_of( begin( m_requireObjs ), end( m_requireObjs ),
                      [&]( const std::string& loc ) { return this->exist<DataObject>( evtSvc(), loc ); } );

  // execute the generic method:
  if ( doIt ) sc = GaudiCommon<Gaudi::Sequence>::sysExecute( ctx );
  return sc;
}
//=============================================================================
