//$Id: EventCounter.cpp,v 1.1 2004/07/12 13:39:20 mato Exp $
#include "GaudiAlg/EventCounter.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

/**
 ** Constructor(s)
 **/
EventCounter::EventCounter(const std::string& name, ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator),
  m_skip ( 0 ),
  m_total( 0 )
{
    declareProperty( "Frequency", m_frequency=1 );
    m_frequency.verifier().setBounds( 0, 1000 );
}

/**
 ** Destructor
 **/
EventCounter::~EventCounter( )
{
}

StatusCode
EventCounter::initialize()
{
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << name( ) << ":EventCounter::initialize - Frequency: " << m_frequency << endmsg;
    return StatusCode::SUCCESS;
}

StatusCode
EventCounter::execute()
{
     MsgStream log(msgSvc(), name());
     m_total++;
     int freq = m_frequency;
     if ( freq > 0 ) {
         m_skip++;
         if ( m_skip >= freq ) {
             log << MSG::INFO << name( ) << ":EventCounter::execute - seen events: " << m_total << endmsg;
             m_skip = 0;
         }
     }
    return StatusCode::SUCCESS;
}

StatusCode
EventCounter::finalize()
{
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << name( ) << ":EventCounter::finalize - total events: " << m_total << endmsg;
    return StatusCode::SUCCESS;
}
