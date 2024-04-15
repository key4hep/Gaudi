/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/Transformer.h>
#include <GaudiKernel/IRndmEngine.h>
#include <GaudiKernel/IRndmGenSvc.h>
#include <GaudiKernel/RndmGenerators.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <GaudiKernel/Vector4DTypes.h>
#include <Math/Vector4D.h>
#include <cmath>
#include <mylee/mc/event.h>
#include <mylee/mc/gen_header.h>
#include <mylee/mc/particle.h>
#include <mylee/mc/vertex.h>
#include <utility>
#include <vector>

namespace mylee::mc::gen {
  struct elastic : Gaudi::Functional::Transformer<event( const gen_header& )> {
    elastic( const std::string& name, ISvcLocator* svcLoc )
        : Transformer{ name, svcLoc, { KeyValue{ "header", "gen/header" } }, KeyValue{ "event", "gen/event" } } {}

    StatusCode initialize() override {
      return Transformer::initialize().andThen( [this]() {
        m_rnd_flat = randSvc()->generator( Rndm::Flat{ 0., 1. } );
        if ( !m_rnd_flat ) throw GaudiException( "failed to retrieve random generator", name(), StatusCode::FAILURE );
      } );
    }

    event operator()( const gen_header& header ) const override {
      randSvc()->engine()->setSeeds( header.seeds ).orThrow( "failed to set the random seeds", name() );

      namespace u = Gaudi::Units;

      event evt;

      // from flat distribution [0,1]  to [-1,1] mm
      const auto v_x = ( m_rnd_flat->shoot() * 2.0 - 1.0 ) * u::mm;
      const auto v_y = ( m_rnd_flat->shoot() * 2.0 - 1.0 ) * u::mm;
      const auto v_z = ( m_rnd_flat->shoot() * 2.0 - 1.0 ) * u::mm;
      vertex     v;
      v.id       = 0;
      v.position = { static_cast<float>( v_x ), static_cast<float>( v_y ), static_cast<float>( v_z ) };
      evt.vertices.emplace_back( std::move( v ) );

      // flat energy distribution between 100 and 150 GeV
      const auto energy      = ( m_rnd_flat->shoot() * 50.0 + 100.0 ) * u::GeV;
      const auto proton_mass = 0.938 * u::GeV;

      // outgoing 4-momentum
      const auto theta = ( m_rnd_flat->shoot() * 180.0 ) * u::deg;
      const auto phi   = ( m_rnd_flat->shoot() * 360.0 - 180.0 ) * u::deg;

      const auto eta = -std::log( std::tan( ( theta / 2.0 ) / u::rad ) );
      const auto pt  = std::sin( theta / u::rad ) * std::sqrt( energy * energy - proton_mass * proton_mass ); // u::GeV

      const ROOT::Math::PtEtaPhiMVector m{ pt, eta, phi, proton_mass };

      evt.particles.push_back(
          particle{ 0, 0, -1u, Gaudi::ParticleID{ 2212 }, { m.px(), m.py(), m.pz(), m.energy() } } );
      evt.particles.push_back(
          particle{ 1, 0, -1u, Gaudi::ParticleID{ 2212 }, { -m.px(), -m.py(), -m.pz(), m.energy() } } );
      evt.vertices[0].outgoing_particles = { { 0 }, { 1 } };

      return evt;
    }

    SmartIF<IRndmGen> m_rnd_flat;
  };
  DECLARE_COMPONENT( elastic )
} // namespace mylee::mc::gen
