// Sequencer class
// Implements:
// 1) Common functionality of IInterface
// 2) Default behavior for the IAlgorithm

#include "GaudiAlg/Sequencer.h"

#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/GaudiException.h"

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

/**
 ** Constructor(s)
 **/
Sequencer::Sequencer( const std::string& name, ISvcLocator* pSvcLocator )
: Algorithm( name, pSvcLocator ),
  m_branchFilterPassed( false )
{

  // Create vector of branch algorithms
  m_branchAlgs = new std::vector<Algorithm*>();

  // Declare Sequencer properties with their defaults
  declareProperty( "Members", m_names );
  declareProperty( "BranchMembers", m_branchNames );
  declareProperty( "StopOverride", m_stopOverride=false );

  // Associate action handlers with the "Members" and "BranchMembers" properties
  m_names.declareUpdateHandler      ( &Sequencer::membershipHandler      , this );
  m_branchNames.declareUpdateHandler( &Sequencer::branchMembershipHandler, this );

}

/**
 ** Destructor
 **/
Sequencer::~Sequencer()
{
  delete m_branchAlgs;
}

StatusCode
Sequencer::initialize()
{
  StatusCode result = StatusCode::SUCCESS;
  MsgStream log( msgSvc( ), name( ) );

  std::vector<Algorithm*>* theAlgs;
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend;

  result = decodeMemberNames();
  if( result.isFailure() ) {
    log << MSG::ERROR << "Unable to configure one or more sequencer members " << endmsg;
    return result;
  }
  result = decodeBranchMemberNames();
  if( result.isFailure() ) {
    log << MSG::ERROR << "Unable to configure one or more branch members " << endmsg;
    return result;
  }

  // Loop over all sub-algorithms
  theAlgs = subAlgorithms( );
  itend   = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    result = theAlgorithm->sysInitialize( );
    if( result.isFailure() ) {
      log << MSG::ERROR << "Unable to initialize Algorithm " << theAlgorithm->name() << endmsg;
      return result;
    }
  }

  // Loop over all branches
  theAlgs = branchAlgorithms( );
  itend   = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    result = theAlgorithm->sysInitialize( );
    if( result.isFailure() ) {
      log << MSG::ERROR << "Unable to initialize Algorithm " << theAlgorithm->name() << endmsg;
      return result;
    }
  }

  return result;
}

StatusCode
Sequencer::reinitialize()
{
  // Bypass the loop if this sequencer is disabled
  if ( isEnabled( ) ) {

    // Loop over all members calling their reinitialize functions
    // if they are not disabled.
    std::vector<Algorithm*>* theAlgms = subAlgorithms( );
    std::vector<Algorithm*>::iterator it;
    std::vector<Algorithm*>::iterator itend = theAlgms->end( );
    for (it = theAlgms->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      if ( ! theAlgorithm->isEnabled( ) ) {
        theAlgorithm->reinitialize( ).ignore();
      }
    }
    // Loop over all branch members calling their reinitialize functions
    // if they are not disabled.
    theAlgms = branchAlgorithms( );
    itend    = theAlgms->end( );
    for (it = theAlgms->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      if ( ! theAlgorithm->isEnabled( ) ) {
        theAlgorithm->reinitialize( ).ignore();
      }
    }

  }
  return StatusCode::SUCCESS;
}

