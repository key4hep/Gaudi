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
#include "StatusCodeSvc.h"
#include "GaudiKernel/StatusCode.h"

//
///////////////////////////////////////////////////////////////////////////
//
inline void toupper( std::string& s ) { std::transform( s.begin(), s.end(), s.begin(), (int ( * )( int ))toupper ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode StatusCodeSvc::initialize() {

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) return sc;

  info() << "initialize" << endmsg;

  for ( const auto& itr : m_pFilter.value() ) {
    std::string fnc, lib;
    parseFilter( itr, fnc, lib );
    if ( !fnc.empty() ) m_filterfnc.insert( fnc );
    if ( !lib.empty() ) m_filterlib.insert( lib );
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode StatusCodeSvc::reinitialize() {

  info() << "reinitialize" << endmsg;

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode StatusCodeSvc::finalize() {

  list();
  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::regFnc( const std::string& fnc, const std::string& lib ) {

  if ( m_state == Gaudi::StateMachine::OFFLINE || m_state == Gaudi::StateMachine::CONFIGURED ) { return; }

  // A StatusCode instance may be create internally by ROOT dictionaries and,
  // of course, it's not checked, so here we whitelist a few library names
  // that are known to produce spurious reports.
  if ( m_dict ) {
    // ROOT's library names can end with either ".so" or ".so.x.y" with x.y the ROOT version in use
    const auto len = lib.length();
    if ( len >= 3 && lib.compare( len - 3, 3, ".so" ) == 0 ) {
      if ( ( len >= 7 && ( lib.compare( len - 7, 4, "Dict" ) == 0 || lib.compare( len - 7, 4, "Core" ) == 0 ) ) ||
           ( len >= 8 && lib.compare( len - 8, 5, "Cling" ) == 0 ) ) {
        return;
      }
    } else if ( lib.rfind( "Dict.so" ) != std::string::npos || lib.rfind( "Cling.so" ) != std::string::npos ||
                lib.rfind( "Core.so" ) != std::string::npos ) {
      return;
    }
  }
  // this appears only with gcc 4.9...
  if ( fnc == "_PyObject_GC_Malloc" ) return;
  // GAUDI-1036
  if ( fnc == "PyThread_get_thread_ident" ) return;
  if ( fnc == "local" ) return;

  const std::string rlib = lib.substr( lib.rfind( "/" ) + 1 );
  if ( failsFilter( fnc, rlib ) ) return;

  if ( m_abort ) {
    fatal() << "Unchecked StatusCode in " << fnc << " from lib " << lib << endmsg;
    msgStream().flush();
    abort();
  }

  const std::string key = fnc + lib;

  decltype( m_dat )::const_iterator itr = m_dat.find( key );
  if ( itr != m_dat.end() ) {
    ++( itr->second.count );
  } else {
    m_dat.emplace( key, StatCodeDat{ fnc, rlib } );
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool StatusCodeSvc::failsFilter( const std::string& fnc, const std::string& lib ) const {
  return m_filterfnc.find( fnc ) != m_filterfnc.end() || m_filterlib.find( lib ) != m_filterlib.end();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::list() const {

  std::ostringstream os;
  unsigned int       N = 0;

  // Created a sorted map of the results
  std::map<std::string, StatCodeDat> dat_sorted( m_dat.begin(), m_dat.end() );
  for ( const auto& itr : dat_sorted ) {
    const auto& dat = itr.second;

    // we need to do this in case someone has gotten to regFnc before filters were setup in initialize
    if ( failsFilter( dat.fnc, dat.lib ) ) continue;

    N++;
    os.width( 3 );
    os.setf( std::ios_base::right, std::ios_base::adjustfield );
    os << dat.count;

    os << " | ";
    os.width( 30 );
    os.setf( std::ios_base::left, std::ios_base::adjustfield );
    os << dat.fnc;

    os << " | ";
    os.setf( std::ios_base::left, std::ios_base::adjustfield );
    os << dat.lib;

    os << std::endl;
  }
  if ( N > 0 ) {
    info() << "listing all unchecked return codes:" << endmsg;
    info() << std::endl
           << "Num | Function                       | Source Library" << std::endl
           << "----+--------------------------------+------------------------------------------" << std::endl
           << os.str() << endmsg;
  } else {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "all StatusCode instances where checked" << endmsg;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::parseFilter( const std::string& str, std::string& fnc, std::string& lib ) const {

  auto loc = str.find( "=" );
  if ( loc == std::string::npos ) {
    fnc = str;
    lib = "";
  } else {
    std::string key = str.substr( 0, loc );
    std::string val = str.substr( loc + 1 );

    toupper( key );

    if ( key == "FCN" || key == "FNC" ) {
      fnc = val;
      lib.clear();
    } else if ( key == "LIB" ) {
      fnc.clear();
      lib = val;
    } else {
      fnc.clear();
      lib.clear();

      warning() << "ignoring unknown token in Filter: " << str << endmsg;
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_COMPONENT( StatusCodeSvc )
