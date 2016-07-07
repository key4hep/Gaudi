#include <algorithm>
#include <atomic>

#include "GaudiAlg/Transformer.h"
#include "GaudiAlg/FilterPredicate.h"

// Event Model related classes
#include "GaudiExamples/MyTrack.h"

namespace Gaudi { namespace Examples {

  class SelectTracks: public Functional::Transformer<MyTrackVector(const MyTrackVector&)> {
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



  class CountSelectedTracks: public Functional::FilterPredicate<bool(const MyTrackVector&)> {
  public:
    CountSelectedTracks(const std::string& name, ISvcLocator* pSvc):
      FilterPredicate(name, pSvc, 
                      { KeyValue("InputData",{"BogusLocation&MoreBogus&MyOutTracks"}) } ) {

    }

    StatusCode initialize() override {
      StatusCode sc = FilterPredicate::initialize();
      if (!sc) return sc;
      m_tracksCount = 0;
      m_eventsCount = 0;
      return sc;
    }

    bool operator()(const MyTrackVector& in_tracks) const override {
      ++m_eventsCount;
      m_tracksCount += in_tracks.size();
      return true;
    }

    StatusCode finalize() override {
      info() << "extracted " << m_tracksCount << " tracks in " << m_eventsCount << " events" << endmsg;
      return FilterPredicate::finalize();
    }
  private:
    mutable std::atomic<long> m_tracksCount{0};
    mutable std::atomic<long> m_eventsCount{0};
  };

  DECLARE_COMPONENT(CountSelectedTracks)

 
}}
