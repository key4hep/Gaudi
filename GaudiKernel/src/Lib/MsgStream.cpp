//====================================================================
//	MsgStream.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: Class to ease error logging to the message service
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define KERNEL_MSGSTREAM_CPP

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/Message.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>

bool MsgStream::m_countInactive = false;

bool MsgStream::enableCountInactive( bool value ) {
  bool old        = m_countInactive;
  m_countInactive = value;
  return old;
}

bool MsgStream::countInactive() { return m_countInactive; }

MsgStream::MsgStream( IMessageSvc* svc, int ) : m_service( svc ) {
  setLevel( svc ? svc->outputLevel() : MSG::INFO );
  m_currLevel = m_level;
  m_useColors = ( svc ? svc->useColor() : false );
#ifndef NDEBUG
  m_inactCounter = svc ? Gaudi::Cast<IInactiveMessageCounter>( svc ) : 0;
#endif
}

MsgStream::MsgStream( IMessageSvc* svc, std::string source, int ) : m_service( svc ), m_source( std::move( source ) ) {
  setLevel( svc ? svc->outputLevel( m_source ) : MSG::INFO );
  m_currLevel = m_level;
  m_useColors = ( svc && svc->useColor() );
#ifndef NDEBUG
  m_inactCounter = svc ? Gaudi::Cast<IInactiveMessageCounter>( svc ) : 0;
#endif
}

MsgStream& MsgStream::doOutput() {
  try {
    // This piece of code may throw and we cannot afford it when we print a message
    // in the middle of a catch block.
    if ( isActive() ) {
      const Message msg( m_source, m_currLevel, m_stream.str() );
      if ( m_service ) {
        m_service->reportMessage( msg, m_currLevel );
      } else {
        std::cout << msg << std::endl;
      }
    }
    m_stream.str( "" );
  } catch ( ... ) {}
  return *this;
}

void MsgStream::setColor( MSG::Color col ) {
#ifndef _WIN32
  if ( m_useColors ) {
    int fc = 90 + col;
    try { // this may throw and we must not do it
      m_stream << "\x1b[" << fc << ";1m";
    } catch ( ... ) {}
  }
#endif
}

void MsgStream::setColor( MSG::Color fg, MSG::Color bg ) {
#ifndef _WIN32
  if ( m_useColors ) {
    try { // this may throw and we must not do it
      int fc = 90 + fg;
      m_stream << "\x1b[" << fc;
      int bc = 100 + bg;
      m_stream << ";" << bc;
      m_stream << ";1m";
    } catch ( ... ) {}
  }
#endif
}

void MsgStream::resetColor() {
#ifndef _WIN32
  if ( m_useColors ) {
    try { // this may throw and we must not do it
      m_stream << "\x1b[m" << m_service->getLogColor( m_currLevel );
    } catch ( ... ) {}
  }
#endif
}

std::string format( const char* fmt, ... ) {
  const int   buffsize = 2048;
  static char buffer[buffsize];
  va_list     arguments;
  va_start( arguments, fmt );
  if ( vsnprintf( buffer, buffsize, fmt, arguments ) >= buffsize )
    throw GaudiException( "Insufficient buffer size (" + std::to_string( buffsize ) + ") when formatting message",
                          "MsgStream", StatusCode::FAILURE );
  va_end( arguments );
  return std::string( buffer );
}
