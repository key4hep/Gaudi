/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/Transformer.h>
#include <GaudiTestSuite/MyTrack.h>
#include <algorithm>

namespace Gaudi {
  namespace TestSuite {

    struct SelectTracks final : Functional::Transformer<Gaudi::TestSuite::MyTrack::Selection(
                                    const Gaudi::Range_<Gaudi::TestSuite::MyTrack::ConstVector>& )> {

      SelectTracks( const std::string& name, ISvcLocator* pSvc )
          : Transformer( name, pSvc, { KeyValue( "InputData", { "MyTracks" } ) },
                         KeyValue( "OutputData", { "MyOutTracks" } ) ) {}

      Gaudi::TestSuite::MyTrack::Selection
      operator()( const Gaudi::Range_<Gaudi::TestSuite::MyTrack::ConstVector>& in_tracks ) const override {
        Gaudi::TestSuite::MyTrack::Selection out_tracks;
        out_tracks.insert( in_tracks.begin(), in_tracks.end(), []( const MyTrack* t ) { return t->px() >= 10.; } );
        return out_tracks;
      }
    };

    DECLARE_COMPONENT( SelectTracks )
  } // namespace TestSuite
} // namespace Gaudi