StatusCode
Sequencer::execute()
{
  StatusCode result = StatusCode::SUCCESS;
  MsgStream log( msgSvc( ), name( ) );

  ON_DEBUG log << MSG::DEBUG << name( ) << " Sequencer::execute( )" << endmsg;

  // Bypass the loop if this sequencer is disabled or has already been executed
  if ( isEnabled( ) && ! isExecuted( ) ) {
    Algorithm* lastAlgorithm;
    result = execute( subAlgorithms( ), m_isInverted, lastAlgorithm );
    if ( result.isSuccess( ) ) {
      bool passed = filterPassed( );
      if ( ! passed && ! isStopOverride( ) ) {

        // Filter failed and stop override not set. Execute the
        // branch if there is one associated with the filter
        // algorithm that failed. Note that the first member on
        // the branch is the failing algorithm and so should
        // be skipped.
        std::vector<Algorithm*>* theAlgs = branchAlgorithms( );
        if ( theAlgs->size( ) > 0 ) {
          Algorithm* branchAlgorithm = (*theAlgs)[0];
          if ( lastAlgorithm == branchAlgorithm ) {

            // Branch specified - Loop over branch members
            result = execute( branchAlgorithms( ),
                              m_isBranchInverted,
                              lastAlgorithm, 1 );
            if ( result.isSuccess( ) ) {

              // The final filter passed state will be set true if either
              // of the main or branches passed, otherwise false.

              // Save the branch  filter passed state.
              setBranchFilterPassed( filterPassed( ) ).ignore();
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

StatusCode
Sequencer::finalize()
{
  // Loop over all branch members calling their finalize functions
  // if they are not disabled. Note that the Algorithm::sysFinalize
  // function already does this for the main members.
  std::vector<Algorithm*>* theAlgs = branchAlgorithms( );
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    if (theAlgorithm->sysFinalize( ).isFailure()) {
      MsgStream log( msgSvc( ), name( ) );
      log << MSG::ERROR << "Unable to finalize Algorithm "
          << theAlgorithm->name() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
Sequencer::start()
{
  StatusCode result = StatusCode::SUCCESS;
  MsgStream log( msgSvc( ), name( ) );

  std::vector<Algorithm*>* theAlgs;
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend;

  // Loop over all sub-algorithms
  theAlgs = subAlgorithms( );
  itend   = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    result = theAlgorithm->sysStart( );
    if( result.isFailure() ) {
      log << MSG::ERROR << "Unable to start Algorithm " << theAlgorithm->name() << endmsg;
      return result;
    }
  }

  // Loop over all branches
  theAlgs = branchAlgorithms( );
  itend   = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    result = theAlgorithm->sysStart( );
    if( result.isFailure() ) {
      log << MSG::ERROR << "Unable to start Algorithm " << theAlgorithm->name() << endmsg;
      return result;
    }
  }

  return result;
}

StatusCode
Sequencer::stop()
{
  // Loop over all branch members calling their finalize functions
  // if they are not disabled.
  std::vector<Algorithm*>* theAlgs;
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend;

  theAlgs = subAlgorithms( );
  itend   = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    if (theAlgorithm->sysStop( ).isFailure()) {
      MsgStream log( msgSvc( ), name( ) );
      log << MSG::ERROR << "Unable to stop Algorithm "
          << theAlgorithm->name() << endmsg;
    }
  }

  theAlgs = branchAlgorithms( );
  itend   = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    if (theAlgorithm->sysStop( ).isFailure()) {
      MsgStream log( msgSvc( ), name( ) );
      log << MSG::ERROR << "Unable to stop Algorithm "
          << theAlgorithm->name() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
Sequencer::beginRun()
{
  StatusCode result = StatusCode::SUCCESS;
  MsgStream log( msgSvc( ), name( ) );

  // Bypass the loop if this sequencer is disabled
  if ( isEnabled( ) ) {

    // Loop over all members calling their sysInitialize functions
    // if they are not disabled. Note that the Algoriithm::sysInitialize
    // function protects this from affecting Algorithms that have already
    // been initialized.
    std::vector<Algorithm*>* theAlgs = subAlgorithms( );
    std::vector<Algorithm*>::iterator it;
    std::vector<Algorithm*>::iterator itend = theAlgs->end( );
    for (it = theAlgs->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      result = theAlgorithm->sysInitialize( );
      if( result.isFailure() ) {
        log << MSG::ERROR << "Unable to initialize Algorithm " << theAlgorithm->name() << endmsg;
        break;
      }
      result = theAlgorithm->sysStart( );
      if( result.isFailure() ) {
        log << MSG::ERROR << "Unable to start Algorithm " << theAlgorithm->name() << endmsg;
        break;
      }
    }

    // Loop over all members calling their beginRun functions
    // if they are not disabled.
    for (it = theAlgs->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      if ( ! theAlgorithm->isEnabled( ) ) {
        theAlgorithm->beginRun( ).ignore();
      }
    }

    // Loop over all branch members calling their sysInitialize functions
    // if they are not disabled. Note that the Algoriithm::sysInitialize
    // function protects this from affecting Algorithms that have already
    // been initialized.
    theAlgs = branchAlgorithms( );
    itend   = theAlgs->end( );
    for (it = theAlgs->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      result = theAlgorithm->sysInitialize( );
      if( result.isFailure() ) {
        log << MSG::ERROR << "Unable to initialize Algorithm " << theAlgorithm->name() << endmsg;
        break;
      }
      result = theAlgorithm->sysStart( );
      if( result.isFailure() ) {
        log << MSG::ERROR << "Unable to start Algorithm " << theAlgorithm->name() << endmsg;
        break;
      }
    }

    // Loop over all branch members calling their beginRun functions
    // if they are not disabled.
    for (it = theAlgs->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      if ( ! theAlgorithm->isEnabled( ) ) {
        theAlgorithm->beginRun( ).ignore();
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
Sequencer::endRun()
{
  // Bypass the loop if this sequencer is disabled
  if ( isEnabled( ) ) {

    // Loop over all members calling their endRun functions
    // if they are not disabled.
    std::vector<Algorithm*>* theAlgms = subAlgorithms( );
    std::vector<Algorithm*>::iterator it;
    std::vector<Algorithm*>::iterator itend = theAlgms->end( );
    for (it = theAlgms->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      if ( ! theAlgorithm->isEnabled( ) ) {
        theAlgorithm->endRun( ).ignore();
      }
    }
    // Loop over all branch members calling their endRun functions
    // if they are not disabled.
    theAlgms = branchAlgorithms( );
    itend    = theAlgms->end( );
    for (it = theAlgms->begin(); it != itend; it++) {
      Algorithm* theAlgorithm = (*it);
      if ( ! theAlgorithm->isEnabled( ) ) {
        theAlgorithm->endRun( ).ignore();
      }
    }
  }
  return StatusCode::SUCCESS;
}

void
Sequencer::resetExecuted( )
{
  Algorithm::resetExecuted( );

  // Loop over all members calling their resetExecuted functions
  // if they are not disabled.
  std::vector<Algorithm*>* subAlgms = subAlgorithms( );
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend = subAlgms->end( );
  for (it = subAlgms->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    theAlgorithm->resetExecuted( );
  }

  // Loop over all branch members calling their resetExecuted functions
  // if they are not disabled.
  subAlgms = branchAlgorithms( );
  itend    = subAlgms->end( );
  for (it = subAlgms->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    theAlgorithm->resetExecuted( );
  }

  // Reset the branch filter passed flag
  m_branchFilterPassed = false;
}

bool
Sequencer::branchFilterPassed( ) const
{
  return m_branchFilterPassed;
}

StatusCode
Sequencer::setBranchFilterPassed( bool state )
{
  m_branchFilterPassed = state;
  return StatusCode::SUCCESS;
}

bool
Sequencer::isStopOverride( ) const
{
  return m_stopOverride.value( );
}

StatusCode
Sequencer::append( Algorithm* pAlgorithm )
{
  StatusCode result = append( pAlgorithm, subAlgorithms( ) );
  return result;
}

StatusCode
Sequencer::appendToBranch( Algorithm* pAlgorithm )
{
  StatusCode result = append( pAlgorithm, branchAlgorithms( ) );
  return result;
}

StatusCode
Sequencer::createAndAppend( const std::string& type,
                            const std::string& name,
                            Algorithm*& pAlgorithm )
{
  StatusCode result = createAndAppend( type, name, pAlgorithm, subAlgorithms( ) );
  return result;
}

StatusCode
Sequencer::createAndAppendToBranch( const std::string& type,
                                    const std::string& name,
                                    Algorithm*& pAlgorithm )
{
  StatusCode result = createAndAppend( type, name, pAlgorithm, branchAlgorithms( ) );
  return result;
}

StatusCode
Sequencer::remove( Algorithm* pAlgorithm )
{
  std::string theName = pAlgorithm->name( );
  StatusCode result = remove( theName );
  return result;
}

StatusCode
Sequencer::remove( const std::string& algname )
{
  StatusCode result = remove( algname, subAlgorithms( ) );
  return result;
}

StatusCode
Sequencer::removeFromBranch( Algorithm* pAlgorithm )
{
  std::string theName = pAlgorithm->name( );
  StatusCode result = removeFromBranch( theName );
  return result;
}

StatusCode
Sequencer::removeFromBranch( const std::string& algname )
{
  StatusCode result = remove( algname, branchAlgorithms( ) );
  return result;
}

std::vector<Algorithm*>*
Sequencer::branchAlgorithms( ) const {
  return m_branchAlgs;
}

StatusCode
Sequencer::decodeMemberNames( )
{
  StatusCode result = StatusCode::SUCCESS;

  // Decode the membership list
  result = decodeNames( m_names,
                        subAlgorithms( ),
                        m_isInverted );

  return result;
}

void
Sequencer::membershipHandler( Property& /* theProp */ )
{
  if ( isInitialized() ) decodeMemberNames();
}

StatusCode
Sequencer::decodeBranchMemberNames( )
{
  StatusCode result = StatusCode::SUCCESS;

  // Decode the branch membership list
  result = decodeNames( m_branchNames,
                        branchAlgorithms( ),
                        m_isBranchInverted );

  return result;
}

void
Sequencer::branchMembershipHandler( Property& /* theProp */ )
{
  if ( isInitialized() ) decodeBranchMemberNames();
}

/**
 ** Protected Member Functions
 **/

StatusCode
Sequencer::append( Algorithm* pAlgorithm,
                   std::vector<Algorithm*>* theAlgs )
{
  StatusCode result = StatusCode::SUCCESS;
  // Check that the specified algorithm doesn't already exist in the membership list
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    if ( theAlgorithm == pAlgorithm ) {
      result = StatusCode::FAILURE;
      break;
    }
  }
  if ( result.isSuccess( ) ) {
    theAlgs->push_back( pAlgorithm );
    pAlgorithm->addRef();
  }
  return result;
}

StatusCode
Sequencer::createAndAppend( const std::string& type,
	                        const std::string& algName,
	                        Algorithm*& pAlgorithm,
	                        std::vector<Algorithm*>* theAlgs )
{
  StatusCode result = StatusCode::FAILURE;
  MsgStream log( msgSvc( ), name( ) );
  SmartIF<IAlgManager> theAlgMgr(serviceLocator()->service("ApplicationMgr"));
  if ( theAlgMgr.isValid() ) {
    IAlgorithm* tmp;
    result = theAlgMgr->createAlgorithm( type, algName, tmp );
    if ( result.isSuccess( ) ) {
      try{
        pAlgorithm = dynamic_cast<Algorithm*>(tmp);
        theAlgs->push_back( pAlgorithm );
      } catch(...){
        log << MSG::ERROR << "Unable to create Algorithm " << algName << endmsg;
        result = StatusCode::FAILURE;
      }
    }
  }
  return result;
}

StatusCode
Sequencer::decodeNames( StringArrayProperty& theNames,
                        std::vector<Algorithm*>* theAlgs,
                        std::vector<bool>& theLogic )
{
  StatusCode result;
  MsgStream log( msgSvc( ), name( ) );
  SmartIF<IAlgManager> theAlgMgr(serviceLocator()->service("ApplicationMgr"));
  if ( theAlgMgr.isValid() ) {
    // Clear the existing list of algorithms
    theAlgs->clear( );

    // Build the list of member algorithms from the contents of the
    // theNames list.
    const std::vector<std::string>& theNameVector = theNames.value( );
    std::vector<std::string>::const_iterator it;
    std::vector<std::string>::const_iterator itend = theNameVector.end( );
    for (it = theNameVector.begin(); it != itend; it++) {

      // Parse the name for a syntax of the form:
      //
      // <type>/<name>
      //
      // Where <name> is the algorithm instance name, and <type> is the
      // algorithm class type (being a subclass of Algorithm).
      const Gaudi::Utils::TypeNameString typeName(*it);
      std::string theName = typeName.name();
      std::string theType = typeName.type();

      // Parse the name for a syntax of the form:
      //
      // <name>:invert
      //
      // Where <name> is the algorithm instance name and ":invert"
      // indicates that the filter passed logic is inverted.
      bool isInverted = false;
      std::string::size_type invert = theName.find_first_of( ":" );
      // Skip all occurrences of "::" (allow namespaces)
      while ( std::string::npos != invert
              && invert < (theName.size() - 1) && theName[invert+1] == ':' )
        invert = theName.find_first_of( ":", invert+2 );
      if ( std::string::npos != invert ) {
        if ( theName == theType ) {
          // This means that we got something like "Type:invert",
          // so we have to strip the ":invert" from the type too.
          theType = theType.substr( 0, invert );
        }
        theName = theName.substr( 0, invert );
        isInverted = true;
      }
      // Check whether the supplied name corresponds to an existing
      // Algorithm object.
      SmartIF<IAlgorithm>& theIAlg = theAlgMgr->algorithm(theName, false);
      Algorithm*  theAlgorithm = 0;
      StatusCode status = StatusCode::SUCCESS;
      if ( theIAlg.isValid() ) {
        try{
          theAlgorithm = dynamic_cast<Algorithm*>(theIAlg.get());
        } catch(...){
          log << MSG::WARNING << theName << " is not an Algorithm - Failed dynamic cast" << endmsg;
          theAlgorithm = 0; // release
        }
      }
      if ( theAlgorithm ) {

        // The specified Algorithm already exists - just append it to the membership list.
        status = append( theAlgorithm, theAlgs );
        if ( status.isSuccess( ) ) {
          ON_DEBUG log << MSG::DEBUG << theName << " already exists - appended to member list" << endmsg;
        } else {
          log << MSG::WARNING << theName << " already exists - append failed!!!" << endmsg;
          result = StatusCode::FAILURE;
        }
      } else {

        // The specified name doesn't exist - create a new object of the specified type
        // and append it to the membership list.
        status = createAndAppend( theType, theName, theAlgorithm, theAlgs );
        if ( status.isSuccess( ) ) {
          ON_DEBUG log << MSG::DEBUG << theName << " doesn't exist - created and appended to member list" << endmsg;
        } else {
          log << MSG::WARNING << theName << " doesn't exist - creation failed!!!" << endmsg;
          result = StatusCode::FAILURE;
        }
      }
      if ( status.isSuccess( ) ) {
        theLogic.push_back( isInverted );
      }
    }

  }
  // Print membership list
  if ( result.isSuccess() && theAlgs->size() != 0 ) {
    log << MSG::INFO << "Member list: ";
    std::vector<Algorithm*>::iterator ai = theAlgs->begin();
    std::vector<bool>::iterator li = theLogic.begin();
    for ( ; ai != theAlgs->end(); ++ai, ++li ) {

      if ( ai != theAlgs->begin() ) log << ", ";

      if ( (*ai)->name() == System::typeinfoName(typeid(**ai)) )
        log << (*ai)->name();
      else
        log << System::typeinfoName(typeid(**ai)) << "/" << (*ai)->name();

      if (*li) log << ":invert";
    }
    log << endmsg;
  }
  return result;
}

StatusCode
Sequencer::execute( std::vector<Algorithm*>* theAlgs,
                    std::vector<bool>& theLogic,
                    Algorithm*& lastAlgorithm,
                    unsigned int first )
{
  StatusCode result = StatusCode::SUCCESS;

  // Loop over all algorithms calling their execute functions if they
  // are (a) not disabled, and (b) aren't already executed. Note that
  // in the latter case the filter state is still examined. Terminate
  // the loop if an algorithm indicates that it's filter didn't pass.
  unsigned int size = theAlgs->size( );
  for (unsigned int i = first; i < size; i++) {
    lastAlgorithm = (*theAlgs)[i];
    result = executeMember( lastAlgorithm );
    if ( result.isSuccess( ) ) {

      // Take the filter passed status of this algorithm as my own status.
      // Note that we take into account inverted logic.
      bool passed = lastAlgorithm->filterPassed( );
      bool isInverted = theLogic[i];
      if ( isInverted ) {
        passed = ! passed;
      }
      setFilterPassed( passed );

      // The behaviour when the filter fails depends on the StopOverride property.
      // The default action is to stop processing, but this default can be
      // overridden by setting the "StopOverride" property to true.
      if ( ! isStopOverride( ) ) {
        if ( ! passed ) break;
      }
    } else {
      break;
    }
  }
  return result;
}

StatusCode
Sequencer::executeMember( Algorithm* theAlgorithm )
{
  StatusCode result = StatusCode::SUCCESS;
  if ( theAlgorithm->isEnabled( ) ) {
    if ( ! theAlgorithm->isExecuted( ) ) {
      result = theAlgorithm->sysExecute( );

      // Set the executed state of the algorithm.
      // I think this should be done by the algorithm itself, but just in case...
      theAlgorithm->setExecuted( true );
    }
  }
  return result;
}

StatusCode
Sequencer::remove( const std::string& algname, std::vector<Algorithm*>* theAlgs )
{
  MsgStream log( msgSvc( ), name( ) );
  StatusCode result = StatusCode::FAILURE;

  // Test that the algorithm exists in the member list
  std::vector<Algorithm*>::iterator it;
  std::vector<Algorithm*>::iterator itend = theAlgs->end( );
  for (it = theAlgs->begin(); it != itend; it++) {
    Algorithm* theAlgorithm = (*it);
    if ( theAlgorithm->name( ) == algname ) {

      // Algorithm with specified name exists in the algorithm list - remove it
      // THIS ISN'T IMPLEMENTED YET!!!!
      log << MSG::INFO <<"Sequencer::remove( ) isn't implemented yet!!!!!" << endmsg;
      result = StatusCode::SUCCESS;
      break;
    }
  }
  return result;
}
