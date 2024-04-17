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

#include "MCTrack.h"

#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Functional/Producer.h>

#include <cmath>
#include <random>

namespace Gaudi::Example::TinyExperiment {

  /**
   * Generator algorithm returning a set of random tracks
   */
  class GeneratorAlg : public Functional::Producer<MCTracks()> {
  public:
    GeneratorAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Producer( name, pSvcLocator, { "MCTracksLocation", "/Event/MCTracks" } ) {}

    StatusCode initialize() override {
      return Producer::initialize().andThen( [&] { m_engine = std::default_random_engine( m_randomSeed ); } );
    }

    MCTracks operator()() const override {
      // FIXME, we should use ranges here, something like
      // std::views::repeat(void) | std::views::transform(...) | std::ranges::take_view( m_nbTracksToGenerate )
      MCTracks tracks;
      tracks.reserve( m_nbTracksToGenerate );
      std::normal_distribution dist( 0.0f, (float)M_PI / 4 ); // µ, σ
      for ( unsigned int i = 0; i < m_nbTracksToGenerate; i++ ) {
        auto theta = dist( m_engine );
        ++m_thetas[theta];
        tracks.emplace_back( theta );
      }
      return tracks;
    };

  private:
    mutable std::default_random_engine m_engine; // FIXME this is not thread safe !
    Gaudi::Property<unsigned int>      m_nbTracksToGenerate{ this, "NbTracksToGenerate", 10 };
    Gaudi::Property<unsigned long>     m_randomSeed{ this, "RandomSeed", 0 };
    mutable Gaudi::Accumulators::Histogram<1, Accumulators::atomicity::full, float> m_thetas{
        this, "Theta values", "Theta", { 40, -1.57, 1.57 } };
  };

  DECLARE_COMPONENT_WITH_ID( GeneratorAlg, "GeneratorAlg" )

} // namespace Gaudi::Example::TinyExperiment
