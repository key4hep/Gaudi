#include <algorithm>

#include "GaudiAlg/Transformer.h"

// Event Model related classes
#include "GaudiExamples/MyTrack.h"

namespace Gaudi
{
  namespace Examples
  {

    struct SelectTracks : Functional::Transformer<MyTrackVector( const MyTrackVector& )> {

      SelectTracks( const std::string& name, ISvcLocator* pSvc )
          : Transformer( name, pSvc, {KeyValue( "InputData", {"MyTracks"} )},
                         KeyValue( "OutputData", {"MyOutTracks"} ) )
      {
      }

      MyTrackVector operator()( const MyTrackVector& in_tracks ) const override
      {
        MyTrackVector out_tracks;
        std::for_each( in_tracks.begin(), in_tracks.end(), [&out_tracks]( const MyTrack* t ) {
          if ( t->px() >= 10. ) out_tracks.add( new MyTrack( *t ) );
        } );
        return out_tracks;
      }
    };

    DECLARE_COMPONENT( SelectTracks )
  }
}
