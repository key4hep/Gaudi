#include "PartPropExa.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/AlgFactory.h"

#include "HepPDT/ParticleDataTable.hh"

#include <sstream>

// Static Factory declaration

DECLARE_ALGORITHM_FACTORY(PartPropExa)

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

  m_pps->setUnknownParticleHandler(new HepPDT::TestUnknownID, "My Unknwon PID Test");

  log << MSG::INFO << "this should cause a warning: " << endmsg;
  m_pps->setUnknownParticleHandler(new HepPDT::TestUnknownID, "Second Unknwon PID Test");

  HepPDT::ParticleDataTable *pdt = m_pps->PDT();

  m_pps->setUnknownParticleHandler(new HepPDT::TestUnknownID, "Third Unknwon PID Test");

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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace HepPDT {

CommonParticleData*
TestUnknownID::processUnknownID
              ( ParticleID key, const ParticleDataTable & pdt ) {

  std::cout << "TestUnknownID: " << key.PDTname() << std::endl;

  CommonParticleData * cpd = 0;
  if( key.isNucleus() ) {

    // have to create a TempParticleData with all properties first
    TempParticleData tpd(key);
    // calculate approximate mass
    // WARNING: any calls to particle() from here MUST reference
    //          a ParticleData which is already in the table
    // This convention is enforced.
    const ParticleData * proton = pdt.particle(2212);
    if( proton ) {
      double protonMass = proton->mass();
      tpd.tempMass = Measurement(key.A()*protonMass, 0.);
      // now create CommonParticleData
      cpd = new CommonParticleData(tpd);
    }
  }
  return cpd;
}

}

