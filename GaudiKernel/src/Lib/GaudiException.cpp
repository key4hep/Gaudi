/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/GaudiException.h>

#include <GaudiKernel/System.h>

bool              GaudiException::s_proc( false );
static const bool enableBacktrace = System::isEnvSet( "ENABLE_BACKTRACE" );

namespace {
  inline std::string captureBacktrace( const StatusCode& code ) {
    std::string backtrace;
    if ( enableBacktrace && !code.isSuccess() ) System::backTrace( backtrace, 100, 1 );
    return backtrace;
  }
} // namespace

GaudiException::GaudiException( std::string Message, std::string Tag, StatusCode Code )
    : m_message( std::move( Message ) ), m_tag( std::move( Tag ) ), m_code( std::move( Code ) ) {
  s_proc      = true;
  m_backTrace = captureBacktrace( m_code );
}

GaudiException::GaudiException( std::string Message, std::string Tag, StatusCode Code, const GaudiException& Exception )
    : m_message( std::move( Message ) )
    , m_tag( std::move( Tag ) )
    , m_code( std::move( Code ) )
    , m_previous( Exception.clone() ) {
  // Do not capture backtrace in outer chained exceptions, so only innermost exception is printed
}

GaudiException::GaudiException( std::string Message, std::string Tag, StatusCode Code, const std::exception& Exception )
    : m_message( std::move( Message ) ), m_tag( std::move( Tag ) ), m_code( std::move( Code ) ) {
  s_proc = true;
  m_message += ": " + System::typeinfoName( typeid( Exception ) ) + ", " + Exception.what();
  m_backTrace = captureBacktrace( m_code );
}

GaudiException::GaudiException( const GaudiException& Exception )
    : std::exception( Exception )
    , m_message{ Exception.message() }
    , m_tag{ Exception.tag() }
    , m_code{ Exception.code() }
    , m_backTrace{ Exception.backTrace() }
    , m_previous{ Exception.previous() ? Exception.previous()->clone() : nullptr } {
  s_proc = true;
}

GaudiException::~GaudiException() throw() { s_proc = false; }

GaudiException& GaudiException::operator=( const GaudiException& Exception ) {
  m_message   = Exception.message();
  m_tag       = Exception.tag();
  m_code      = Exception.code();
  m_backTrace = Exception.backTrace();
  m_previous.reset( Exception.previous() ? Exception.previous()->clone() : nullptr );
  return *this;
}

std::ostream& GaudiException::printOut( std::ostream& os ) const {
  os << tag() << " \t " << message() << "\t StatusCode=" << code();
  if ( !backTrace().empty() ) os << std::endl << "Exception stack trace\n" << backTrace();
  return ( 0 != previous() ) ? previous()->printOut( os << std::endl ) : os;
}

MsgStream& GaudiException::printOut( MsgStream& os ) const {
  os << tag() << " \t " << message() << "\t StatusCode=" << code();
  if ( !backTrace().empty() ) os << endmsg << "Exception stack trace\n" << backTrace();
  return ( 0 != previous() ) ? previous()->printOut( os << endmsg ) : os;
}

std::ostream& operator<<( std::ostream& os, const GaudiException& ge ) { return ge.printOut( os ); }

std::ostream& operator<<( std::ostream& os, const GaudiException* pge ) {
  return ( 0 == pge ) ? ( os << " GaudiException* points to NULL!" ) : ( os << *pge );
}

MsgStream& operator<<( MsgStream& os, const GaudiException& ge ) { return ge.printOut( os ); }

MsgStream& operator<<( MsgStream& os, const GaudiException* pge ) {
  return ( 0 == pge ) ? ( os << " GaudiException* points to NULL!" ) : ( os << *pge );
}
