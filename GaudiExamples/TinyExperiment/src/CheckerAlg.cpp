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
#include "Track.h"

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Consumer.h>

#include <climits>

namespace Gaudi::Example::TinyExperiment {

  /**
   * Comparison of Reconstructed Tracks and MCTracks
   */
  class CheckerAlg : public Functional::Consumer<void( MCTracks const&, Tracks const& )> {
  public:
    CheckerAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Consumer(
              name, pSvcLocator,
              { KeyValue{ "MCTracksLocation", "/Event/MCTracks" }, KeyValue{ "TracksLocation", "/Event/Tracks" } } ) {}

    void operator()( MCTracks const& mcTracks, Tracks const& tracks ) const override {
      for ( auto const& track : tracks ) {
        MCTracks selection;
        selection.reserve( 10 );
        std::copy_if( mcTracks.begin(), mcTracks.end(), std::back_inserter( selection ),
                      [track, this]( MCTrack mcTrack ) { return almostEqual( track.theta, mcTrack.theta ); } );
        if ( selection.size() == 0 ) {
          ++n_ghostTracks;
        } else {
          ++n_foundTracks;
        }
      }
      for ( auto const& mcTrack : mcTracks ) {
        Tracks selection;
        selection.reserve( 10 );
        std::copy_if( tracks.begin(), tracks.end(), std::back_inserter( selection ),
                      [mcTrack, this]( Track track ) { return almostEqual( track.theta, mcTrack.theta ); } );
        if ( selection.size() == 0 ) {
          ++n_lostTracks;
        } else if ( selection.size() > 1 ) {
          ++n_duplicatedTracks;
        }
      }
    }

  private:
    bool almostEqual( float a, float b ) const { return std::abs( a - b ) <= m_maxDeltaTheta; }

    Gaudi::Property<float>                 m_maxDeltaTheta{ this, "DeltaThetaMax", 0.001,
                                            "(Relative) maximum delta theta to consider 2 tracks as identical" };
    mutable Gaudi::Accumulators::Counter<> n_foundTracks{ this, "Number of Tracks properly reconstructed" };
    mutable Gaudi::Accumulators::Counter<> n_ghostTracks{ this, "Number of ghost Tracks" };
    mutable Gaudi::Accumulators::Counter<> n_duplicatedTracks{ this, "Number of Tracks reconstructed twice" };
    mutable Gaudi::Accumulators::Counter<> n_lostTracks{ this, "Number of Tracks missing after reconstruction" };
  };

  DECLARE_COMPONENT( CheckerAlg )

} // namespace Gaudi::Example::TinyExperiment
