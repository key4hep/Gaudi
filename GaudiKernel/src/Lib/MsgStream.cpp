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
#include "GaudiKernel/Message.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IMessageSvc.h"

#include <iostream>
#include <stdarg.h>
#include <stdio.h>

bool MsgStream::m_countInactive = false;

bool MsgStream::enableCountInactive(bool value) {
  bool old = m_countInactive;
  m_countInactive = value;
  return old;
}

bool MsgStream::countInactive() {
  return m_countInactive;
}


MsgStream::MsgStream(IMessageSvc* svc, int)
: m_service(svc),
  m_source(""),
  m_active(false),
  m_inactCounter(0)
{
  setLevel((0==svc) ? MSG::INFO : svc->outputLevel());
  m_useColors = (0==svc) ? false : svc->useColor();
#ifndef NDEBUG
  m_inactCounter = svc ? Gaudi::Cast<IInactiveMessageCounter>(svc) : 0;
#endif
}

MsgStream::MsgStream(IMessageSvc* svc, const std::string& source, int)
: m_service(svc),
  m_source(source),
  m_active(false),
  m_inactCounter(0)
{
  setLevel((0==svc) ? MSG::INFO : svc->outputLevel(source));
  m_useColors = (0==svc) ? false : svc->useColor();
#ifndef NDEBUG
  m_inactCounter = svc ? Gaudi::Cast<IInactiveMessageCounter>(svc) : 0;
#endif
}

MsgStream::~MsgStream()    {
}

MsgStream& MsgStream::doOutput()       {
  try {
    // This piece of code may throw and we cannot afford it when we print a message
    // in the middle of a catch block.
    if ( isActive() )   {
      Message msg(m_source,m_currLevel,m_stream.str());
      if ( m_service != 0 )   {
        m_service->reportMessage (msg, m_currLevel);
      }
      else     {
        std::cout << msg << std::endl;
      }
    }
    m_stream.str("");
  } catch(...) {}
  return *this;
}

void MsgStream::setColor(MSG::Color col) {
#ifndef _WIN32
  if ( m_useColors)   {
    int fc = 90 + col;
    try { // this may throw and we must not do it
      m_stream << "[" << fc << ";1m";
    }
    catch(...) {}
  }
#endif
}

void MsgStream::setColor(MSG::Color fg, MSG::Color bg) {
#ifndef _WIN32
  if ( m_useColors )   {
    try { // this may throw and we must not do it
      int fc = 90 + fg;
      m_stream << "[" << fc;
      int bc = 100 + bg;
      m_stream << ";" << bc;
      m_stream << ";1m";
    }
    catch(...) {}
  }
#endif
}

void MsgStream::resetColor() {
#ifndef _WIN32
  if ( m_useColors )   {
    try { // this may throw and we must not do it
      m_stream << "[m" << m_service->getLogColor(m_currLevel);
    }
    catch(...) {}
  }
#endif
}

#ifdef WIN32
// Disable warning
//   C4996: 'vsprintf': This function or variable may be unsafe.
#pragma warning(disable:4996)
#endif
std::string format( const char* fmt, ... )
{
  const int buffsize = 2048;
  static char buffer[buffsize];
  va_list arguments;
  va_start( arguments, fmt );
  if( vsprintf(buffer, fmt, arguments) >= buffsize )
    throw GaudiException("Insufficient buffer size (2048) when formatting message",
                         "MsgStream", 0);
  return std::string(buffer);
}
