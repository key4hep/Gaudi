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

#include "IRandomGenSvc.h"
#include "MCTrack.h"

#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Functional/Transformer.h>

#include <cmath>
#include <random>

namespace Gaudi::Example::TinyExperiment {

  /**
   * Generator algorithm returning a set of random tracks
   */
  class GeneratorAlg : public Functional::Transformer<MCTracks( EventContext const& )> {
  public:
    GeneratorAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Transformer( name, pSvcLocator, { "MCTracksLocation", "/Event/MCTracks" } ) {}

    MCTracks operator()( EventContext const& ctx ) const override {
      MCTracks tracks;
      tracks.reserve( m_nbTracksToGenerate );
      std::normal_distribution dist( 0.0f, (float)M_PI / 4 ); // µ, σ
      auto                     engine = m_rndSvc->getEngine( ctx.evt() );
      for ( unsigned int i = 0; i < m_nbTracksToGenerate; i++ ) {
        auto theta = dist( engine );
        m_thetas += theta;
        tracks.emplace_back( theta );
      }
      return tracks;
    };

  private:
    ServiceHandle<IRandomGenSvc>                    m_rndSvc{ this, "RandomGenSvc", "RandomGenSvc",
                                           "A service providing a thread safe random number generator" };
    Gaudi::Property<unsigned int>                   m_nbTracksToGenerate{ this, "NbTracksToGenerate", 10 };
    mutable Gaudi::Accumulators::AveragingCounter<> m_thetas{ this, "Theta values" };
  };

  DECLARE_COMPONENT( GeneratorAlg )

} // namespace Gaudi::Example::TinyExperiment
