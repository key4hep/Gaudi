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

#include "Gaudi/BaseSink.h"
#include "Gaudi/MonitoringHub.h"
#include "GaudiKernel/MsgStream.h"

#include <boost/algorithm/string.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

#include <map>
#include <string_view>

#if FMT_VERSION < 80000
namespace fmt {
  template <typename T>
  const T& runtime( const T& v ) {
    return v;
  }
} // namespace fmt
#endif

namespace {

  /**
   * map of formating strings for the different Entities printed here
   * note that the format of each entry has to be something like {0:name|fmt} where :
   *  - 0 is mandatory and tells that all entries will use the same unique input (the json object)
   *  - name is the name of the entry in the json dictionnary to be printed
   *  - | only separates the name and the format part
   *  - fmt is the standard formater for your type, e.g. 9.7f or so
   * Note that fmt does only support 'd', 'g' and 'p' as the letter for the format at this time
   * and that 'p' means 'percentage', that is it's the same as 'g' where the value displayed is
   * first multiplies by 100.
   */
  static const auto registry = std::map<std::string_view, std::string_view>{
      { "counter", "{0:nEntries|10d}" },   // all unknown counters, and default
      { "histogram", "{0:nEntries|10d}" }, // all histograms
      { "counter:AveragingCounter", "{0:nEntries|10d} |{0:sum|11.7g} |{0:mean|#11.5g}" },
      { "counter:SigmaCounter", "{0:nEntries|10d} |{0:sum|11.7g} |{0:mean|#11.5g} |{0:standard_deviation|#11.5g}" },
      { "counter:StatCounter", "{0:nEntries|10d} |{0:sum|11.7g} |{0:mean|#11.5g} |{0:standard_deviation|#11.5g} "
                               "|{0:min|#12.5g} |{0:max|#12.5g}" },
      { "counter:BinomialCounter",
        "{0:nEntries|10d} |{0:nTrueEntries|11d} |({0:efficiency|#9.7p} +- {0:efficiencyErr|-#8.7p})%" },
  };

  // Helper to fix custom formatting of nlohmann::json version 3.10.5
  // See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/220
  struct json_fmt_arg {
    json_fmt_arg( const nlohmann::json& j ) : payload{ j } {}
    const nlohmann::json& payload;
  };
} // namespace

/**
 * fmt formatter function for json class
 * able to handle 2 types of formats :
 *   {} : in this case the type entry of json is used to deduce
 *        what to print, looking into the registry
 *   {:name|fmt} : in this case, the entry 'name' of the json
 *        will be printed in given format. See comment on registry
 *        for more details
 */
template <>
class fmt::formatter<json_fmt_arg> {
public:
  template <typename ParseContext>
  constexpr auto parse( ParseContext& ctx ) {
    auto fmt_begin = ctx.begin();
    auto fmt_end   = std::find( fmt_begin, ctx.end(), '}' );
    if ( fmt_begin == fmt_end ) {
      // we are dealing with {}, only make sure currentFormat is empty
      currentFormat = "";
    } else {
      // non empty format, let's split name from format
      auto fmt_colon = std::find( fmt_begin, fmt_end, '|' );
      currentName    = std::string( fmt_begin, fmt_colon - fmt_begin );
      currentFormat  = std::string( fmt_colon + 1, fmt_end - fmt_colon - 1 );
    }
    return fmt_end;
  }
  template <typename FormatContext>
  auto format( const json_fmt_arg& json_arg, FormatContext& ctx ) {
    const auto& j = json_arg.payload;
    if ( currentFormat.size() == 0 ) {
      // dealing with {} format, let's find entry for our type in registry
      const auto type = j.at( "type" ).get<std::string>();
      // first looking for the entry, then we drop on ":abc" suffix at a time and try again
      std::string_view type_key{ type };
      // look for the full entry
      auto entry = registry.find( type_key );
      // we check if we have type separators before entering the loop
      auto sep = type_key.rfind( ':' );
      while ( sep != type_key.npos && entry == registry.end() ) {
        // not found, remove the trailing ":abc" section
        type_key.remove_suffix( type_key.size() - sep );
        // check if we have another chunk to strip
        sep = type_key.rfind( ':' );
        // see if the shorter key works
        entry = registry.find( type_key );
      }
      // if still not found, we use the basic "counter"
      if ( entry == registry.end() ) entry = registry.find( "counter" );
      assert( entry != registry.end() );
      // print the json string according to format found
      // This actually will call this formatter again a number of times
      return fmt::format_to( ctx.out(), fmt::runtime( entry->second ), json_arg );
    } else {
      // dealing with a {:name|fmt} format
      auto actualFormat = "{:" + currentFormat + '}';
      switch ( currentFormat.back() ) {
      case 'd':
        return fmt::format_to( ctx.out(), fmt::runtime( actualFormat ),
                               j.at( currentName ).template get<unsigned int>() );
      case 'g':
        return fmt::format_to( ctx.out(), fmt::runtime( actualFormat ), j.at( currentName ).template get<double>() );
      case 'p':
        actualFormat[actualFormat.size() - 2] = 'g';
        return fmt::format_to( ctx.out(), fmt::runtime( actualFormat ),
                               j.at( currentName ).template get<double>() * 100 );
      default:
        return fmt::format_to( ctx.out(), "Unknown counter format : {}", currentFormat );
      }
    }
  }

private:
  std::string currentFormat;
  std::string currentName;
};

