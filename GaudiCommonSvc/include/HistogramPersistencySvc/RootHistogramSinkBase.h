/***********************************************************************************\
* (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/BaseSink.h>
#include <Gaudi/MonitoringHub.h>

#include <functional>

#include <TDirectory.h>
#include <TFile.h>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split_when.hpp>
#include <range/v3/view/transform.hpp>
// upstream has renamed namespace ranges::view -> ranges::views
#if RANGE_V3_VERSION < 900
namespace ranges::views {
  using namespace ranges::view;
}
#endif

#include <nlohmann/json.hpp>

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace Gaudi::Histograming::Sink {

  namespace details {

    struct Axis {
      unsigned int nBins;
      double       minValue;
      double       maxValue;
      std::string  title;
    };

    Axis toAxis( nlohmann::json& jAxis ) {
      return { jAxis.at( "nBins" ).get<unsigned int>(), jAxis.at( "minValue" ).get<double>(),
               jAxis.at( "maxValue" ).get<double>(),
               ";" + jAxis.at( "title" ).get<std::string>() }; // ";" to prepare concatenations of titles
    }

    /**
     * generic function to handle histograms - internal implemenatation
     */
    template <typename Traits, std::size_t... index>
    void saveRootHistoInternal( TFile& file, std::string dir, std::string name, nlohmann::json const& j,
                                std::index_sequence<index...> ) {
      // extract data from json
      auto jsonAxis = j.at( "axis" );
      auto axis     = std::array{ toAxis( jsonAxis[index] )... };
      auto weights  = j.at( "bins" ).get<std::vector<typename Traits::WeightType>>();
      auto title    = j.at( "title" ).get<std::string>();
      auto nentries = j.at( "nEntries" ).get<unsigned int>();
      // weird way ROOT has to give titles to axis
      title += ( axis[index].title + ... );
      // compute total number of bins, multiplying bins per axis
      auto totNBins = ( ( axis[index].nBins + 2 ) * ... );
      assert( weights.size() == totNBins );

      if ( name[0] == '/' ) {
        dir  = "";
        name = name.substr( 1 );
      }

      // take into account the case where name contains '/'s (e.g. "Group/Name") by
      // moving the prefix into dir
      if ( auto pos = name.rfind( '/' ); pos != std::string::npos ) {
        dir += '/' + name.substr( 0, pos );
        name = name.substr( pos + 1 );
      }

      // remember the current directory
      auto previousDir = gDirectory;

      // find or create the directory for the histogram
      {
        using namespace ranges;
        auto is_delimiter        = []( auto c ) { return c == '/' || c == '.'; };
        auto transform_to_string = views::transform( []( auto&& rng ) { return rng | to<std::string>; } );

        auto currentDir = accumulate( dir | views::split_when( is_delimiter ) | transform_to_string,
                                      file.GetDirectory( "" ), []( auto current, auto&& dir_level ) {
                                        if ( current ) {
                                          // try to get next level
                                          auto nextDir = current->GetDirectory( dir_level.c_str() );
                                          // if it does not exist, create it
                                          if ( !nextDir ) nextDir = current->mkdir( dir_level.c_str() );
                                          // move to next level
                                          current = nextDir;
                                        }
                                        return current;
                                      } );

        if ( !currentDir )
          throw GaudiException( "Could not create directory " + dir, "Histogram::Sink::Root", StatusCode::FAILURE );
        // switch to the directory
        currentDir->cd();
      }

      // Create Root histogram calling constructors with the args tuple
      auto histo = Traits::create( name, title, axis[index]... );

      // fill Histo
      for ( unsigned int i = 0; i < totNBins; i++ ) Traits::fill( histo, i, weights[i] );
      // fill histo metadata, e.g. bins and number of entries
      Traits::fillMetaData( histo, jsonAxis, nentries );
      // write to file
      histo.Write();

      // switch back to the previous directory
      previousDir->cd();
    }

    /**
     * generic method to save histograms to files
     *
     * Can be used in most cases as the handler function to register into Sink::Base
     * contains all the boiler plate code and redirects specific code to the Traits template
     *
     * The Traits type must provide the following static functions :
     *   - Histo create( std::string& name, std::string& title, Axis& axis... )
     *       it should intanciate a new ROOT histogram instance and return it
     *   - void fillMetaData( Histo& histo, nlohmann::json const& jsonAxis, unsigned int nentries)
     *       it should fill metadata in the ROOT histo histogram provides from the list of axis number of entries
     *   - void fill( Histo& histo, unsigned int i, const WeightType& weight )
     *       it should fill the given bin with the given value
     * Last point, it should have a static constexpr unsigned int Dimension
     */
    template <typename Traits>
    void saveRootHisto( TFile& file, std::string dir, std::string name, nlohmann::json& j ) {
      details::saveRootHistoInternal<Traits>( file, std::move( dir ), std::move( name ), j,
                                              std::make_index_sequence<Traits::Dimension>() );
    }
  } // namespace details

  /*
   * a Base class for Root related Sinks dealing with Histograms.
   *
   * provides the common method plus a generic way of registering handler for different types
   */
  class Base : public Monitoring::BaseSink {
  public:
    using HistoIdentification = std::pair<std::string, int>;
    using HistoHandler        = std::function<void( TFile& file, std::string, std::string, nlohmann::json& )>;
    using HistoRegistry       = std::map<HistoIdentification, HistoHandler>;

    Base( std::string name, ISvcLocator* svcloc ) : Monitoring::BaseSink( name, svcloc ) {
      // only deal with histograms
      setProperty( "TypesToSave", std::vector<std::string>{ "histogram:.*" } );
    }

    StatusCode stop() override {
      return Service::stop().andThen( [&] {
        TFile histoFile( m_fileName.value().c_str(), "RECREATE" );
        applytoAllEntities(
            [&histoFile, this]( auto& ent ) {
              auto j    = ent.toJSON();
              auto dim  = j.at( "dimension" ).template get<unsigned int>();
              auto type = j.at( "type" ).template get<std::string>();
              // cut type after last ':' if there is one. The rest is precision parameter that we do not need here
              // as ROOT anyway treats everything as doubles in histograms
              type       = type.substr( 0, type.find_last_of( ':' ) );
              auto saver = m_registry.find( { type, dim } );
              if ( saver != m_registry.end() ) ( saver->second )( histoFile, ent.component, ent.name, j );
            },
            true );
        return StatusCode::SUCCESS;
      } );
    }

    void registerHandler( HistoIdentification const& id, HistoHandler const& func ) {
      m_registry.emplace( std::piecewise_construct, std::make_tuple( id ), std::make_tuple( func ) );
    }

  private:
    /// map of supported type and the way to handle them
    HistoRegistry m_registry{};

    Gaudi::Property<std::string> m_fileName{ this, "FileName", "testHisto.root",
                                             "Name of file where to save histograms" };
  };

} // namespace Gaudi::Histograming::Sink
