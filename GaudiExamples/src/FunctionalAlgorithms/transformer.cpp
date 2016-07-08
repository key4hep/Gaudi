#include <algorithm>
#include <atomic>

#include "GaudiAlg/Transformer.h"
#include "GaudiAlg/FilterPredicate.h"

// Event Model related classes
#include "GaudiExamples/MyTrack.h"

namespace Gaudi { namespace Examples {

  class SelectTracks: public Functional::Transformer<MyTrackVector(const MyTrackVector&),Functional::useDataObjectHandle> {
  public:
    SelectTracks(const std::string& name, ISvcLocator* pSvc):
      Transformer(name,
                  pSvc, 
                  { KeyValue("InputData",{"MyTracks"}) },
                  KeyValue("OutputData",{"MyOutTracks"}) ) {
    }

    MyTrackVector operator()(const MyTrackVector& in_tracks) const override {
      MyTrackVector out_tracks;
      std::for_each(in_tracks.begin(), in_tracks.end(),
                    [&out_tracks](const MyTrack* t) {
                      if (t->px() >= 10.) out_tracks.add(new MyTrack(*t));
                    });
      return out_tracks;
    }
  };

  DECLARE_COMPONENT(SelectTracks)

 
}}
