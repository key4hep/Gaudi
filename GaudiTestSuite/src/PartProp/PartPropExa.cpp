/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "PartPropExa.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

#include "HepPDT/ParticleDataTable.hh"

#include <sstream>

// Static Factory declaration

DECLARE_COMPONENT( PartPropExa )

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode PartPropExa::initialize() {

  if ( service( "PartPropSvc", m_pps ).isFailure() ) {
    error() << "Could not get PartPropSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_pps->setUnknownParticleHandler( new HepPDT::TestUnknownID, "My Unknwon PID Test" );

  info() << "this should cause a warning: " << endmsg;
  m_pps->setUnknownParticleHandler( new HepPDT::TestUnknownID, "Second Unknwon PID Test" );

  HepPDT::ParticleDataTable* pdt = m_pps->PDT();

  m_pps->setUnknownParticleHandler( new HepPDT::TestUnknownID, "Third Unknwon PID Test" );

  std::ostringstream ost;
  pdt->writeParticleData( ost );

  info() << ost.str() << endmsg;

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode PartPropExa::execute() { return StatusCode::SUCCESS; }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace HepPDT {

  CommonParticleData* TestUnknownID::processUnknownID( ParticleID key, const ParticleDataTable& pdt ) {

    std::cout << "TestUnknownID: " << key.PDTname() << std::endl;

    CommonParticleData* cpd = nullptr;
    if ( key.isNucleus() ) {

      // have to create a TempParticleData with all properties first
      TempParticleData tpd( key );
      // calculate approximate mass
      // WARNING: any calls to particle() from here MUST reference
      //          a ParticleData which is already in the table
      // This convention is enforced.
      const ParticleData* proton = pdt.particle( 2212 );
      if ( proton ) {
        double protonMass = proton->mass();
        tpd.tempMass      = Measurement( key.A() * protonMass, 0. );
        // now create CommonParticleData
        cpd = new CommonParticleData( tpd );
      }
    }
    return cpd;
  }
} // namespace HepPDT
