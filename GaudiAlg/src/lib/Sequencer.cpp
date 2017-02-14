// Sequencer class
// Implements:
// 1) Common functionality of IInterface
// 2) Default behavior for the IAlgorithm

#include "GaudiAlg/Sequencer.h"

#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Stat.h"

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

/**
 ** Constructor(s)
 **/
Sequencer::Sequencer( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator )
{
  // Associate action handlers with the "Members" and "BranchMembers" properties
  m_names.declareUpdateHandler( [this]( Gaudi::Details::PropertyBase& ) {
    if ( isInitialized() ) decodeMemberNames().ignore();
  } );
  m_branchNames.declareUpdateHandler( [this]( Gaudi::Details::PropertyBase& ) {
    if ( isInitialized() ) decodeBranchMemberNames().ignore();
  } );
}

StatusCode Sequencer::initialize()
{
  StatusCode result = StatusCode::SUCCESS;

  result = decodeMemberNames();
  if ( result.isFailure() ) {
    error() << "Unable to configure one or more sequencer members " << endmsg;
    return result;
  }
  result = decodeBranchMemberNames();
  if ( result.isFailure() ) {
    error() << "Unable to configure one or more branch members " << endmsg;
    return result;
  }

  // Loop over all sub-algorithms
  for ( auto& alg : *subAlgorithms() ) {
    result = alg->sysInitialize();
    if ( result.isFailure() ) {
      error() << "Unable to initialize Algorithm " << alg->name() << endmsg;
      return result;
    }
  }

  // Loop over all branches
  for ( auto& alg : branchAlgorithms() ) {
    result = alg->sysInitialize();
    if ( result.isFailure() ) {
      error() << "Unable to initialize Algorithm " << alg->name() << endmsg;
      return result;
    }
  }

  return result;
}

