#include <algorithm>
#include <atomic>

#include "GaudiAlg/TransformAlgorithm.h"

// Event Model related classes
//#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"
//#include "GaudiExamples/Counter.h"

namespace Gaudi { namespace Examples {

  class SelectTracks: public TransformAlgorithm<MyTrackVector(const MyTrackVector&)> {
  public:
    SelectTracks(const std::string& name, ISvcLocator* pSvc):
      TransformAlgorithm(name, pSvc, 
                         { KeyValue("InputData",{}) },
                           KeyValue("OutputData",{}) ) {
      setProperty("InputData", "MyTracks");
      setProperty("OutputData", "MyOutTracks");
    }
    StatusCode initialize() {
      StatusCode sc = TransformAlgorithm::initialize();
      if (sc) {
         m_tracksCount = 0;
         m_eventsCount = 0;
      }
      return sc;
    }
    MyTrackVector operator()(const MyTrackVector& in_tracks) const {
      MyTrackVector out_tracks;
      ++m_eventsCount;
      std::for_each(in_tracks.begin(), in_tracks.end(),
                    [&out_tracks, this](const MyTrack* t) {
                      if (t->px() >= 10.) {
                        out_tracks.add(new MyTrack(*t));
                        ++m_tracksCount;
                      }
                    });
      //info() << (*in_tracks.begin())->px() << endmsg;
      return out_tracks;
    }
    StatusCode finalize() {
      info() << "extracted " << m_tracksCount << " tracks in " << m_eventsCount << " events" << endmsg;
      return TransformAlgorithm::finalize();
    }
  private:
    mutable std::atomic<long> m_tracksCount{0};
    mutable std::atomic<long> m_eventsCount{0};
  };

  DECLARE_COMPONENT(SelectTracks)
}}

