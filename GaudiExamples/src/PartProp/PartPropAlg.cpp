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
#include <iostream>

#include "PartPropAlg.h"

Gaudi::Examples::PartPropAlg::PartPropAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Gaudi::Algorithm( name, pSvcLocator ) {}

StatusCode Gaudi::Examples::PartPropAlg::initialize() {
  StatusCode sc = Algorithm::initialize(); //     initialize the base
  if ( sc.isFailure() ) { return sc; }
  // locate particle property service
  Gaudi::Examples::PartPropAlg::ppSvc();
  return StatusCode::SUCCESS;
}

StatusCode Gaudi::Examples::PartPropAlg::finalize() {
  // release the aquired service
  m_ppSvc.reset();
  return Algorithm::finalize(); //        finalize the base
}

// locate the new particle property service
const Gaudi::Interfaces::IParticlePropertySvc* Gaudi::Examples::PartPropAlg::ppSvc() const {
  if ( !m_ppSvc ) {
    m_ppSvc = service( "Gaudi::ParticlePropertySvc", true );
    if ( !m_ppSvc ) {
      throw GaudiException( "Service [Gaudi::ParticlePropertySvc] not found", name(), StatusCode::FAILURE );
    }
  }
  return m_ppSvc.get();
}

// execute the algorithm
StatusCode Gaudi::Examples::PartPropAlg::execute( const EventContext& ctx ) const {
  // get the service
  const Gaudi::Interfaces::IParticlePropertySvc* svc = ppSvc();

  if ( !svc ) { return StatusCode::FAILURE; }
  if ( !ctx.valid() ) { return StatusCode::FAILURE; }

  // avoid long names
  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties PPs;

  MsgStream log( msgSvc(), name() );

  { // get all invalid
    PPs invalid;
    svc->get(
        // functor : invalid
        []( const Gaudi::ParticleProperty* pp ) { return !pp->pid().isValid(); },
        // output
        std::back_inserter( invalid ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Invalid = " << invalid.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( invalid, log, svc );
    log << endmsg;
  }

  { // get all not from quarks
    PPs noquarks;
    svc->get(
        // functor : has no quarks
        []( const Gaudi::ParticleProperty* pp ) { return !pp->pid().hasQuarks(); },
        // output
        std::back_inserter( noquarks ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Has no quarks = " << noquarks.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( noquarks, log, svc );
    log << endmsg;
  }

  { // get all 'fundamental'
    PPs fundamental;
    svc->get(
        // functor : fundamental
        []( const Gaudi::ParticleProperty* pp ) {
          auto fid = pp->pid().fundamentalID();
          return 0 < fid && 100 >= fid;
        },
        // output
        std::back_inserter( fundamental ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Fundamental (0,100] = " << fundamental.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( fundamental, log, svc );
    log << endmsg;
  }

  { // get all leptons
    PPs leptons;
    svc->get(
        // functor : lepton
        []( const Gaudi::ParticleProperty* pp ) { return pp->pid().isLepton(); },
        // output
        std::back_inserter( leptons ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Leptons = " << leptons.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( leptons, log, svc );
    log << endmsg;
  }

  { // get all long-lived (>1um)
    PPs longlived;
    svc->get(
        // functor : ctau>1um
        []( const Gaudi::ParticleProperty* pp ) { return pp->ctau() > 1 * Gaudi::Units::micrometer; },
        // output
        std::back_inserter( longlived ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Long-lived(>1mu) = " << longlived.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( longlived, log, svc );
    log << endmsg;
  }

  { // get all nuclea
    PPs nuclea;
    svc->get(
        // functor : nucleus
        []( const Gaudi::ParticleProperty* pp ) { return pp->pid().isNucleus(); },
        // output
        std::back_inserter( nuclea ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Nuclea  = " << nuclea.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( nuclea, log, svc );
    log << endmsg;
  }

  { // get all beauty baryons
    PPs bbaryons;
    svc->get(
        // functor : beauty & baryon
        []( const Gaudi::ParticleProperty* pp ) { return pp->pid().hasBottom() && pp->pid().isBaryon(); },
        // output
        std::back_inserter( bbaryons ) );
    // print as the table
    // header ?
    log << MSG::INFO << " # Beauty Baryons  = " << bbaryons.size() << std::endl;
    // content
    Gaudi::ParticleProperties::printAsTable( bbaryons, log, svc );
    log << endmsg;
  }

  return StatusCode::SUCCESS;
}

DECLARE_COMPONENT( Gaudi::Examples::PartPropAlg )
