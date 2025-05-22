/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/Guards.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/System.h>
#include <exception>

/** @file
 *  Implementation file for class Gaudi::Guards::ExceptionGuard
 *  and class Gaudi::Guards::AuditorGuard
 *  Local handle (print) of GaudiException
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-07
 */

void Gaudi::Guards::ExceptionGuard::handle( const GaudiException& exc, MsgStream& log ) {
  log << MSG::FATAL << System::typeinfoName( typeid( exc ) ) << "('" << exc.tag() << "') is caught!" << endmsg;
  log << MSG::ERROR << exc << endmsg;
  m_sc = exc.code();
}

void Gaudi::Guards::ExceptionGuard::handle( const std::exception& exc, MsgStream& log ) {
  log << MSG::FATAL << System::typeinfoName( typeid( exc ) ) << " is caught!" << endmsg;
  log << MSG::ERROR << exc.what() << endmsg;
}

void Gaudi::Guards::ExceptionGuard::handle( MsgStream& log ) {
  log << MSG::FATAL << "UNKNOWN exception is caught!" << endmsg;
}

Gaudi::Guards::ExceptionGuard::~ExceptionGuard() { m_sc.ignore(); }

Gaudi::Guards::AuditorGuard::AuditorGuard( std::string name, IAuditor* svc, std::string const& evt,
                                           EventContext const& context )
    : m_objName( name ), m_svc( svc ), m_evt( evt ), m_context( context ) {
  i_before();
}

Gaudi::Guards::AuditorGuard::AuditorGuard( std::string name, IAuditor* svc, std::string const& evt,
                                           StatusCode const& sc, EventContext const& context )
    : m_objName( name ), m_svc( svc ), m_evt( evt ), m_sc( &sc ), m_context( context ) {
  i_before();
}
