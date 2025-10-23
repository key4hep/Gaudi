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
// Sequencer class
// Implements:
// 1) Common functionality of IInterface
// 2) Default behavior for the IAlgorithm
#include <Gaudi/Sequencer.h>

#include <GaudiKernel/Chrono.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Stat.h>
#include <GaudiKernel/ThreadLocalContext.h>

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

namespace Gaudi {
  StatusCode Sequencer::initialize() {
    auto is_good = decodeMemberNames();
    if ( !is_good ) {
      error() << "Unable to configure one or more sequencer members " << endmsg;
      return is_good;
    }

    is_good = decodeBranchMemberNames();
    if ( !is_good ) {
      error() << "Unable to configure one or more branch members " << endmsg;
      return is_good;
    }

    // We have to "decode" members before calling base class initialize
    is_good = Sequence::initialize();
    if ( !is_good ) return is_good;

    // Loop over all branches
    // (Sequence does not know about branches)
    for ( auto& alg : branchAlgorithms() ) {
      is_good = alg->sysInitialize();
      if ( is_good.isFailure() ) {
        error() << "Unable to initialize Algorithm " << alg->name() << endmsg;
        return is_good;
      }
    }

    return is_good;
  }

  StatusCode Sequencer::reinitialize() {
    // Bypass the loop if this sequencer is disabled
    if ( isEnabled() ) {
      // Loop over all branch members calling their reinitialize functions
      // if they are not disabled.
      for ( auto& alg : branchAlgorithms() ) {
        if ( alg->isEnabled() ) { alg->reinitialize().ignore(); }
      }
      return Sequence::reinitialize();
    }
    return StatusCode::SUCCESS;
  }

  StatusCode Sequencer::execute( const EventContext& ctx ) const {
    StatusCode result = StatusCode::SUCCESS;
    ON_DEBUG   debug() << name() << " Sequencer::execute()" << endmsg;

    auto state = execState( ctx );

    // Bypass the loop if this sequencer is disabled or has already been executed
    if ( isEnabled() && !( state.state() == AlgExecState::Done ) ) {
      Gaudi::Algorithm* lastAlgorithm;
      result = execute( ctx, *subAlgorithms(), m_isInverted, lastAlgorithm );
      if ( result.isSuccess() ) {
        const bool passed = state.filterPassed();
        if ( !passed && m_shortCircuit ) {

          // Filter failed and stop override not set. Execute the
          // branch if there is one associated with the filter
          // algorithm that failed. Note that the first member on
          // the branch is the failing algorithm and so should
          // be skipped.
          const auto& theAlgs = branchAlgorithms();
          if ( !theAlgs.empty() ) {
            Gaudi::Algorithm* branchAlgorithm = theAlgs[0];
            if ( lastAlgorithm == branchAlgorithm ) {

              // Branch specified - Loop over branch members
              result = execute( ctx, branchAlgorithms(), m_isBranchInverted, lastAlgorithm, 1 );
              if ( result.isSuccess() ) {

                // The final filter passed state will be set true if either
                // of the main or branches passed, otherwise false.

                // Save the branch  filter passed state.
                setBranchFilterPassed( ctx, state.filterPassed() );
              }
            }
          }
        }
      }

      // Prevent multiple executions of this sequencer for the current event
      state.setState( AlgExecState::Done );
    }
    return result;
  }

  StatusCode Sequencer::finalize() {
    // Loop over all branch members calling their finalize functions
    // if they are not disabled. Note that the Sequence::finalize
    // function already does this for the main members.
    for ( auto& alg : branchAlgorithms() ) {
      if ( alg->sysFinalize().isFailure() ) { error() << "Unable to finalize Algorithm " << alg->name() << endmsg; }
    }
    return Sequence::finalize();
  }

  StatusCode Sequencer::start() {
    auto is_good = Sequence::start();
    if ( !is_good ) return is_good;

    // Loop over all branches
    for ( auto& alg : branchAlgorithms() ) {
      is_good = alg->sysStart();
      if ( !is_good ) {
        error() << "Unable to start Algorithm " << alg->name() << endmsg;
        return is_good;
      }
    }

    return is_good;
  }

