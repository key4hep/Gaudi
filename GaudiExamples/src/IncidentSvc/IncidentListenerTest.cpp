#include "IncidentListenerTest.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/MsgStream.h"

#include "GaudiKernel/GaudiException.h"

//=============================================================================
IncidentListenerTest::IncidentListenerTest( const std::string& name, ISvcLocator* svcloc, long shots )
    : m_name( name ), m_shots( shots ) {
  m_msgSvc = svcloc; // default message service
  if ( !m_msgSvc ) throw GaudiException( "Cannot find MessageSvc", m_name, StatusCode::FAILURE );
  m_incSvc = svcloc->service( "IncidentSvc" );
  if ( !m_incSvc ) throw GaudiException( "Cannot find IncidentSvc", m_name, StatusCode::FAILURE );
}

IncidentListenerTest::~IncidentListenerTest() {}

//=============================================================================
void IncidentListenerTest::handle( const Incident& incident ) {
  MsgStream log( m_msgSvc, m_name );
  log << MSG::INFO << "Handling incident '" << incident.type() << "'" << endmsg;
  if ( ( m_shots > 0 ) && ( --m_shots == 0 ) ) {
    log << MSG::INFO << "deregistering" << endmsg;
    m_incSvc->removeListener( this, incident.type() );
  }
}
