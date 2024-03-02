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
#include <Gaudi/Functional/FilterPredicate.h>
#include <GaudiTestSuite/MyTrack.h>
#include <algorithm>
#include <atomic>

namespace Gaudi {
  namespace TestSuite {

    class CountSelectedTracks final
        : public Functional::FilterPredicate<bool( const Gaudi::Range_<Gaudi::TestSuite::MyTrack::ConstVector>& )> {
    public:
      CountSelectedTracks( const std::string& name, ISvcLocator* pSvc )
          : FilterPredicate( name, pSvc, { KeyValue{ "InputData", "MyOutTracks" } } ) {}

      StatusCode initialize() override {
        StatusCode sc = FilterPredicate::initialize();
        if ( !sc ) return sc;
        m_tracksCount = 0;
        m_eventsCount = 0;
        return sc;
      }

      bool operator()( const Gaudi::Range_<Gaudi::TestSuite::MyTrack::ConstVector>& in_tracks ) const override {
        ++m_eventsCount;
        m_tracksCount += in_tracks.size();
        return true;
      }

      StatusCode finalize() override {
        info() << "extracted " << m_tracksCount << " tracks in " << m_eventsCount << " events" << endmsg;
        return FilterPredicate::finalize();
      }

    private:
      mutable std::atomic<long> m_tracksCount{ 0 };
      mutable std::atomic<long> m_eventsCount{ 0 };
    };

    DECLARE_COMPONENT( CountSelectedTracks )
  } // namespace TestSuite
} // namespace Gaudi
