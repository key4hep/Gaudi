// $Id: Guards.cpp,v 1.4 2008/11/11 14:43:37 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <exception>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"
// ============================================================================
/** @file
 *  Implementation file for class Gaudi::Guards::ExceptionGuard
 *  and class Gaudi::Guards::AuditorGuard
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-07
 */
// ============================================================================
// Local handle (print) of GaudiException
// ============================================================================
void Gaudi::Guards::ExceptionGuard::handle
( const GaudiException& exc , MsgStream& log )
{
  // the general printout
  log << MSG::FATAL
      << System::typeinfoName( typeid ( exc ) )
      << "('" << exc.tag() << "') is caught!" << endmsg ;
  // print the detailes about the exception:
  log << MSG::ERROR << exc << endmsg ;
  // get the status code form the exception:
  m_sc = exc.code() ;
}
// ============================================================================
// Local handle (print) of std::exception
// ============================================================================
void Gaudi::Guards::ExceptionGuard::handle
( const std::exception& exc , MsgStream& log )
{
  // the general printout
  log << MSG::FATAL
      << System::typeinfoName( typeid ( exc ) ) << " is caught!" << endmsg ;
  // print the detailes abotu the exception:
  log << MSG::ERROR << exc.what() << endmsg ;
}
// ============================================================================
// Local handle (print) of unknown exception
// ============================================================================
void Gaudi::Guards::ExceptionGuard::handle
( MsgStream& log )
{
  // the general printout
  log << MSG::FATAL << "UNKNOWN exception is caught!" << endmsg ;
}
// ============================================================================
// dectructor
// ============================================================================
Gaudi::Guards::ExceptionGuard::~ExceptionGuard() { m_sc.ignore() ; }
// ============================================================================
// constructor with standard post-action
// ============================================================================
Gaudi::Guards::AuditorGuard::AuditorGuard ( INamedInterface*             obj      ,
                              IAuditor*                    svc      ,
                              IAuditor::StandardEventType  evt      ):
                 m_obj(obj),
                 m_objName(),
                 m_svc(svc),
                 m_evt(evt),
                 m_cevt(),
                 m_sc(0),
                 m_customEvtType(false)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( INamedInterface*          obj      ,
                              IAuditor*                       svc      ,
                              IAuditor::CustomEventTypeRef    evt      ):
                 m_obj(obj),
                 m_objName(),
                 m_svc(svc),
                 m_evt(IAuditor::Initialize), // Windows needs an explicit value
                 m_cevt(evt),
                 m_sc(0),
                 m_customEvtType(true)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( INamedInterface*             obj      ,
    IAuditor*                    svc      ,
    IAuditor::StandardEventType  evt      ,
    const StatusCode             &sc       ):
      m_obj(obj),
      m_objName(),
      m_svc(svc),
      m_evt(evt),
      m_cevt(),
      m_sc(&sc),
      m_customEvtType(false)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( INamedInterface*                   obj      ,
    IAuditor*                          svc      ,
    IAuditor::CustomEventTypeRef       evt      ,
    const StatusCode                  &sc       ):
      m_obj(obj),
      m_objName(),
      m_svc(svc),
      m_evt(IAuditor::Initialize), // Windows needs an explicit value
      m_cevt(evt),
      m_sc(&sc),
      m_customEvtType(true)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( const std::string           &name      ,
    IAuditor*                    svc      ,
    IAuditor::StandardEventType  evt      ):
      m_obj(0),
      m_objName(name),
      m_svc(svc),
      m_evt(evt),
      m_cevt(),
      m_sc(0),
      m_customEvtType(false)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( const std::string           &name      ,
    IAuditor*                          svc      ,
    IAuditor::CustomEventTypeRef       evt      ):
      m_obj(0),
      m_objName(name),
      m_svc(svc),
      m_evt(IAuditor::Initialize), // Windows needs an explicit value
      m_cevt(evt),
      m_sc(0),
      m_customEvtType(true)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( const std::string           &name      ,
    IAuditor*                    svc      ,
    IAuditor::StandardEventType  evt      ,
    const StatusCode            &sc       ):
      m_obj(0),
      m_objName(name),
      m_svc(svc),
      m_evt(evt),
      m_cevt(),
      m_sc(&sc),
      m_customEvtType(false)
{
  i_before();
}
Gaudi::Guards::AuditorGuard::AuditorGuard ( const std::string           &name      ,
    IAuditor*                          svc      ,
    IAuditor::CustomEventTypeRef       evt      ,
    const StatusCode                  &sc       ):
      m_obj(0),
      m_objName(name),
      m_svc(svc),
      m_evt(IAuditor::Initialize), // Windows needs an explicit value
      m_cevt(evt),
      m_sc(&sc),
      m_customEvtType(true)
{
  i_before();
}
// ============================================================================
// dectructor
// ============================================================================
Gaudi::Guards::AuditorGuard::~AuditorGuard()
{
  i_after();
}
// ============================================================================
/// The END
// ============================================================================
