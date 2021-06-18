/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Local:
// ============================================================================

#include "JobOptionsSvc.h"

#include "Analyzer.h"
#include "Catalog.h"
#include "Messages.h"
#include "Node.h"
#include "PragmaOptions.h"
#include "PythonConfig.h"
#include "Units.h"
// ============================================================================
// Gaudi:
// ============================================================================
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
#include <Gaudi/Parsers/Factory.h>

#include <algorithm>
#include <cctype>

#if __cplusplus >= 201703
#  include <string_view>
#else
#  include <experimental/string_view>
namespace std {
  using experimental::string_view;
}
#endif

namespace {
#if __cplusplus >= 202000
  inline bool starts_with( std::string_view s, std::string_view prefix ) { return s.starts_with( prefix ); }
#else
  inline bool starts_with( const std::string_view s, const std::string& prefix ) {
    return s.substr( 0, prefix.size() ) == prefix;
  }
#endif

} // namespace

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
    std::vector<std::string_view> unused;
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
    for ( const auto& [key, value] : items() ) { out << key << " = " << value << ";\n"; }
  }
}

void JobOptionsSvc::fillServiceCatalog( const gp::Catalog& catalog ) {
  for ( const auto& client : catalog ) {
    for ( const auto& current : client.second ) {
      set( client.first + '.' + current.NameInClient(), current.ValueAsString() );
    }
  }
}

StatusCode JobOptionsSvc::readOptions( const std::string& file, const std::string& path ) {
  std::string search_path = path;
  if ( search_path.empty() && !m_dir_search_path.empty() ) { search_path = m_dir_search_path; }
  //
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Reading options from the file "
            << "'" << file << "'" << endmsg;
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

  std::tuple<bool, std::string_view> defaultValue{false, ""};
  if ( !has( key ) && !m_globalDefaults.empty() ) { // look for a global default only if it was not set
    std::smatch match;
    for ( const auto& p : m_globalDefaults ) {
      if ( regex_match( key, match, p.first ) ) { defaultValue = {true, p.second}; }
    }
  }

  m_options[key] = *property;

  // at this point the property is bound, so we can set the default if needed
  if ( std::get<0>( defaultValue ) ) set( key, std::string{std::get<1>( defaultValue )} );
}

void JobOptionsSvc::broadcast( const std::regex& filter, const std::string& value, OnlyDefaults defaults_only ) {
  std::smatch match;
  for ( auto& p : m_options ) {
    if ( !defaults_only || !p.second.isSet() ) {
      if ( regex_match( p.first, match, filter ) ) { p.second = value; }
    }
  }
}