  StatusCode Sequencer::stop() {
    // Loop over all branch members calling their finalize functions
    // if they are not disabled.
    for ( auto& alg : branchAlgorithms() ) {
      if ( alg->sysStop().isFailure() ) { error() << "Unable to stop Algorithm " << alg->name() << endmsg; }
    }
    return Sequence::stop();
  }

  bool Sequencer::branchFilterPassed( const EventContext& ctx ) const {
    auto lock = std::scoped_lock{ m_branchFilterMutex };
    return m_branchFilterPassed[ctx.slot()];
  }

  void Sequencer::setBranchFilterPassed( const EventContext& ctx, bool state ) const {
    auto lock                        = std::scoped_lock{ m_branchFilterMutex };
    m_branchFilterPassed[ctx.slot()] = state;
  }

  StatusCode Sequencer::append( Gaudi::Algorithm* pAlgorithm ) { return append( pAlgorithm, *subAlgorithms() ); }

  StatusCode Sequencer::appendToBranch( Gaudi::Algorithm* pAlgorithm ) {
    return append( pAlgorithm, branchAlgorithms() );
  }

  StatusCode Sequencer::createAndAppend( const std::string& type, const std::string& name,
                                         Gaudi::Algorithm*& pAlgorithm ) {
    return createAndAppend( type, name, pAlgorithm, *subAlgorithms() );
  }

  StatusCode Sequencer::createAndAppendToBranch( const std::string& type, const std::string& name,
                                                 Gaudi::Algorithm*& pAlgorithm ) {
    return createAndAppend( type, name, pAlgorithm, branchAlgorithms() );
  }

  StatusCode Sequencer::remove( Gaudi::Algorithm* pAlgorithm ) { return remove( pAlgorithm->name() ); }

  StatusCode Sequencer::remove( const std::string& algname ) { return remove( algname, *subAlgorithms() ); }

  StatusCode Sequencer::removeFromBranch( Gaudi::Algorithm* pAlgorithm ) {
    return removeFromBranch( pAlgorithm->name() );
  }

  StatusCode Sequencer::removeFromBranch( const std::string& algname ) { return remove( algname, branchAlgorithms() ); }

  const std::vector<Gaudi::Algorithm*>& Sequencer::branchAlgorithms() const { return m_branchAlgs; }

  std::vector<Gaudi::Algorithm*>& Sequencer::branchAlgorithms() { return m_branchAlgs; }

  StatusCode Sequencer::decodeMemberNames() {
    // Decode the membership list
    return decodeNames( m_names, *subAlgorithms(), m_isInverted );
  }

  StatusCode Sequencer::decodeBranchMemberNames() {
    // Decode the branch membership list
    return decodeNames( m_branchNames, branchAlgorithms(), m_isBranchInverted );
  }

  /**
   ** Protected Member Functions
   **/

  StatusCode Sequencer::append( Gaudi::Algorithm* pAlgorithm, std::vector<Gaudi::Algorithm*>& theAlgs ) {
    // Check that the specified algorithm doesn't already exist in the membership list
    if ( std::find( std::begin( theAlgs ), std::end( theAlgs ), pAlgorithm ) != std::end( theAlgs ) ) {
      return StatusCode::FAILURE;
    }
    theAlgs.push_back( pAlgorithm );
    pAlgorithm->addRef();
    return StatusCode::SUCCESS;
  }

  StatusCode Sequencer::createAndAppend( const std::string& type, const std::string& algName,
                                         Gaudi::Algorithm*& pAlgorithm, std::vector<Gaudi::Algorithm*>& theAlgs ) {
    auto theAlgMgr = serviceLocator()->service<IAlgManager>( "ApplicationMgr" );
    if ( !theAlgMgr ) return StatusCode::FAILURE;

    IAlgorithm* tmp;
    StatusCode  result = theAlgMgr->createAlgorithm( type, algName, tmp );
    if ( result.isSuccess() ) {
      try {
        pAlgorithm = dynamic_cast<Gaudi::Algorithm*>( tmp );
        theAlgs.push_back( pAlgorithm );
      } catch ( ... ) {
        error() << "Unable to create Algorithm " << algName << endmsg;
        result = StatusCode::FAILURE;
      }
    }

    return result;
  }

