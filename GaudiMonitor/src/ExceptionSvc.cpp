/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <GaudiKernel/IExceptionSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>

// STL includes
#include <algorithm>
#include <cassert>
#include <cctype>
#include <exception>
#include <map>
#include <regex>
#include <string>

// ============================================================================
/** @class ExceptionSvc
 *  Simple implementation of IExceptionSvc abstract interface
 *  @author (1) ATLAS collaboration
 *  @author (2) modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-08
 */
// ============================================================================
class ExceptionSvc : public extends<Service, IExceptionSvc> {
public:
  /// Handle caught GaudiExceptions
  StatusCode handle( const INamedInterface& o, const GaudiException& e ) const override; ///< Handle caught exceptions
  /// Handle caught std::exceptions
  StatusCode handle( const INamedInterface& o, const std::exception& e ) const override; ///< Handle caught exceptions
  /// Handle caught (unknown)exceptions
  StatusCode handle( const INamedInterface& o ) const override; ///< Handle caught exceptions
  /// Handle errors
  StatusCode handleErr( const INamedInterface& o, const StatusCode& s ) const override; ///< Handle errors
public:
  /// initialize the service
  StatusCode initialize() override;

public:
  using extends::extends;

private:
  // default constructor is disabled
  ExceptionSvc() = delete; ///< no default constructor
  // copy constructor is disabled
  ExceptionSvc( const ExceptionSvc& ) = delete;
  // assignment operator is disabled
  ExceptionSvc& operator=( const ExceptionSvc& ) = delete;
  // process exceptions
  virtual StatusCode process( const INamedInterface& o ) const;

private:
  enum Policy { ALL, NONE };
  enum ReturnState { SUCCESS, FAILURE, RECOVERABLE, RETHROW, DEFAULT };

  Policy                             m_mode_exc = ALL, m_mode_err = NONE;
  std::map<std::string, ReturnState> m_retCodesExc, m_retCodesErr;

  Gaudi::Property<std::string> m_mode_exc_s{ this, "Catch", "ALL" };
  Gaudi::Property<std::string> m_mode_err_s{ this, "Errors", "NONE" };
};

// ============================================================================
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_COMPONENT( ExceptionSvc )

inline void toupper( std::string& s ) { std::transform( s.begin(), s.end(), s.begin(), (int ( * )( int ))toupper ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::initialize() {
  StatusCode status = Service::initialize();
  if ( status.isFailure() ) { return status; }

  std::string key = m_mode_exc_s.value();

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

  if ( loc == std::string::npos ) {
    key.clear();
  } else {
    key = key.substr( loc + 1 );
  }

  std::string VAL, TAG;

  static const std::regex exp{ "[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*([^[:space:]]+)" };
  static const auto       tok_end = std::sregex_iterator();
  for ( auto tok_iter = std::sregex_iterator( begin( key ), end( key ), exp ); tok_iter != tok_end; ++tok_iter ) {
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

  if ( loc == std::string::npos ) {
    key.clear();
  } else {
    key = key.substr( loc + 1 );
  }

  for ( auto tok_iter = std::sregex_iterator( begin( key ), end( key ), exp ); tok_iter != tok_end; ++tok_iter ) {
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

StatusCode ExceptionSvc::handleErr( const INamedInterface& alg, const StatusCode& st ) const {
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

StatusCode ExceptionSvc::process( const INamedInterface& alg ) const {

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
      throw; // cppcheck-suppress rethrowNoCurrentException
    }
  }

  if ( m_mode_exc == ALL ) { throw; } // cppcheck-suppress rethrowNoCurrentException
  assert( m_mode_exc == NONE );
  return StatusCode::FAILURE;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle( const INamedInterface& alg ) const {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling unknown exception for " << alg.name() << endmsg;
  return process( alg );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle( const INamedInterface& alg, const std::exception& exc ) const {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling std:except: \"" << exc.what() << "\" for " << alg.name() << endmsg;
  return process( alg );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ExceptionSvc::handle( const INamedInterface& alg, const GaudiException& exc ) const {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Handling GaudiException: \"" << exc << "\" for " << alg.name() << endmsg;
  return process( alg );
}

// ============================================================================
// The END
// ============================================================================
