// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <cassert>
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Tokenizer.h"
// ============================================================================
//Local
// ============================================================================
#include "ExceptionSvc.h"
// ============================================================================

using namespace std;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_COMPONENT(ExceptionSvc)

inline void toupper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 (int(*)(int)) toupper);
}

//
///////////////////////////////////////////////////////////////////////////
//

ExceptionSvc::ExceptionSvc( const std::string& name, ISvcLocator* svc )
  : base_class( name, svc )
  , m_mode_exc ( ALL ), m_mode_err( NONE )
  , m_log(msgSvc(), name )
{
  // for exceptions
  declareProperty( "Catch"      , m_mode_exc_s="ALL" ) ;

  // for return codes
  declareProperty( "Errors"     , m_mode_err_s="NONE" ) ;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ExceptionSvc::~ExceptionSvc() {

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
ExceptionSvc::initialize() {
  StatusCode status = Service::initialize();
  m_log.setLevel( m_outputLevel.value() );

  if ( status.isFailure() )  { return status ; }                  // RETURN

  string key = m_mode_exc_s.value();

  string::size_type loc = key.find(" ");
  std::string mode;
  if (loc == std::string::npos) {
    mode = key;
  } else {
    mode = key.substr(0,loc);
  }

  toupper(mode);

  if (mode == "NONE") {
    m_mode_exc = NONE;
  } else if (mode == "ALL") {
    m_mode_exc = ALL;
  } else {
    m_log << MSG::ERROR << "Unknown mode for Exception handling: \"" << mode
	<< "\". Default must be one of \"ALL\" or \"NONE\"" << endmsg;
    m_state = Gaudi::StateMachine::OFFLINE;
    return StatusCode::FAILURE;
  }

  if (loc == string::npos) {
    key = "";
  } else {
    key = key.substr(loc+1,key.length());
  }

  Tokenizer tok(true);
  std::string val,VAL,TAG;

  tok.analyse( key, " ", "", "", "=", "", "");

  for ( Tokenizer::Items::iterator i = tok.items().begin();
	i != tok.items().end(); i++)    {
    const std::string& tag = (*i).tag();
    TAG = tag;

    val = (*i).value();
    VAL = val;
    toupper(VAL);

    if (VAL == "SUCCESS") {
      m_retCodesExc[TAG] = SUCCESS;
    } else if ( VAL == "FAILURE" ) {
      m_retCodesExc[TAG] = FAILURE;
    } else if ( VAL == "REVOVERABLE" ) {
      m_retCodesExc[TAG] = RECOVERABLE;
    } else if ( VAL == "RETHROW" ) {
      m_retCodesExc[TAG] = RETHROW;
    } else if ( VAL == "DEFAULT" ) {
      m_retCodesExc[TAG] = DEFAULT;
    } else {
      m_log << MSG::ERROR << "In JobOpts: unknown return code \"" << VAL
            << "\" for Algorithm " << TAG << std::endl
            << "   Must be one of: DEFAULT, SUCCESS, FAILURE, RECOVERABLE, RETHROW"
            << endmsg;
      m_state = Gaudi::StateMachine::OFFLINE;
      return StatusCode::FAILURE;
    }

    m_log << MSG::DEBUG << "Will catch exceptions thrown by: " << TAG
	<< " -> action: " << VAL << endmsg;

  }

  // now process errors

  key = m_mode_err_s.value();

  loc = key.find(" ");
  if (loc == std::string::npos) {
    mode = key;
  } else {
    mode = key.substr(0,loc);
  }

  toupper(mode);

  if (mode == "NONE") {
    m_mode_err = NONE;
  } else if (mode == "ALL") {
    m_mode_err = ALL;
  } else {
    m_log << MSG::ERROR << "Unknown mode for Error handling: \"" << mode
          << "\". Default must be one of \"ALL\" or \"NONE\"" << endmsg;
    m_state = Gaudi::StateMachine::OFFLINE;
    return StatusCode::FAILURE;
  }

  if (loc == string::npos) {
    key = "";
  } else {
    key = key.substr(loc+1,key.length());
  }

  Tokenizer tok2(true);
  tok2.analyse( key, " ", "", "", "=", "", "");

  for ( Tokenizer::Items::iterator i = tok2.items().begin();
        i != tok2.items().end(); i++)    {
    const std::string& tag = (*i).tag();
    TAG = tag;

    val = (*i).value();
    VAL = val;
    toupper(VAL);

    if (VAL == "SUCCESS") {
      m_retCodesErr[TAG] = SUCCESS;
    } else if ( VAL == "FAILURE" ) {
      m_retCodesErr[TAG] = FAILURE;
    } else if ( VAL == "RECOVERABLE" ) {
      m_retCodesErr[TAG] = RECOVERABLE;
    } else {
      m_log << MSG::ERROR << "In JobOpts: unknown return code \"" << VAL
            << "\" for Algorithm " << TAG << std::endl
            << "   Must be one of: SUCCESS, FAILURE, RECOVERABLE"
            << endmsg;
      m_state = Gaudi::StateMachine::OFFLINE;
      return StatusCode::FAILURE;
    }

    m_log << MSG::DEBUG << "Will process Errors returned by: " << TAG
	<< " -> action: " << VAL << endmsg;

  }

  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
ExceptionSvc::finalize() {
  StatusCode status = Service::finalize();

  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handleErr
( const INamedInterface& alg ,
  const StatusCode&      st  ) const
{
  m_log << MSG::DEBUG << "Handling Error from " << alg.name() << endmsg;

  // is this Alg special?
  if (m_retCodesErr.find(alg.name()) != m_retCodesErr.end()) {
    ReturnState iret = m_retCodesErr.find(alg.name())->second;

    switch ( iret ) {
    case SUCCESS:
      return StatusCode::SUCCESS;
    case FAILURE:
      return StatusCode::FAILURE;
    case RECOVERABLE:
      return StatusCode::RECOVERABLE;
    case RETHROW:
      // should never get here
      break;
    case DEFAULT:
      // should never get here
      break;
    }

  } else {

    if (m_mode_err == ALL) {
      // turn it into a FAILURE
      return StatusCode::FAILURE;

    } else {
      assert (m_mode_err == NONE );
      // don't touch the return code
      return st;
    }
  }

  return StatusCode::FAILURE;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::process
( const INamedInterface& alg ) const
{

  // is this Alg special?
  if (m_retCodesExc.find(alg.name()) != m_retCodesExc.end()) {
    ReturnState iret = m_retCodesExc.find(alg.name())->second;

    switch ( iret ) {
    case DEFAULT:
      // there is no default
      return StatusCode::FAILURE;
    case SUCCESS:
      return StatusCode::SUCCESS;
    case FAILURE:
      return StatusCode::FAILURE;
    case RECOVERABLE:
      return StatusCode::RECOVERABLE;
    case RETHROW:
      throw;
    }

  } else {

    if (m_mode_exc == ALL) {
      throw;
    } else {
      assert (m_mode_exc == NONE);
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::FAILURE;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle
( const INamedInterface& alg ) const
{
  m_log << MSG::DEBUG << "Handling unknown exception for " << alg.name()
        << endmsg;

  return process(alg);
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle
( const INamedInterface& alg ,
  const std::exception & exc ) const
{
  m_log << MSG::DEBUG << "Handling std:except: \"" << exc.what() << "\" for "
        << alg.name() << endmsg;

  return process(alg) ;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle
( const INamedInterface& alg ,
  const GaudiException & exc ) const
{
  m_log << MSG::DEBUG << "Handling GaudiException: \"" << exc << "\" for "
        << alg.name() << endmsg;

  return process(alg);

}

// ============================================================================
// The END
// ============================================================================