  StatusCode Sequencer::decodeNames( Gaudi::Property<std::vector<std::string>>& theNames,
                                     std::vector<Gaudi::Algorithm*>& theAlgs, std::vector<bool>& theLogic ) {
    StatusCode result;
    auto       theAlgMgr = serviceLocator()->service<IAlgManager>( "ApplicationMgr" );
    if ( theAlgMgr ) {
      // Clear the existing list of algorithms
      theAlgs.clear();

      // Build the list of member algorithms from the contents of the
      // theNames list.
      for ( const auto& n : theNames.value() ) {

        // Parse the name for a syntax of the form:
        //
        // <type>/<name>
        //
        // Where <name> is the algorithm instance name, and <type> is the
        // algorithm class type (being a subclass of Algorithm).
        const Gaudi::Utils::TypeNameString typeName( n );
        std::string                        theName = typeName.name();
        std::string                        theType = typeName.type();

        // Parse the name for a syntax of the form:
        //
        // <name>:invert
        //
        // Where <name> is the algorithm instance name and ":invert"
        // indicates that the filter passed logic is inverted.
        bool                   isInverted = false;
        std::string::size_type invert     = theName.find_first_of( ":" );
        // Skip all occurrences of "::" (allow namespaces)
        while ( std::string::npos != invert && invert < ( theName.size() - 1 ) && theName[invert + 1] == ':' )
          invert = theName.find_first_of( ":", invert + 2 );
        if ( std::string::npos != invert ) {
          if ( theName == theType ) {
            // This means that we got something like "Type:invert",
            // so we have to strip the ":invert" from the type too.
            theType.resize( invert );
          }
          theName.resize( invert );
          isInverted = true;
        }
        // Check whether the supplied name corresponds to an existing
        // Algorithm object.
        SmartIF<IAlgorithm>& theIAlg      = theAlgMgr->algorithm( theName, false );
        Gaudi::Algorithm*    theAlgorithm = nullptr;
        StatusCode           status       = StatusCode::SUCCESS;
        if ( theIAlg ) {
          try {
            theAlgorithm = dynamic_cast<Gaudi::Algorithm*>( theIAlg.get() );
          } catch ( ... ) {
            warning() << theName << " is not an Algorithm - Failed dynamic cast" << endmsg;
            theAlgorithm = nullptr; // release
          }
        }
        if ( theAlgorithm ) {

          // The specified Algorithm already exists - just append it to the membership list.
          status = append( theAlgorithm, theAlgs );
          if ( status.isSuccess() ) {
            ON_DEBUG debug() << theName << " already exists - appended to member list" << endmsg;
          } else {
            warning() << theName << " already exists - append failed!!!" << endmsg;
            result = StatusCode::FAILURE;
          }
        } else {

          // The specified name doesn't exist - create a new object of the specified type
          // and append it to the membership list.
          status = createAndAppend( theType, theName, theAlgorithm, theAlgs );
          if ( status.isSuccess() ) {
            ON_DEBUG debug() << theName << " doesn't exist - created and appended to member list" << endmsg;
          } else {
            warning() << theName << " doesn't exist - creation failed!!!" << endmsg;
            result = StatusCode::FAILURE;
          }
        }
        if ( status.isSuccess() ) theLogic.push_back( isInverted );
      }
    }
    // Print membership list
    if ( result.isSuccess() && theAlgs.size() != 0 ) {
      info() << "Member list: ";
      auto ai = theAlgs.begin();
      auto li = theLogic.begin();
      for ( ; ai != theAlgs.end(); ++ai, ++li ) {

        if ( ai != theAlgs.begin() ) info() << ", ";
        auto alg = *ai;
        if ( alg->name() == System::typeinfoName( typeid( *alg ) ) )
          info() << alg->name();
        else
          info() << System::typeinfoName( typeid( *alg ) ) << "/" << alg->name();

        if ( *li ) info() << ":invert";
      }
      info() << endmsg;
    }
    return result;
  }

