#include "PartPropExa.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/AlgFactory.h"

#include "HepPDT/ParticleDataTable.hh"

#include <sstream>

// Static Factory declaration

DECLARE_ALGORITHM_FACTORY(PartPropExa);

PartPropExa::PartPropExa( const std::string& name, ISvcLocator* pSvcLocator ) :
  Algorithm(name, pSvcLocator)
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode PartPropExa::initialize() {

  MsgStream log(msgSvc(), name());

  if (service("PartPropSvc",m_pps).isFailure()) {
    log << MSG::ERROR << "Could not get PartPropSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  HepPDT::ParticleDataTable *pdt = m_pps->PDT();

  std::ostringstream ost;
  pdt->writeParticleData( ost );

  log << MSG::INFO << ost.str() << endmsg;

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode PartPropExa::execute() {


  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode PartPropExa::finalize() {

  MsgStream log(msgSvc(), name());

  return StatusCode::SUCCESS;

}
