#include "GaudiAlg/Prescaler.h"

#include "GaudiKernel/MsgStream.h"

Prescaler::Prescaler( std::string name, ISvcLocator* pSvcLocator ) : GaudiAlgorithm( std::move( name ), pSvcLocator )
{
  m_percentPass.verifier().setBounds( 0.0, 100.0 );
}

StatusCode Prescaler::initialize()
{
  const StatusCode sc = GaudiAlgorithm::initialize();
  if ( !sc ) return sc;

  info() << name() << ":Prescaler::Initialize - pass: " << m_percentPass << endmsg;

  return sc;
}

StatusCode Prescaler::execute()
{
  ++m_seen;
  float fraction = ( float( 100.0 ) * (float)( m_pass + 1 ) ) / (float)m_seen;
  if ( fraction > m_percentPass ) {
    setFilterPassed( false );
    info() << name() << ":Prescaler::execute - filter failed" << endmsg;
  } else {
    info() << name() << ":Prescaler::execute - filter passed" << endmsg;
    ++m_pass;
  }
  return StatusCode::SUCCESS;
}

StatusCode Prescaler::finalize()
{
  info() << name() << ":Prescaler::finalize - total events: " << m_seen << ", passed events: " << m_pass << endmsg;
  return GaudiAlgorithm::finalize();
}
