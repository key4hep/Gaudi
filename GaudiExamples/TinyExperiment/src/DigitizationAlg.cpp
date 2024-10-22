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

#include "Hit.h"
#include "IRandomGenSvc.h"
#include "MCHit.h"

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Transformer.h>

#include <random>

namespace Gaudi::Example::TinyExperiment {

  /**
   * Digitization algorithm returning a set of hits from MCHits, basically adding noise
   */
  class DigitizationAlg : public Functional::Transformer<Hits( EventContext const&, MCHits const& )> {
  public:
    DigitizationAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Transformer( name, pSvcLocator, { { "MCHitsLocation", "/Event/MCHits" } },
                       { "HitsLocation", "/Event/Hits" } ) {}

    Hits operator()( const EventContext& ctx, MCHits const& mcHits ) const override {
      Hits hits;
      hits.reserve( mcHits.size() );
      std::normal_distribution dist( 0.0f, m_sigmaNoise.value() ); // µ, σ
      auto                     engine = m_rndSvc->getEngine( ctx.evt() );
      for ( auto const& mcHit : mcHits ) {
        auto nx = mcHit.x + dist( engine );
        auto ny = mcHit.y + dist( engine );
        // ignore negative axis.
        // Simplifies reconstruction and simulate the apperture of the detector
        if ( nx > 0 ) {
          hits.emplace_back( nx, ny );
          ++n_hits;
        }
      }
      return hits;
    };

  private:
    ServiceHandle<IRandomGenSvc>           m_rndSvc{ this, "RandomGenSvc", "RandomGenSvc",
                                           "A service providing a thread safe random number generator" };
    Gaudi::Property<float>                 m_sigmaNoise{ this, "SigmaNoise", 1.f,
                                         "Sigma of the noise (a normal distribution centered on 0)" };
    mutable Gaudi::Accumulators::Counter<> n_hits{ this, "Number of Hits" };
  };

  DECLARE_COMPONENT( DigitizationAlg )

} // namespace Gaudi::Example::TinyExperiment