StatusCode Sequencer::reinitialize()
{
  // Bypass the loop if this sequencer is disabled
  if ( isEnabled() ) {

    // Loop over all members calling their reinitialize functions
    // if they are not disabled.
    for ( auto& alg : *subAlgorithms() ) {
      if ( alg->isEnabled() ) alg->reinitialize().ignore();
    }
    // Loop over all branch members calling their reinitialize functions
    // if they are not disabled.
    for ( auto& alg : branchAlgorithms() ) {
      if ( alg->isEnabled() ) {
        alg->reinitialize().ignore();
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode Sequencer::execute()
{
  StatusCode result = StatusCode::SUCCESS;
  ON_DEBUG debug() << name() << " Sequencer::execute( )" << endmsg;

  // Bypass the loop if this sequencer is disabled or has already been executed
  if ( isEnabled() && !isExecuted() ) {
    Algorithm* lastAlgorithm;
    result = execute( *subAlgorithms(), m_isInverted, lastAlgorithm );
    if ( result.isSuccess() ) {
      bool passed = filterPassed();
      if ( !passed && !isStopOverride() ) {

        // Filter failed and stop override not set. Execute the
        // branch if there is one associated with the filter
        // algorithm that failed. Note that the first member on
        // the branch is the failing algorithm and so should
        // be skipped.
        const auto& theAlgs = branchAlgorithms();
        if ( !theAlgs.empty() ) {
          Algorithm* branchAlgorithm = theAlgs[0];
          if ( lastAlgorithm == branchAlgorithm ) {

            // Branch specified - Loop over branch members
            result = execute( branchAlgorithms(), m_isBranchInverted, lastAlgorithm, 1 );
            if ( result.isSuccess() ) {

              // The final filter passed state will be set true if either
              // of the main or branches passed, otherwise false.

              // Save the branch  filter passed state.
              setBranchFilterPassed( filterPassed() ).ignore();
            }
          }
        }
      }
    }

    // Prevent multiple executions of this sequencer for the current event
    setExecuted( true );
  }
  return result;
}

StatusCode Sequencer::finalize()
{
  // Loop over all branch members calling their finalize functions
  // if they are not disabled. Note that the Algorithm::sysFinalize
  // function already does this for the main members.
  for ( auto& alg : branchAlgorithms() ) {
    if ( alg->sysFinalize().isFailure() ) {
      error() << "Unable to finalize Algorithm " << alg->name() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode Sequencer::start()
{
  StatusCode result = StatusCode::SUCCESS;

  // Loop over all sub-algorithms
  for ( auto& alg : *subAlgorithms() ) {
    result = alg->sysStart();
    if ( result.isFailure() ) {
      error() << "Unable to start Algorithm " << alg->name() << endmsg;
      return result;
    }
  }

  // Loop over all branches
  for ( auto& alg : branchAlgorithms() ) {
    result = alg->sysStart();
    if ( result.isFailure() ) {
      error() << "Unable to start Algorithm " << alg->name() << endmsg;
      return result;
    }
  }

  return result;
}

StatusCode Sequencer::stop()
{
  // Loop over all branch members calling their finalize functions
  // if they are not disabled.

  for ( auto& alg : *subAlgorithms() ) {
    if ( alg->sysStop().isFailure() ) {
      error() << "Unable to stop Algorithm " << alg->name() << endmsg;
    }
  }

  for ( auto& alg : branchAlgorithms() ) {
    if ( alg->sysStop().isFailure() ) {
      error() << "Unable to stop Algorithm " << alg->name() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode Sequencer::beginRun()
{
  StatusCode result = StatusCode::SUCCESS;

  // Bypass the loop if this sequencer is disabled
  if ( isEnabled() ) {

    // Loop over all members calling their sysInitialize functions
    // if they are not disabled. Note that the Algoriithm::sysInitialize
    // function protects this from affecting Algorithms that have already
    // been initialized.
    for ( auto& alg : *subAlgorithms() ) {
      result = alg->sysInitialize();
      if ( result.isFailure() ) {
        error() << "Unable to initialize Algorithm " << alg->name() << endmsg;
        break;
      }
      result = alg->sysStart();
      if ( result.isFailure() ) {
        error() << "Unable to start Algorithm " << alg->name() << endmsg;
        break;
      }
    }

    // Loop over all members calling their beginRun functions
    // if they are not disabled.
    for ( auto& alg : *subAlgorithms() ) {
      if ( !alg->isEnabled() ) {
        alg->beginRun().ignore();
      }
    }

    // Loop over all branch members calling their sysInitialize functions
    // if they are not disabled. Note that the Algoriithm::sysInitialize
    // function protects this from affecting Algorithms that have already
    // been initialized.
    for ( auto& alg : branchAlgorithms() ) {
      result = alg->sysInitialize();
      if ( result.isFailure() ) {
        error() << "Unable to initialize Algorithm " << alg->name() << endmsg;
        break;
      }
      result = alg->sysStart();
      if ( result.isFailure() ) {
        error() << "Unable to start Algorithm " << alg->name() << endmsg;
        break;
      }
    }

    // Loop over all branch members calling their beginRun functions
    // if they are not disabled.
    for ( auto& alg : branchAlgorithms() ) {
      if ( !alg->isEnabled() ) {
        alg->beginRun().ignore();
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode Sequencer::endRun()
{
  // Bypass the loop if this sequencer is disabled
  if ( isEnabled() ) {

    // Loop over all members calling their endRun functions
    // if they are not disabled.
    for ( auto& alg : *subAlgorithms() ) {
      if ( !alg->isEnabled() ) alg->endRun().ignore();
    }
    // Loop over all branch members calling their endRun functions
    // if they are not disabled.
    for ( auto& alg : branchAlgorithms() ) {
      if ( !alg->isEnabled() ) alg->endRun().ignore();
    }
  }
  return StatusCode::SUCCESS;
}

void Sequencer::resetExecuted()
{
  Algorithm::resetExecuted();

  // Loop over all members calling their resetExecuted functions
  // if they are not disabled.
  for ( auto& alg : *subAlgorithms() ) alg->resetExecuted();

  // Loop over all branch members calling their resetExecuted functions
  // if they are not disabled.
  for ( auto& alg : branchAlgorithms() ) alg->resetExecuted();

  // Reset the branch filter passed flag
  m_branchFilterPassed = false;
}

bool Sequencer::branchFilterPassed() const { return m_branchFilterPassed; }

StatusCode Sequencer::setBranchFilterPassed( bool state )
{
  m_branchFilterPassed = state;
  return StatusCode::SUCCESS;
}

bool Sequencer::isStopOverride() const { return m_stopOverride.value(); }

StatusCode Sequencer::append( Algorithm* pAlgorithm ) { return append( pAlgorithm, *subAlgorithms() ); }

StatusCode Sequencer::appendToBranch( Algorithm* pAlgorithm ) { return append( pAlgorithm, branchAlgorithms() ); }

StatusCode Sequencer::createAndAppend( const std::string& type, const std::string& name, Algorithm*& pAlgorithm )
{
  return createAndAppend( type, name, pAlgorithm, *subAlgorithms() );
}

StatusCode Sequencer::createAndAppendToBranch( const std::string& type, const std::string& name,
                                               Algorithm*& pAlgorithm )
{
  return createAndAppend( type, name, pAlgorithm, branchAlgorithms() );
}

StatusCode Sequencer::remove( Algorithm* pAlgorithm ) { return remove( pAlgorithm->name() ); }

StatusCode Sequencer::remove( const std::string& algname ) { return remove( algname, *subAlgorithms() ); }

StatusCode Sequencer::removeFromBranch( Algorithm* pAlgorithm ) { return removeFromBranch( pAlgorithm->name() ); }

StatusCode Sequencer::removeFromBranch( const std::string& algname ) { return remove( algname, branchAlgorithms() ); }

const std::vector<Algorithm*>& Sequencer::branchAlgorithms() const { return m_branchAlgs; }

std::vector<Algorithm*>& Sequencer::branchAlgorithms() { return m_branchAlgs; }

StatusCode Sequencer::decodeMemberNames()
{
  // Decode the membership list
  return decodeNames( m_names, *subAlgorithms(), m_isInverted );
}

StatusCode Sequencer::decodeBranchMemberNames()
{
  // Decode the branch membership list
  return decodeNames( m_branchNames, branchAlgorithms(), m_isBranchInverted );
}

/**
 ** Protected Member Functions
 **/

StatusCode Sequencer::append( Algorithm* pAlgorithm, std::vector<Algorithm*>& theAlgs )
{
  // Check that the specified algorithm doesn't already exist in the membership list
  if ( std::find( std::begin( theAlgs ), std::end( theAlgs ), pAlgorithm ) != std::end( theAlgs ) ) {
    return StatusCode::FAILURE;
  }
  theAlgs.push_back( pAlgorithm );
  pAlgorithm->addRef();
  return StatusCode::SUCCESS;
}

StatusCode Sequencer::createAndAppend( const std::string& type, const std::string& algName, Algorithm*& pAlgorithm,
                                       std::vector<Algorithm*>& theAlgs )
{
  auto theAlgMgr = serviceLocator()->service<IAlgManager>( "ApplicationMgr" );
  if ( !theAlgMgr ) return StatusCode::FAILURE;

  IAlgorithm* tmp;
  StatusCode result = theAlgMgr->createAlgorithm( type, algName, tmp );
  if ( result.isSuccess() ) {
    try {
      pAlgorithm = dynamic_cast<Algorithm*>( tmp );
      theAlgs.push_back( pAlgorithm );
    } catch ( ... ) {
      error() << "Unable to create Algorithm " << algName << endmsg;
      result = StatusCode::FAILURE;
    }
  }

  return result;
}

StatusCode Sequencer::decodeNames( Gaudi::Property<std::vector<std::string>>& theNames,
                                   std::vector<Algorithm*>& theAlgs, std::vector<bool>& theLogic )
{
  StatusCode result;
  auto theAlgMgr = serviceLocator()->service<IAlgManager>( "ApplicationMgr" );
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
      std::string theName = typeName.name();
      std::string theType = typeName.type();

      // Parse the name for a syntax of the form:
      //
      // <name>:invert
      //
      // Where <name> is the algorithm instance name and ":invert"
      // indicates that the filter passed logic is inverted.
      bool isInverted               = false;
      std::string::size_type invert = theName.find_first_of( ":" );
      // Skip all occurrences of "::" (allow namespaces)
      while ( std::string::npos != invert && invert < ( theName.size() - 1 ) && theName[invert + 1] == ':' )
        invert = theName.find_first_of( ":", invert + 2 );
      if ( std::string::npos != invert ) {
        if ( theName == theType ) {
          // This means that we got something like "Type:invert",
          // so we have to strip the ":invert" from the type too.
          theType = theType.substr( 0, invert );
        }
        theName    = theName.substr( 0, invert );
        isInverted = true;
      }
      // Check whether the supplied name corresponds to an existing
      // Algorithm object.
      SmartIF<IAlgorithm>& theIAlg = theAlgMgr->algorithm( theName, false );
      Algorithm* theAlgorithm      = nullptr;
      StatusCode status            = StatusCode::SUCCESS;
      if ( theIAlg ) {
        try {
          theAlgorithm = dynamic_cast<Algorithm*>( theIAlg.get() );
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

StatusCode Sequencer::execute( const std::vector<Algorithm*>& theAlgs, std::vector<bool>& theLogic,
                               Algorithm*& lastAlgorithm, unsigned int first )
{
  StatusCode result = StatusCode::SUCCESS;

  // Loop over all algorithms calling their execute functions if they
  // are (a) not disabled, and (b) aren't already executed. Note that
  // in the latter case the filter state is still examined. Terminate
  // the loop if an algorithm indicates that it's filter didn't pass.
  unsigned int size = theAlgs.size();
  for ( unsigned int i = first; i < size; i++ ) {
    lastAlgorithm = theAlgs[i];
    result        = executeMember( lastAlgorithm );
    if ( result.isSuccess() ) {

      // Take the filter passed status of this algorithm as my own status.
      // Note that we take into account inverted logic.
      bool passed              = lastAlgorithm->filterPassed();
      bool isInverted          = theLogic[i];
      if ( isInverted ) passed = !passed;
      setFilterPassed( passed );

      // The behaviour when the filter fails depends on the StopOverride property.
      // The default action is to stop processing, but this default can be
      // overridden by setting the "StopOverride" property to true.
      if ( !isStopOverride() ) {
        if ( !passed ) break;
      }
    } else {
      break;
    }
  }
  return result;
}

StatusCode Sequencer::executeMember( Algorithm* theAlgorithm )
{
  StatusCode result = StatusCode::SUCCESS;
  if ( theAlgorithm->isEnabled( ) ) {
    if ( ! theAlgorithm->isExecuted( ) ) {
      result = theAlgorithm->sysExecute( getContext() );

      // Set the executed state of the algorithm.
      // I think this should be done by the algorithm itself, but just in case...
      theAlgorithm->setExecuted( true );
    }
  }
  return result;
}

StatusCode Sequencer::remove( const std::string& algname, std::vector<Algorithm*>& theAlgs )
{
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

std::ostream& Sequencer::toControlFlowExpression(std::ostream& os) const {
  auto &theAlgs = *subAlgorithms();
  if (theAlgs.empty()) return os << "CFTrue";

  os << "seq(";
  const auto algs_count = theAlgs.size();
  const auto op = isStopOverride() ? " >> " : " & ";
  size_t i = 0;
  while (i < algs_count) {
    if (i) os << op;
    if (m_isInverted[i]) os << "~";
    theAlgs[i]->toControlFlowExpression(os);
    ++i;
  }
  return os << ")";
}
