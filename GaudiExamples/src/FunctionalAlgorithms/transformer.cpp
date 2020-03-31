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
#include <algorithm>

#include "GaudiAlg/Transformer.h"

// Event Model related classes
#include "GaudiExamples/MyTrack.h"

namespace Gaudi {
  namespace Examples {

    struct SelectTracks final : Functional::Transformer<SelectTracks, Gaudi::Examples::MyTrack::Selection(
                                    const Gaudi::Range_<Gaudi::Examples::MyTrack::ConstVector>& )> {

      SelectTracks( const std::string& name, ISvcLocator* pSvc )
          : Transformer( name, pSvc, {KeyValue( "InputData", {"MyTracks"} )},
                         KeyValue( "OutputData", {"MyOutTracks"} ) ) {}

      Gaudi::Examples::MyTrack::Selection
      operator()( const Gaudi::Range_<Gaudi::Examples::MyTrack::ConstVector>& in_tracks ) const {
        Gaudi::Examples::MyTrack::Selection out_tracks;
        out_tracks.insert( in_tracks.begin(), in_tracks.end(), []( const MyTrack* t ) { return t->px() >= 10.; } );
        return out_tracks;
      }
    };

    DECLARE_COMPONENT( SelectTracks )
  } // namespace Examples
} // namespace Gaudi
