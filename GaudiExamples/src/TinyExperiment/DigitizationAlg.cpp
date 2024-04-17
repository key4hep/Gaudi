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
#include "MCHit.h"

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Transformer.h>

#include <random>

namespace Gaudi::Example::TinyExperiment {

  /**
   * Digitization algorithm returning a set of hits from MCHits, basically adding noise
   */
  class DigitizationAlg : public Functional::Transformer<Hits( MCHits const& )> {
  public:
    DigitizationAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Transformer( name, pSvcLocator, { { "MCHitsLocation", "/Event/MCHits" } },
                       { "HitsLocation", "/Event/Hits" } ) {}

    StatusCode initialize() override {
      return Transformer::initialize().andThen( [&] { m_engine = std::default_random_engine( m_randomSeed ); } );
    }
    Hits operator()( MCHits const& mcHits ) const override {
      Hits hits;
      hits.reserve( mcHits.size() );
      std::normal_distribution dist( 0.0f, m_sigmaNoise.value() ); // µ, σ
      for ( auto const& mcHit : mcHits ) {
        auto nx = mcHit.x + dist( m_engine );
        auto ny = mcHit.y + dist( m_engine );
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
    mutable std::default_random_engine     m_engine; // FIXME this is not thread safe !
    Gaudi::Property<float>                 m_sigmaNoise{ this, "SigmaNoise", 1.f,
                                         "Sigma of the noise (a normal distribution centered on 0)" };
    Gaudi::Property<unsigned long>         m_randomSeed{ this, "RandomSeed", 0 };
    mutable Gaudi::Accumulators::Counter<> n_hits{ this, "Number of Hits" };
  };

  DECLARE_COMPONENT_WITH_ID( DigitizationAlg, "DigitizationAlg" )

} // namespace Gaudi::Example::TinyExperiment