  StatusCode Sequencer::execute( const EventContext& ctx, const std::vector<Gaudi::Algorithm*>& theAlgs,
                                 const std::vector<bool>& theLogic, Gaudi::Algorithm*& lastAlgorithm,
                                 std::size_t first ) const {
    StatusCode result = StatusCode::SUCCESS;

    auto state = execState( ctx );
    state.setFilterPassed( !m_modeOR ); //  for OR, result will be false, unless (at least) one is true
                                        //  for AND, result will be true, unless (at least) one is false
                                        //    also see comment below ....)

    // Reset the branch filter passed flag
    setBranchFilterPassed( ctx, false );

    auto exists = [&]( const std::string_view loc ) -> bool {
      DataObject* tmp{ nullptr };
      return evtSvc()->retrieveObject( loc, tmp ).isSuccess();
    };

    // Do not execute if one or more of the m_vetoObjs exist in TES
    if ( const auto it = find_if( begin( m_vetoObjs ), end( m_vetoObjs ), exists ); it != end( m_vetoObjs ) ) {
      if ( msgLevel( MSG::DEBUG ) ) debug() << *it << " found, skipping event " << endmsg;
      return result;
    }

    // Execute if m_requireObjs is empty
    // or if one or more of the m_requireObjs exist in TES
    if ( !( m_requireObjs.empty() || any_of( begin( m_requireObjs ), end( m_requireObjs ), exists ) ) ) {
      return result;
    }

    // Loop over all algorithms calling their execute functions if they
    // are (a) not disabled, and (b) aren't already executed. Note that
    // in the latter case the filter state is still examined. Terminate
    // the loop if an algorithm indicates that it's filter didn't pass.
    auto size = theAlgs.size();
    for ( auto i = first; i < size; i++ ) {
      lastAlgorithm = theAlgs[i];
      result        = executeMember( lastAlgorithm, ctx );
      if ( result.isSuccess() ) {
        if ( !m_ignoreFilter ) {
          // Take the filter passed status of this algorithm as my own status.
          // Note that we take into account inverted logic.
          bool passed     = lastAlgorithm->execState( ctx ).filterPassed();
          bool isInverted = theLogic[i];
          if ( isInverted ) passed = !passed;

          // in OR mode, we don't care about things
          // which are false, as they leave our current state alone (provided
          // we stared as 'false'!), and in AND mode, we keep our current
          // state until someone returns 'false' (provided we started as 'true')
          if ( m_modeOR ? passed : !passed ) {
            state.setFilterPassed( m_modeOR );
            if ( m_shortCircuit ) { break; }
          }
        }
      } else {
        break;
      }
    }
    if ( m_invert ) state.setFilterPassed( !state.filterPassed() );
    return result;
  }

  StatusCode Sequencer::executeMember( Gaudi::Algorithm* theAlgorithm, const EventContext& context ) const {
    StatusCode result = StatusCode::SUCCESS;
    if ( theAlgorithm->isEnabled() ) {
      if ( theAlgorithm->execState( context ).state() != AlgExecState::Done ) {
        result = theAlgorithm->sysExecute( context );
      }
    }
    return result;
  }

  StatusCode Sequencer::remove( const std::string& algname, std::vector<Gaudi::Algorithm*>& theAlgs ) {
    StatusCode result = StatusCode::FAILURE;

    // Test that the algorithm exists in the member list
    for ( auto& alg : theAlgs ) {
      if ( alg->name() == algname ) {

        // Algorithm with specified name exists in the algorithm list - remove it
        // THIS ISN'T IMPLEMENTED YET!!!!
        info() << "Sequencer::remove( ) isn't implemented yet!!!!!" << endmsg;
        result = StatusCode::SUCCESS;
        break;
      }
    }
    return result;
  }

  std::ostream& Sequencer::toControlFlowExpression( std::ostream& os ) const {
    if ( m_invert ) os << '~';

    auto& theAlgs = *subAlgorithms();
    if ( theAlgs.empty() ) return os << ( ( !m_modeOR ) ? "CFTrue" : "CFFalse" );

    // if we have only one element, we do not need a name
    if ( theAlgs.size() > 1 ) os << "seq(";

    const auto algs_count = theAlgs.size();
    const auto op         = m_shortCircuit ? ( m_modeOR ? " | " : " & " ) : " >> ";
    size_t     i          = 0;
    while ( i < algs_count ) {
      if ( i ) os << op;
      if ( m_isInverted[i] ) os << '~';
      theAlgs[i]->toControlFlowExpression( os );
      ++i;
    }
    if ( theAlgs.size() > 1 ) os << ')';
    return os;
  }
} // namespace Gaudi
