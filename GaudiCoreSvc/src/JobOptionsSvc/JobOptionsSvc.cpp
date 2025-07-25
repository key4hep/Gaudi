/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Analyzer.h"
#include "Catalog.h"
#include "Messages.h"
#include "Node.h"
#include "PragmaOptions.h"
#include "PropertyId.h"
#include "PythonConfig.h"
#include "Units.h"
#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <Gaudi/Parsers/Factory.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/PathResolver.h>
#include <GaudiKernel/PropertyHolder.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/System.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Gaudi {
  namespace Parsers {
    class Catalog;
  }
} // namespace Gaudi

class JobOptionsSvc : public extends<Service, Gaudi::Interfaces::IOptionsSvc> {
public:
  typedef std::vector<const Gaudi::Details::PropertyBase*> PropertiesT;

private:
  using PropertyId  = Gaudi::Details::PropertyId;
  using StorageType = std::unordered_map<PropertyId, Gaudi::Details::WeakPropertyRef>;

  StorageType m_options;

  mutable std::map<std::string, std::unique_ptr<Gaudi::Details::PropertyBase>> m_old_iface_compat;
  mutable std::map<std::string, PropertiesT>                                   m_old_iface_compat_2;

protected:
  /// @{
  void        set( const std::string& key, const std::string& value ) override { m_options[key] = value; }
  std::string get( const std::string& key, const std::string& default_ = {} ) const override {
    auto item = m_options.find( key );
    return item != m_options.end() ? std::string{ item->second } : default_;
  }
  std::string pop( const std::string& key, const std::string& default_ = {} ) override {
    std::string result = default_;

    auto item = m_options.find( key );
    if ( item != m_options.end() ) {
      result = std::move( item->second );
      m_options.erase( item );
    }
    return result;
  }
  bool has( const std::string& key ) const override { return m_options.find( key ) != m_options.end(); }
  std::vector<std::tuple<std::string, std::string>> items() const override {
    std::vector<std::tuple<std::string, std::string>> v;
    v.reserve( m_options.size() );
    std::for_each( begin( m_options ), end( m_options ), [&v]( const auto& item ) { v.emplace_back( item ); } );
    std::sort( begin( v ), end( v ) );
    return v;
  }
  bool isSet( const std::string& key ) const override {
    auto item = m_options.find( key );
    return item != m_options.end() && item->second.isSet();
  }

  void bind( const std::string& prefix, Gaudi::Details::PropertyBase* property ) override;

  void broadcast( const std::regex& filter, const std::string& value, OnlyDefaults defaults_only ) override;
  /// @}

public:
  // Constructor
  JobOptionsSvc( const std::string& name, ISvcLocator* svc );

  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode stop() override;

  /** look for file 'file' into search path 'path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param file file   name
   *  @param path search path
   *  @return status code
   */
  StatusCode readOptions( std::string_view file, std::string_view path = "" ) override;

private:
  void fillServiceCatalog( const Gaudi::Parsers::Catalog& catalog );

  /// dump properties catalog to file
  void dump( const std::string& file, const Gaudi::Parsers::Catalog& catalog ) const;
  void dump( const std::string& file ) const;

private:
  Gaudi::Property<std::string> m_source_type{ this, "TYPE" };
  Gaudi::Property<std::string> m_source_path{ this, "PATH" };
  Gaudi::Property<std::string> m_dir_search_path{ this, "SEARCHPATH" };
  Gaudi::Property<std::string> m_dump{ this, "DUMPFILE" };
  Gaudi::Property<std::string> m_pythonAction{ this, "PYTHONACTION" };
  Gaudi::Property<std::string> m_pythonParams{ this, "PYTHONPARAMS" };

  Gaudi::Property<std::vector<std::pair<std::string, std::string>>> m_globalDefaultsProp{
      this,
      "GlobalDefaults",
      {},
      "Allow definition of global defaults for properties as list of pairs (regex, value)" };

  Gaudi::Property<bool> m_reportUnused{ this, "ReportUnused", false, "Print report of properties set, but not used" };

  std::vector<std::pair<std::regex, std::string>> m_globalDefaults;
};

// ============================================================================
DECLARE_COMPONENT( JobOptionsSvc )
// ============================================================================
// Namespace aliases:
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
JobOptionsSvc::JobOptionsSvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc ) {
  if ( System::isEnvSet( "JOBOPTSEARCHPATH" ) ) m_dir_search_path = System::getEnv( "JOBOPTSEARCHPATH" );
  if ( System::isEnvSet( "JOBOPTSDUMPFILE" ) ) m_dump = System::getEnv( "JOBOPTSDUMPFILE" );

  m_globalDefaultsProp.declareUpdateHandler( [this]( Gaudi::Details::PropertyBase& ) {
    m_globalDefaults.clear();
    m_globalDefaults.reserve( m_globalDefaultsProp.size() );
    for ( const auto& p : m_globalDefaultsProp ) { m_globalDefaults.emplace_back( p.first, p.second ); }
  } );
}
// ============================================================================
StatusCode JobOptionsSvc::initialize() {
  // Call base class initializer
  StatusCode sc = Service::initialize();
  // Read the job options if needed
  if ( sc ) {
    if ( m_source_type == "NONE" ) {
      return sc;
    } else if ( m_source_type == "PYTHON" ) {
      PythonConfig conf( this );
      return conf.evaluateConfig( m_source_path, m_pythonParams, m_pythonAction );
    } else {
      return readOptions( m_source_path, m_dir_search_path );
    }
  }
  return sc;
}
StatusCode JobOptionsSvc::stop() {
  if ( m_reportUnused ) {
    std::vector<std::string> unused;
    unused.reserve( m_options.size() );

    for ( const auto& p : m_options ) {
      if ( !p.second.isBound() ) unused.emplace_back( p.first );
    }

    if ( !unused.empty() ) {
      std::sort( unused.begin(), unused.end() );
      auto& log = warning();
      log << unused.size() << " unused properties:";
      for ( const auto& k : unused ) log << "\n - " << k;
      log << endmsg;
    }
  }
  return Service::stop();
}

