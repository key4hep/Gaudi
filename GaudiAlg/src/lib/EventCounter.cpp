#include "GaudiAlg/EventCounter.h"
#include "GaudiKernel/MsgStream.h"

/**
 ** Constructor(s)
 **/
EventCounter::EventCounter( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {
  m_frequency.verifier().setBounds( 0, 1000 );
}

StatusCode EventCounter::initialize() {
  info() << name() << ":EventCounter::initialize - Frequency: " << m_frequency << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode EventCounter::execute() {
  m_total++;
  int freq = m_frequency;
  if ( freq > 0 ) {
    ++m_skip;
    if ( m_skip >= freq ) {
      info() << name() << ":EventCounter::execute - seen events: " << m_total << endmsg;
      m_skip = 0;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode EventCounter::finalize() {
  info() << name() << ":EventCounter::finalize - total events: " << m_total << endmsg;
  return StatusCode::SUCCESS;
}
