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

#include "Gaudi/MonitoringHub.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"

#include <boost/algorithm/string.hpp>

#include <fmt/format.h>

#include <deque>
#include <map>

namespace {
  template <typename stream>
  stream printCounter( stream& log, const std::string& id, const nlohmann::json& j ) {
    // we have either counter of message json types, only counter have
    auto type = j.at( "type" ).get<std::string>();
    if ( type == "statentity" ) {
      // backward compatible code for StatEntity support. Should be dropped together
      // with StatEntity when migration to new counters is over
      using boost::algorithm::icontains;
      type = ( icontains( id, "eff" ) || icontains( id, "acc" ) || icontains( id, "filt" ) || icontains( id, "fltr" ) ||
               icontains( id, "pass" ) )
                 ? "counter:BinomialCounter"
                 : "counter:StatCounter";
    }
    // nEntries is common to all counters
    auto nEntries = j.at( "nEntries" ).get<unsigned int>();
    if ( type == "counter:AveragingCounter" ) {
      auto sum  = j.at( "sum" ).get<double>();
      auto mean = j.at( "mean" ).get<double>();
      return log << fmt::format( " | {:48s}|{:10d} |{:11.7g} |{:#11.5g} |", "\"" + id + "\"", nEntries, sum, mean );
    } else if ( type == "counter:SigmaCounter" ) {
      auto sum    = j.at( "sum" ).get<double>();
      auto mean   = j.at( "mean" ).get<double>();
      auto stddev = j.at( "standard_deviation" ).get<double>();
      return log << fmt::format( " | {:48s}|{:10d} |{:11.7g} |{:#11.5g} |{:#11.5g} |", "\"" + id + "\"", nEntries, sum,
                                 mean, stddev );
    } else if ( type == "counter:StatCounter" ) {
      auto sum    = j.at( "sum" ).get<double>();
      auto mean   = j.at( "mean" ).get<double>();
      auto stddev = j.at( "standard_deviation" ).get<double>();
      auto min    = j.at( "min" ).get<double>();
      auto max    = j.at( "max" ).get<double>();
      return log << fmt::format( " | {:48s}|{:10d} |{:11.7g} |{:#11.5g} |{:#11.5g} |{:#12.5g} |{:#12.5g} |",
                                 "\"" + id + "\"", nEntries, sum, mean, stddev, min, max );
    } else if ( type == "counter:BinomialCounter" ) {
      auto nTrueEntries  = j.at( "nTrueEntries" ).get<unsigned int>();
      auto efficiency    = j.at( "efficiency" ).get<double>();
      auto efficiencyErr = j.at( "efficiencyErr" ).get<double>();
      return log << fmt::format( " |*{:48s}|{:10d} |{:11d} |({:#9.7g} +- {:-#8.7g})% |", "\"" + id + "\"", nEntries,
                                 nTrueEntries, efficiency * 100, efficiencyErr * 100 );
    } else if ( std::string_view( type ).substr( 0, 10 ) == "histogram:" ) {
      return log << fmt::format( " | {:48s}H{:10d} |", "\"" + id + "\"", nEntries );
    } else { // counter:Counter or counter:MsgCounter
      return log << fmt::format( " | {:48s}|{:10d} |", "\"" + id + "\"", nEntries );
    }
  }
} // namespace

namespace Gaudi::Monitoring {

  class MessageSvcSink : public Service, public Hub::Sink {

  public:
    using Service::Service;

    /// initialization, registers to Monitoring::Hub
    StatusCode initialize() override {
      return Service::initialize().andThen( [&] {
        // declare ourself as a monitoding sink
        serviceLocator()->monitoringHub().addSink( this );
      } );
    }

    /// stop method, handles the printing
    StatusCode stop() override;

    // Gaudi::Monitoring::Hub::Sink implementation
    void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override {
      if ( std::string_view( ent.type ).substr( 0, 8 ) == "counter:" || ent.type == "statentity" ||
           ent.type == "histogram" ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

  private:
    std::deque<Hub::Entity> m_monitoringEntities;
  };

  DECLARE_COMPONENT( MessageSvcSink )
} // namespace Gaudi::Monitoring

StatusCode Gaudi::Monitoring::MessageSvcSink::stop() {
  // We will try to mimic the old monitoring of counters, so we need to split
  // them per Algo. The algo name can be extracted form the id of the entity
  // as its format is "algoName/counterName"
  // This map groups entities per algoName. For each name, the submap gives
  // the counter name of each subentity and the associated json
  std::map<std::string, std::map<std::string, nlohmann::json>> sortedEntities;
  // fill the sorted map
  for ( auto& entity : m_monitoringEntities ) { sortedEntities[entity.component][entity.name] = entity.toJSON(); }
  // dump all counters
  for ( auto& [algoName, entityMap] : sortedEntities ) {
    // check first whether there is any counter to log
    unsigned int nbCounters =
        std::accumulate( begin( entityMap ), end( entityMap ), 0, []( const unsigned int& a, const auto& j ) {
          return a + ( j.second.at( "empty" ).template get<bool>() ? 0 : 1 );
        } );
    if ( 0 == nbCounters ) continue;
    MsgStream log{msgSvc(), algoName};
    log << MSG::INFO << "Number of counters : " << nbCounters << "\n"
        << " |    Counter                                      |     #     |   "
        << " sum     | mean/eff^* | rms/err^*  |     min     |     max     |";
    std::for_each( begin( entityMap ), end( entityMap ), [&log]( auto& p ) {
      // Do not print empty counters
      if ( !p.second.at( "empty" ).template get<bool>() ) {
        log << "\n";
        printCounter( log, p.first, p.second );
      }
    } );
    log << endmsg;
  }
  return Service::stop();
}
