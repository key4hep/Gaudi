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

#include <TFile.h>

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace Gaudi::Histograming::Sink {

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
      setProperty( "TypesToSave", std::vector<std::string>{ "histogram:.*" } )
          .orThrow( "Unable to set typesToSaveProperty", "Histograming::Sink::Base" );
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