// ============================================================================
StatusCode JobOptionsSvc::start() {
  if ( !m_dump.empty() ) { dump( m_dump ); }
  return StatusCode::SUCCESS;
}

void JobOptionsSvc::dump( const std::string& file, const gp::Catalog& catalog ) const {
  std::ofstream out( file, std::ios_base::out | std::ios_base::trunc );
  if ( !out ) {
    error() << "Unable to open dump-file \"" + file + "\"" << endmsg;
    return; // RETURN
  }
  info() << "Properties are dumped into \"" + file + "\"" << endmsg;
  // perform the actual dump:
  out << catalog;
}

void JobOptionsSvc::dump( const std::string& file ) const {
  std::ofstream out( file, std::ios_base::out | std::ios_base::trunc );
  if ( !out ) {
    error() << "Unable to open dump-file \"" + file + "\"" << endmsg;
  } else {
    info() << "Properties are dumped into \"" + file + "\"" << endmsg;
    for ( const auto& [key, value] : items() ) {
      out << key << " = " << value << ';';
      if ( !m_options.find( key )->second.isBound() ) out << " // unused";
      out << '\n';
    }
  }
}

void JobOptionsSvc::fillServiceCatalog( const gp::Catalog& catalog ) {
  for ( const auto& client : catalog ) {
    for ( const auto& current : client.second ) {
      set( client.first + '.' + current.NameInClient(), current.ValueAsString() );
    }
  }
}

StatusCode JobOptionsSvc::readOptions( std::string_view file, std::string_view path ) {
  std::string search_path = std::string{ path };
  if ( search_path.empty() && !m_dir_search_path.empty() ) { search_path = m_dir_search_path; }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Reading options from the file "
            << "'" << file << "'" << endmsg;

  if ( file.size() >= 5 && file.substr( file.size() - 5 ) == ".json" ) {
    nlohmann::json opts;
    std::ifstream  input( System::PathResolver::find_file( std::string( file ), std::string( path ) ) );
    input >> opts;
    for ( auto item = opts.begin(); item != opts.end(); ++item ) { set( item.key(), item.value().get<std::string>() ); }
    return StatusCode::SUCCESS;
  }

  gp::Messages      messages( msgStream() );
  gp::Catalog       catalog;
  gp::Units         units;
  gp::PragmaOptions pragma;
  gp::Node          ast;
  StatusCode        sc = gp::ReadOptions( file, path, &messages, &catalog, &units, &pragma, &ast ) ? StatusCode::SUCCESS
                                                                                                   : StatusCode::FAILURE;

  // --------------------------------------------------------------------------
  if ( sc.isSuccess() ) {
    if ( pragma.IsPrintOptions() ) { info() << "Print options" << std::endl << catalog << endmsg; }
    if ( pragma.IsPrintTree() ) { info() << "Print tree:" << std::endl << ast.ToString() << endmsg; }
    if ( pragma.HasDumpFile() ) dump( pragma.dumpFile(), catalog );
    info() << "Job options successfully read in from " << file << endmsg;
    fillServiceCatalog( catalog );
  } else {
    fatal() << "Job options errors." << endmsg;
  }
  // ----------------------------------------------------------------------------
  return sc;
}

void JobOptionsSvc::bind( const std::string& prefix, Gaudi::Details::PropertyBase* property ) {
  const std::string key = prefix + '.' + property->name();

  std::tuple<bool, std::string_view> defaultValue{ false, "" };
  if ( !has( key ) && !m_globalDefaults.empty() ) { // look for a global default only if it was not set
    std::smatch match;
    for ( const auto& p : m_globalDefaults ) {
      if ( regex_match( key, match, p.first ) ) { defaultValue = { true, p.second }; }
    }
  }

  m_options[key] = *property;

  // at this point the property is bound, so we can set the default if needed
  if ( std::get<0>( defaultValue ) ) set( key, std::string{ std::get<1>( defaultValue ) } );
}

void JobOptionsSvc::broadcast( const std::regex& filter, const std::string& value, OnlyDefaults defaults_only ) {
  std::smatch match;
  for ( auto& p : m_options ) {
    if ( !defaults_only || !p.second.isSet() ) {
      const auto s = p.first.str();
      if ( regex_match( s, match, filter ) ) { p.second = value; }
    }
  }
}
