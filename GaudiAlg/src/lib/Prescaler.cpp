#include "GaudiAlg/Prescaler.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

Prescaler::Prescaler(const std::string& name, ISvcLocator* pSvcLocator) :
  GaudiAlgorithm(name, pSvcLocator) ,
  m_pass( 0 ),
  m_seen( 0 )
{
  declareProperty( "PercentPass", m_percentPass=100.0 );
  m_percentPass.verifier().setBounds( 0.0, 100.0 );
}

Prescaler::~Prescaler( )
{
}

StatusCode
Prescaler::initialize()
{
  const StatusCode sc = GaudiAlgorithm::initialize();
  if ( !sc) return sc;

  info() << name( ) << ":Prescaler::Initialize - pass: " << m_percentPass << endmsg;
 
 return sc;
}

StatusCode
Prescaler::execute()
{
  ++m_seen;
  float fraction = (float(100.0) * (float)(m_pass+1)) / (float)m_seen;
  if ( fraction > m_percentPass ) {
    setFilterPassed( false );
    info() << name() << ":Prescaler::execute - filter failed" << endmsg;
  } else {
    info() << name() << ":Prescaler::execute - filter passed" << endmsg;
    ++m_pass;
  }
  return StatusCode::SUCCESS;
}

StatusCode
Prescaler::finalize()
{
  info() << name( ) << ":Prescaler::finalize - total events: "
         << m_seen << ", passed events: " << m_pass << endmsg;
  return GaudiAlgorithm::finalize();
}
