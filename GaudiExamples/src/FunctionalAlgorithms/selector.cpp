#include <algorithm>
#include <atomic>

#include "GaudiAlg/FilterPredicate.h"

// Event Model related classes
#include "GaudiExamples/MyTrack.h"

namespace Gaudi
{
  namespace Examples
  {

    class CountSelectedTracks
        : public Functional::FilterPredicate<bool( const Gaudi::Range_<Gaudi::Examples::MyTrack::ConstVector>& )>
    {
    public:
      CountSelectedTracks( const std::string& name, ISvcLocator* pSvc )
          : FilterPredicate( name, pSvc, {KeyValue{"InputData", Functional::concat_alternatives(
                                                                    "BogusLocation", "MoreBogus", "MyOutTracks" )}} )
      {
      }

      StatusCode initialize() override
      {
        StatusCode sc = FilterPredicate::initialize();
        if ( !sc ) return sc;
        m_tracksCount = 0;
        m_eventsCount = 0;
        return sc;
      }

      bool operator()( const Gaudi::Range_<Gaudi::Examples::MyTrack::ConstVector>& in_tracks ) const override
      {
        ++m_eventsCount;
        m_tracksCount += in_tracks.size();
        return true;
      }

      StatusCode finalize() override
      {
        info() << "extracted " << m_tracksCount << " tracks in " << m_eventsCount << " events" << endmsg;
        return FilterPredicate::finalize();
      }

    private:
      mutable std::atomic<long> m_tracksCount{0};
      mutable std::atomic<long> m_eventsCount{0};
    };

    DECLARE_COMPONENT( CountSelectedTracks )
  }
}
