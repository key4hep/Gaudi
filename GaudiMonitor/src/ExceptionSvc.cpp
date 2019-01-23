// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <cassert>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ISvcLocator.h"
// ============================================================================
// Local
// ============================================================================
#include "ExceptionSvc.h"
// ============================================================================

#include <boost/regex.hpp>

using namespace std;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_COMPONENT( ExceptionSvc )

inline void toupper( std::string& s ) { std::transform( s.begin(), s.end(), s.begin(), (int ( * )( int ))toupper ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::initialize()
{
  StatusCode status = Service::initialize();
  if ( status.isFailure() ) {
    return status;
  }

  string key = m_mode_exc_s.value();

  auto        loc  = key.find( " " );
  std::string mode = key.substr( 0, loc );

  toupper( mode );

  if ( mode == "NONE" ) {
    m_mode_exc = NONE;
  } else if ( mode == "ALL" ) {
    m_mode_exc = ALL;
  } else {
    error() << "Unknown mode for Exception handling: \"" << mode << "\". Default must be one of \"ALL\" or \"NONE\""
            << endmsg;
    m_state = Gaudi::StateMachine::OFFLINE;
    return StatusCode::FAILURE;
  }

  if ( loc == string::npos ) {
    key.clear();
  } else {
    key = key.substr( loc + 1 );
  }

  std::string VAL, TAG;

  static const boost::regex exp{"[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*([^[:space:]]+)"};
  static const auto         tok_end = boost::sregex_iterator();
  for ( auto tok_iter = boost::sregex_iterator( begin( key ), end( key ), exp ); tok_iter != tok_end; ++tok_iter ) {
    TAG = ( *tok_iter )[1];
    VAL = ( *tok_iter )[2];
    toupper( VAL );

    if ( VAL == "SUCCESS" ) {
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
      error() << "In JobOpts: unknown return code \"" << VAL << "\" for Algorithm " << TAG << std::endl
              << "   Must be one of: DEFAULT, SUCCESS, FAILURE, RECOVERABLE, RETHROW" << endmsg;
      m_state = Gaudi::StateMachine::OFFLINE;
      return StatusCode::FAILURE;
    }

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Will catch exceptions thrown by: " << TAG << " -> action: " << VAL << endmsg;
  }

  // now process errors
  key = m_mode_err_s.value();

  loc  = key.find( " " );
  mode = key.substr( 0, loc );

  toupper( mode );

  if ( mode == "NONE" ) {
    m_mode_err = NONE;
  } else if ( mode == "ALL" ) {
    m_mode_err = ALL;
  } else {
    error() << "Unknown mode for Error handling: \"" << mode << "\". Default must be one of \"ALL\" or \"NONE\""
            << endmsg;
    m_state = Gaudi::StateMachine::OFFLINE;
    return StatusCode::FAILURE;
  }

  if ( loc == string::npos ) {
    key.clear();
  } else {
    key = key.substr( loc + 1 );
  }

  for ( auto tok_iter = boost::sregex_iterator( begin( key ), end( key ), exp ); tok_iter != tok_end; ++tok_iter ) {
    TAG = ( *tok_iter )[1];
    VAL = ( *tok_iter )[2];
    toupper( VAL );

    if ( VAL == "SUCCESS" ) {
      m_retCodesErr[TAG] = SUCCESS;
    } else if ( VAL == "FAILURE" ) {
      m_retCodesErr[TAG] = FAILURE;
    } else if ( VAL == "RECOVERABLE" ) {
      m_retCodesErr[TAG] = RECOVERABLE;
    } else {
      error() << "In JobOpts: unknown return code \"" << VAL << "\" for Algorithm " << TAG << std::endl
              << "   Must be one of: SUCCESS, FAILURE, RECOVERABLE" << endmsg;
      m_state = Gaudi::StateMachine::OFFLINE;
      return StatusCode::FAILURE;
    }

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Will process Errors returned by: " << TAG << " -> action: " << VAL << endmsg;
  }
  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handleErr( const INamedInterface& alg, const StatusCode& st ) const
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling Error from " << alg.name() << endmsg;

  // is this Alg special?
  auto i = m_retCodesErr.find( alg.name() );
  if ( i != m_retCodesErr.end() ) {
    switch ( i->second ) {
    case SUCCESS:
      return StatusCode::SUCCESS;
    case FAILURE:
      return StatusCode::FAILURE;
    case RECOVERABLE:
      return StatusCode::RECOVERABLE;
    // should never get here
    case RETHROW:
      break;
    case DEFAULT:
      break;
    }

  } else {

    if ( m_mode_err == ALL ) return StatusCode::FAILURE; // turn it into FAILURE
    assert( m_mode_err == NONE );
    // don't touch the return code
    return st;
  }
  return StatusCode::FAILURE;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::process( const INamedInterface& alg ) const
{

  // is this Alg special?
  auto i = m_retCodesExc.find( alg.name() );
  if ( i != m_retCodesExc.end() ) {

    switch ( i->second ) {
    case DEFAULT:
      return StatusCode::FAILURE; // there is no default
    case SUCCESS:
      return StatusCode::SUCCESS;
    case FAILURE:
      return StatusCode::FAILURE;
    case RECOVERABLE:
      return StatusCode::RECOVERABLE;
    case RETHROW:
      throw;
    }
  }

  if ( m_mode_exc == ALL ) {
    throw;
  }
  assert( m_mode_exc == NONE );
  return StatusCode::FAILURE;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle( const INamedInterface& alg ) const
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling unknown exception for " << alg.name() << endmsg;
  return process( alg );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle( const INamedInterface& alg, const std::exception& exc ) const
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling std:except: \"" << exc.what() << "\" for " << alg.name() << endmsg;
  return process( alg );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle( const INamedInterface& alg, const GaudiException& exc ) const
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling GaudiException: \"" << exc << "\" for " << alg.name() << endmsg;
  return process( alg );
}

// ============================================================================
// The END
// ============================================================================
