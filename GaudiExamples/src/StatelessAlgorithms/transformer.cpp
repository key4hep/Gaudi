#include <algorithm>
#include <atomic>

#include "GaudiAlg/TransformAlgorithm.h"
#include "GaudiAlg/FilterAlgorithm.h"

// Event Model related classes
#include "GaudiExamples/MyTrack.h"

namespace Gaudi { namespace Examples {

  class SelectTracks: public TransformAlgorithm<MyTrackVector(const MyTrackVector&)> {
  public:
    SelectTracks(const std::string& name, ISvcLocator* pSvc):
      TransformAlgorithm(name, pSvc, 
                         { KeyValue("InputData",{"MyTracks"}) },
                           KeyValue("OutputData",{"MyOutTracks"}) ) {
    }

    MyTrackVector operator()(const MyTrackVector& in_tracks) const {
      MyTrackVector out_tracks;
      std::for_each(in_tracks.begin(), in_tracks.end(),
                    [&out_tracks, this](const MyTrack* t) {
                      if (t->px() >= 10.) {
                        out_tracks.add(new MyTrack(*t));
                      }
                    });
      return out_tracks;
    }
  };

  DECLARE_COMPONENT(SelectTracks)


  class CountSelectedTracks: public FilterAlgorithm<void(const MyTrackVector&)> {
  public:
    CountSelectedTracks(const std::string& name, ISvcLocator* pSvc):
      FilterAlgorithm(name, pSvc, 
                      { KeyValue("InputData",{"MyOutTracks"}) }) {
    }
    StatusCode initialize() {
      StatusCode sc = FilterAlgorithm::initialize();
      if (sc) {
         m_tracksCount = 0;
         m_eventsCount = 0;
      }
      return sc;
    }
    bool operator()(const MyTrackVector& in_tracks) const {
      ++m_eventsCount;
      m_tracksCount += in_tracks.size();
      return true;
    }

    StatusCode finalize() {
      info() << "extracted " << m_tracksCount << " tracks in " << m_eventsCount << " events" << endmsg;
      return FilterAlgorithm::finalize();
    }
  private:
    mutable std::atomic<long> m_tracksCount{0};
    mutable std::atomic<long> m_eventsCount{0};
  };

  DECLARE_COMPONENT(CountSelectedTracks)
}}

