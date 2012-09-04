#include "GaudiHive/HiveTestAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

DECLARE_ALGORITHM_FACTORY(HiveTestAlgorithm)

/**
 ** Constructor(s)
 **/
HiveTestAlgorithm::HiveTestAlgorithm(const std::string& name, ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator),
  m_total( 0 ),
  m_inputs( 0 ),
  m_outputs( 0 )
{
  declareProperty("Inputs", m_inputs, "List of required inputs");
  declareProperty("Outputs", m_inputs, "List of provided outputs");
}

/**
 ** Destructor
 **/
HiveTestAlgorithm::~HiveTestAlgorithm( )
{
}

StatusCode
HiveTestAlgorithm::initialize()
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << name( ) << ":HiveTestAlgorithm::initialize " << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode
HiveTestAlgorithm::execute()
{
  MsgStream log(msgSvc(), name());
  ++m_total;
  log << MSG::INFO << name( ) << ":HiveTestAlgorithm::execute " << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode
HiveTestAlgorithm::finalize()
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << name( ) << ":HiveTestAlgorithm::finalize - total events: " << m_total << endmsg;
  return StatusCode::SUCCESS;
}

const std::vector<std::string>
HiveTestAlgorithm::get_inputs()
{
  return m_inputs;
}

const std::vector<std::string>
HiveTestAlgorithm::get_outputs()
{
  return m_outputs;
}


