/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/Accumulators/StaticHistogram.h>
#include <Gaudi/BaseSink.h>
#include <Gaudi/Histograming/Sink/Utils.h>
#include <Gaudi/MonitoringHub.h>
#include <GaudiKernel/MsgStream.h>

#include <boost/algorithm/string.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

#include <map>
#include <sstream>
#include <string_view>

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
      { "counter", "{0:nEntries|10d}" }, // all unknown counters, and default
      { "counter:AveragingCounter", "{0:nEntries|10d} |{0:sum|11.7g} |{0:mean|#11.5g}" },
      { "counter:SigmaCounter", "{0:nEntries|10d} |{0:sum|11.7g} |{0:mean|#11.5g} |{0:standard_deviation|#11.5g}" },
      { "counter:StatCounter", "{0:nEntries|10d} |{0:sum|11.7g} |{0:mean|#11.5g} |{0:standard_deviation|#11.5g} "
                               "|{0:min|#12.5g} |{0:max|#12.5g}" },
      { "counter:BinomialCounter",
        "{0:nEntries|10d} |{0:nTrueEntries|11d} |({0:efficiency|#9.7p} +- {0:efficiencyErr|-#8.7p})%" },
  };

  // the int says in which bunch of counters to put the histogram. We separate indeed
  // regular counters (0) from 1d/2d/3d histogram (1, 2 and 3) from 1d/2d/3d profile
  // histograms (4, 5 and 6)
  using HistoBinHandler =
      std::pair<unsigned int,
                std::function<std::string( std::string_view, Gaudi::Monitoring::Hub::Entity const&, unsigned int )>>;
  namespace Acc = ::Gaudi::Accumulators;
  /**
   * map of binary formating methods for dedicated types where we want to avoid going through json
   * data for performance reasons. Essentially Profile histograms at this stage
   */
  std::map<std::type_index, HistoBinHandler> const binRegistry = {
      { std::type_index( typeid( Acc::StaticProfileHistogram<1u, Acc::atomicity::full, double> ) ),
        { 4, &Gaudi::Histograming::Sink::printProfileHisto1D<Acc::atomicity::full, double> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<1u, Acc::atomicity::none, double> ) ),
        { 4, &Gaudi::Histograming::Sink::printProfileHisto1D<Acc::atomicity::none, double> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<1u, Acc::atomicity::full, float> ) ),
        { 4, &Gaudi::Histograming::Sink::printProfileHisto1D<Acc::atomicity::full, float> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<1u, Acc::atomicity::none, float> ) ),
        { 4, &Gaudi::Histograming::Sink::printProfileHisto1D<Acc::atomicity::none, float> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<2u, Acc::atomicity::full, double> ) ),
        { 5, &Gaudi::Histograming::Sink::printProfileHisto2D<Acc::atomicity::full, double> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<2u, Acc::atomicity::none, double> ) ),
        { 5, &Gaudi::Histograming::Sink::printProfileHisto2D<Acc::atomicity::none, double> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<2u, Acc::atomicity::full, float> ) ),
        { 5, &Gaudi::Histograming::Sink::printProfileHisto2D<Acc::atomicity::full, float> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<2u, Acc::atomicity::none, float> ) ),
        { 5, &Gaudi::Histograming::Sink::printProfileHisto2D<Acc::atomicity::none, float> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<3u, Acc::atomicity::full, double> ) ),
        { 6, &Gaudi::Histograming::Sink::printProfileHisto3D<Acc::atomicity::full, double> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<3u, Acc::atomicity::none, double> ) ),
        { 6, &Gaudi::Histograming::Sink::printProfileHisto3D<Acc::atomicity::none, double> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<3u, Acc::atomicity::full, float> ) ),
        { 6, &Gaudi::Histograming::Sink::printProfileHisto3D<Acc::atomicity::full, float> } },
      { std::type_index( typeid( Acc::StaticProfileHistogram<3u, Acc::atomicity::none, float> ) ),
        { 6, &Gaudi::Histograming::Sink::printProfileHisto3D<Acc::atomicity::none, float> } },
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
  auto format( const json_fmt_arg& json_arg, FormatContext& ctx ) const {
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

  void printCounter( std::ostringstream& log, std::string_view id, const nlohmann::json& j ) {
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
    // fmt::runtime is required when compiling with GCC 11 but
    // can be dropped when GCC 11 is no longer supported
    log << fmt::format( fmt::runtime( " |{}{:48}|{} |" ),
                        ( std::string_view{ type }.substr( 0, 23 ) == "counter:BinomialCounter" ? '*' : ' ' ),
                        fmt::format( fmt::runtime( "\"{}\"" ), id ), json_fmt_arg{ j } );
  }

} // namespace

