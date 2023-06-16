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

#include <Gaudi/BaseSink.h>
#include <Gaudi/MonitoringHub.h>
#include <TFile.h>
#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
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

    StatusCode initialize() override {
      return BaseSink::initialize().andThen( [&] {
        // empty output file if it exists, as we will update it at the end
        // This allows multiple Sinks to write to the same ROOT file
        std::filesystem::remove( m_fileName.value() );
        info() << "Writing ROOT histograms to: " << m_fileName.value() << endmsg;
      } );
    }

    StatusCode stop() override {
      // File is updated so that multiple sinks can write to the same file
      // As we are in stop, there is no multithreading so it is safe
      // As we dropped the file at initialization, no old data from a previous
      // run may be mixed with new one
      TFile histoFile( m_fileName.value().c_str(), "UPDATE" );
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
      info() << "Completed update of ROOT histograms in: " << m_fileName.value() << endmsg;
      // call parent's stop
      return Service::stop();
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
