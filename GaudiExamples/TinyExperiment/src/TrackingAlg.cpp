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
#include "Track.h"

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Transformer.h>

#include <cmath>
#include <vector>

namespace Gaudi::Example::TinyExperiment {

  /**
   * Tracking algorithm building tracks from hits using a simplified Hough transform
   */
  class TrackingAlg : public Functional::Transformer<Tracks( Hits const& )> {
  public:
    TrackingAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Transformer( name, pSvcLocator, { { "HitsLocation", "/Event/Hits" } },
                       { "TracksLocation", "/Event/Tracks" } ) {}

    Tracks operator()( Hits const& hits ) const override {
      // hough transform, actually only computing theta, and not dealing properly with borders of bins
      std::vector<unsigned int> bins( m_nBins, 0 );
      for ( auto const& hit : hits ) {
        auto theta = std::atan( hit.y / hit.x );
        auto index = static_cast<unsigned int>( ( theta + M_PI / 2 ) / M_PI * m_nBins );
        ++bins.at( index );
      }
      // extract tracks
      Tracks tracks;
      tracks.reserve( hits.size() / 10 );
      for ( unsigned int n = 0; n < m_nBins; n++ ) {
        if ( bins[n] > 0 ) {}
        if ( bins[n] >= m_sensibility ) {
          tracks.emplace_back( -M_PI / 2 + ( M_PI * ( n + 0.5f ) ) / m_nBins );
          ++n_tracks;
        }
      }
      return tracks;
    };

  private:
    Gaudi::Property<unsigned int> m_nBins{ this, "NumberBins", 180, "Number of bins in the Hough Transform for theta" };
    Gaudi::Property<unsigned int> m_sensibility{ this, "Sensibility", 6,
                                                 "How many hits do we want for considering we have a track ?" };
    mutable Gaudi::Accumulators::Counter<> n_tracks{ this, "Number of Tracks" };
  };

  DECLARE_COMPONENT( TrackingAlg )

} // namespace Gaudi::Example::TinyExperiment