namespace Gaudi::Monitoring {

  /**
   * Sink dedicated to printing messages to the MessageSvc.
   *
   * Deals with counters and histograms at this stage.
   * Relies on these entities to have a "type" entry in their json following
   * the convention described in Accumulators.h
   */
  struct MessageSvcSink : BaseSink {
    MessageSvcSink( std::string name, ISvcLocator* svcloc ) : BaseSink( name, svcloc ) {
      // only deal with counters, statentity and histograms
      setProperty( "TypesToSave", std::vector<std::string>{ "counter:.*", "statentity", "histogram:.*" } )
          .orThrow( "Unable to set typesToSaveProperty", "Histograming::Sink::Base" );
    }
    /// stop method, handles the printing
    void                          flush( bool ) override;
    Gaudi::Property<unsigned int> m_histoStringsWidth{ this, "HistoStringsWidth", 45,
                                                       "length of histograms names and titles in the output" };
  };

  DECLARE_COMPONENT( MessageSvcSink )
} // namespace Gaudi::Monitoring

void Gaudi::Monitoring::MessageSvcSink::flush( bool ) {
  std::string curAlgo = "";
  // an array of counters, separating (in this order)
  // regular counters from 1d / 2d / 3d histograms, from
  // 1d / 2d / 3d profile histograms
  std::array<std::ostringstream, 7> curLog;
  // associated counters of non empty entities, in order
  // to know whether to print or not the given section
  std::array<unsigned int, 7> nbNonEmptyEntities{};
  auto                        dumpAlgoCounters = [&]() {
    MsgStream log{ msgSvc(), curAlgo };
    // Regular counters first
    if ( nbNonEmptyEntities[0] > 0 ) {
      log << MSG::INFO << "Number of counters : " << nbNonEmptyEntities[0] << "\n"
          << " |    Counter                                      |     #     |   "
          << " sum     | mean/eff^* | rms/err^*  |     min     |     max     |";
      log << curLog[0].str() << endmsg;
    }
    // Now histograms
    unsigned int nHistos = nbNonEmptyEntities[1] + nbNonEmptyEntities[2] + nbNonEmptyEntities[3] +
                           nbNonEmptyEntities[4] + nbNonEmptyEntities[5] + nbNonEmptyEntities[6];
    if ( nHistos > 0 ) {
      log << MSG::INFO << "Booked " << nHistos << " Histogram(s) : 1D=" << nbNonEmptyEntities[1]
          << " 2D=" << nbNonEmptyEntities[2] << " 3D=" << nbNonEmptyEntities[3] << " 1DProf=" << nbNonEmptyEntities[4]
          << " 2DProf=" << nbNonEmptyEntities[5] << " 3DProf=" << nbNonEmptyEntities[6] << endmsg;
      if ( nbNonEmptyEntities[1] > 0 ) {
        log << MSG::INFO << "1D histograms in directory \"" << curAlgo << "\" : " << nbNonEmptyEntities[1] << "\n"
            << fmt::format(
                   fmt::runtime(
                       " | {:{}s} | {:{}s} |      #     |     Mean   |    RMS     |  Skewness  |  Kurtosis  |" ),
                   "ID", m_histoStringsWidth.value(), "Title", m_histoStringsWidth.value() )
            << curLog[1].str() << endmsg;
      }
      if ( nbNonEmptyEntities[2] > 0 ) {
        log << MSG::INFO << "2D histograms in directory \"" << curAlgo << "\" : " << nbNonEmptyEntities[2]
            << curLog[2].str() << endmsg;
      }
      if ( nbNonEmptyEntities[3] > 0 ) {
        log << MSG::INFO << "3D histograms in directory \"" << curAlgo << "\" : " << nbNonEmptyEntities[3]
            << curLog[3].str() << endmsg;
      }
      if ( nbNonEmptyEntities[4] > 0 ) {
        log << MSG::INFO << "1D profile histograms in directory \"" << curAlgo << "\" : " << nbNonEmptyEntities[4]
            << "\n"
            << fmt::format(
                   fmt::runtime(
                       " | {:{}s} | {:{}s} |      #     |     Mean   |    RMS     |  Skewness  |  Kurtosis  |" ),
                   "ID", m_histoStringsWidth.value(), "Title", m_histoStringsWidth.value() )
            << curLog[4].str() << endmsg;
      }
      if ( nbNonEmptyEntities[5] > 0 ) {
        log << MSG::INFO << "2D profile histograms in directory \"" << curAlgo << "\" : " << nbNonEmptyEntities[5]
            << curLog[5].str() << endmsg;
      }
      if ( nbNonEmptyEntities[6] > 0 ) {
        log << MSG::INFO << "3D profile histograms in directory \"" << curAlgo << "\" : " << nbNonEmptyEntities[6]
            << curLog[6].str() << endmsg;
      }
    }
  };
  applyToAllSortedEntities(
      [&]( std::string const& algo, std::string const& name, Monitoring::Hub::Entity const& ent ) {
        // Did we change to new component ?
        if ( algo != curAlgo ) {
          dumpAlgoCounters();
          curAlgo            = algo;
          nbNonEmptyEntities = { 0, 0, 0, 0, 0, 0, 0 };
          curLog             = { std::ostringstream{}, std::ostringstream{}, std::ostringstream{}, std::ostringstream{},
                                 std::ostringstream{}, std::ostringstream{}, std::ostringstream{} };
        }
        // first try dedicated binary printers for profile histograms
        auto typeIndex = ent.typeIndex();
        auto binWriter = binRegistry.find( typeIndex );
        if ( binWriter != binRegistry.end() ) {
          auto index = binWriter->second.first;
          ++nbNonEmptyEntities[index];
          curLog[index] << "\n" << binWriter->second.second( name, ent, m_histoStringsWidth );
        } else {
          // use json representation of the entity
          nlohmann::json const j = ent;
          // do we have an histogram ?
          const auto type = j.at( "type" ).get<std::string>();
          if ( type.find( "histogram" ) == 0 ) {
            unsigned int d       = j.at( "dimension" ).get<int>();
            auto         subtype = std::string_view( type ).substr( 10 ); // remove "histogram:" in front
            bool isProfile       = subtype.substr( 0, 15 ) == "WeightedProfile" || subtype.substr( 0, 7 ) == "Profile";
            unsigned int index   = ( isProfile ? 3 : 0 ) + d;
            auto         title   = j.at( "title" ).get<std::string>();
            switch ( d ) {
            case 1:
              curLog[index] << "\n"
                            << Gaudi::Histograming::Sink::printHistogram1D( type, name, title, j, m_histoStringsWidth );
              break;
            case 2:
              curLog[index] << "\n"
                            << Gaudi::Histograming::Sink::printHistogram2D( type, name, title, j, m_histoStringsWidth );
              break;
            case 3:
              curLog[index] << "\n"
                            << Gaudi::Histograming::Sink::printHistogram3D( type, name, title, j, m_histoStringsWidth );
              break;
            }
            ++nbNonEmptyEntities[index];
          } else {
            // regular counter. Is current counter empty ?
            if ( !j.at( "empty" ).template get<bool>() ) {
              ++nbNonEmptyEntities[0];
              curLog[0] << "\n";
              printCounter( curLog[0], name, j );
            }
          }
        }
      } );
  // last component
  dumpAlgoCounters();
}