namespace {

  template <typename Stream>
  Stream& printCounter( Stream& log, std::string_view id, const nlohmann::json& j ) {
    const auto type = j.at( "type" ).get<std::string>();
    // for backward compatibility, we need to deal with statentity in a special way
    // this block should be dropped when StatEntities are gone
    if ( type == "statentity" ) {
      using boost::algorithm::icontains;
      bool isBinomial = icontains( id, "eff" ) || icontains( id, "acc" ) || icontains( id, "filt" ) ||
                        icontains( id, "fltr" ) || icontains( id, "pass" );
      auto nj    = j;
      nj["type"] = isBinomial ? "counter:BinomialCounter" : "counter:StatCounter";
      return printCounter( log, id, nj );
    }
    // binomial counters are slightly different ('*' character)
    return log << fmt::format( " |{}{:48}|{} |",
                               ( std::string_view{ type }.substr( 0, 23 ) == "counter:BinomialCounter" ? '*' : ' ' ),
                               fmt::format( "\"{}\"", id ), json_fmt_arg{ j } );
  }

} // namespace

namespace Gaudi::Monitoring {

  struct MessageSvcSink : BaseSink {
    MessageSvcSink( std::string name, ISvcLocator* svcloc ) : BaseSink( name, svcloc ) {
      // only deal with counters, statentity and histograms
      setProperty( "TypesToSave", std::vector<std::string>{ "counter:.*", "statentity", "histogram:" } )
          .orThrow( "Unable to set typesToSaveProperty", "Histograming::Sink::Base" );
    }
    /// stop method, handles the printing
    void flush( bool ) override;
  };

  DECLARE_COMPONENT( MessageSvcSink )
} // namespace Gaudi::Monitoring

void Gaudi::Monitoring::MessageSvcSink::flush( bool ) {
  // We will try to mimic the old monitoring of counters, so we need to split
  // them per Algo. The algo name can be extracted form the id of the entity
  // as its format is "algoName/counterName"
  // This map groups entities per algoName. For each name, the submap gives
  // the counter name of each subentity and the associated json
  std::map<std::string, std::map<std::string, nlohmann::json>> sortedEntities;
  // fill the sorted map
  applyToAllEntities( [&sortedEntities]( auto& ent ) { sortedEntities[ent.component][ent.name] = ent.toJSON(); } );
  // dump all counters
  for ( auto& [algoName, entityMap] : sortedEntities ) {
    // check first whether there is any counter to log
    unsigned int nbCounters =
        std::accumulate( begin( entityMap ), end( entityMap ), 0, []( const unsigned int& a, const auto& j ) {
          return a + ( j.second.at( "empty" ).template get<bool>() ? 0 : 1 );
        } );
    if ( 0 == nbCounters ) continue;
    MsgStream log{ msgSvc(), algoName };
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
}
