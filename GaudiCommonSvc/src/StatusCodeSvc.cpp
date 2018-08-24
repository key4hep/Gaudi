#include "StatusCodeSvc.h"
#include "GaudiKernel/StatusCode.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

using namespace std;
//
///////////////////////////////////////////////////////////////////////////
//
inline void toupper( std::string& s ) { std::transform( s.begin(), s.end(), s.begin(), (int ( * )( int ))toupper ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode StatusCodeSvc::initialize()
{

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) return sc;

  info() << "initialize" << endmsg;

  for ( const auto& itr : m_pFilter.value() ) {
    // we need to do this if someone has gotten to regFnc before initialize

    string fnc, lib;
    parseFilter( itr, fnc, lib );

    if ( !fnc.empty() ) {
      filterFnc( fnc );
      m_filterfnc.insert( fnc );
    }

    if ( !lib.empty() ) {
      filterLib( lib );
      m_filterlib.insert( lib );
    }
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode StatusCodeSvc::reinitialize()
{

  info() << "reinitialize" << endmsg;

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode StatusCodeSvc::finalize()
{

  if ( !m_dat.empty() ) {

    info() << "listing all unchecked return codes:" << endmsg;

    list();

  } else {

    if ( msgLevel( MSG::DEBUG ) ) debug() << "all StatusCode instances where checked" << endmsg;
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::regFnc( const std::string& fnc, const std::string& lib )
{

  if ( m_state == Gaudi::StateMachine::OFFLINE || m_state == Gaudi::StateMachine::CONFIGURED ) {
    return;
  }

  // A StatusCode instance may be create internally by ROOT dictionaries and,
  // of course, it's not checked, so here we whitelist a few library names
  // that are known to produce spurious reports.
  if ( m_dict ) {
    // ROOT's library names can end with either ".so" or ".so.x.y" with x.y the ROOT version in use
    if ( lib.compare( lib.length() - 3, 3, ".so" ) == 0 ) {
      if ( lib.compare( lib.length() - 7, 4, "Dict" ) == 0 || lib.compare( lib.length() - 8, 5, "Cling" ) == 0 ||
           lib.compare( lib.length() - 7, 4, "Core" ) == 0 ) {
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

  {
    const string rlib = lib.substr( lib.rfind( "/" ) + 1 );

    if ( m_filterfnc.find( fnc ) != m_filterfnc.end() || m_filterlib.find( rlib ) != m_filterlib.end() ) {
      return;
    }
  }

  if ( m_abort ) {
    fatal() << "Unchecked StatusCode in " << fnc << " from lib " << lib << endmsg;
    abort();
  }

  string key = fnc + lib;

  auto itr = m_dat.find( key );

  if ( itr != m_dat.end() ) {
    itr->second.count += 1;
  } else {

    const string rlib = lib.substr( lib.rfind( "/" ) + 1 );

    StatCodeDat dat;
    dat.fnc   = fnc;
    dat.lib   = rlib;
    dat.count = 1;

    m_dat[key] = dat;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::list() const
{

  std::ostringstream os;
  os << "Num | Function                       | Source Library" << endl;
  os << "----+--------------------------------+-------------------"
     << "-----------------------" << endl;

  for ( const auto& itr : m_dat ) {
    const auto& dat = itr.second;

    os.width( 3 );
    os.setf( ios_base::right, ios_base::adjustfield );
    os << dat.count;

    os << " | ";
    os.width( 30 );
    os.setf( ios_base::left, ios_base::adjustfield );
    os << dat.fnc;

    os << " | ";
    os.setf( ios_base::left, ios_base::adjustfield );
    os << dat.lib;

    os << endl;
  }

  info() << endl << os.str() << endmsg;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void StatusCodeSvc::filterFnc( const std::string& str )
{

  auto itr = std::find_if( m_dat.begin(), m_dat.end(),
                           [&]( const std::pair<std::string, StatCodeDat>& d ) { return d.second.fnc == str; } );
  if ( itr != std::end( m_dat ) ) m_dat.erase( itr );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::filterLib( const std::string& str )
{

  auto itr = std::find_if( m_dat.begin(), m_dat.end(),
                           [&]( const std::pair<std::string, StatCodeDat>& d ) { return d.second.lib == str; } );
  if ( itr != std::end( m_dat ) ) m_dat.erase( itr );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void StatusCodeSvc::parseFilter( const string& str, string& fnc, string& lib )
{

  auto loc = str.find( "=" );
  if ( loc == std::string::npos ) {
    fnc = str;
    lib = "";
  } else {
    string key = str.substr( 0, loc );
    string val = str.substr( loc + 1 );

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
