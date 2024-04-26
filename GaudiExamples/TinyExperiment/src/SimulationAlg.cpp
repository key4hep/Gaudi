/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "MCHit.h"
#include "MCTrack.h"

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Transformer.h>

#include <cmath>

namespace {
  std::pair<float, float> sincos( float arg ) { return { std::sin( arg ), std::cos( arg ) }; }
} // namespace

namespace Gaudi::Example::TinyExperiment {

  /**
   * Simulation algorithm creating MC Hits from a set of tracks
   */
  class SimulationAlg : public Functional::Transformer<MCHits( MCTracks const& )> {
  public:
    SimulationAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Transformer( name, pSvcLocator, { { "MCTracksLocation", "/Event/MCTracks" } },
                       { "MCHitsLocation", "/Event/MCHits" } ) {}

    MCHits operator()( MCTracks const& tracks ) const override {
      MCHits hits;
      hits.reserve( tracks.size() * m_nbHitsPerTrack );
      for ( auto const& track : tracks ) {
        auto [s, c] = sincos( track.theta );
        for ( unsigned int i = 0; i < m_nbHitsPerTrack; i++ ) {
          hits.emplace_back( i * c, i * s );
          ++n_hits;
        }
      }
      return hits;
    };

  private:
    Gaudi::Property<unsigned int>          m_nbHitsPerTrack{ this, "NbHitsPerTrack", 10 };
    mutable Gaudi::Accumulators::Counter<> n_hits{ this, "Number of MCHits" };
  };

  DECLARE_COMPONENT( SimulationAlg )

} // namespace Gaudi::Example::TinyExperiment
