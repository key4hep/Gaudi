/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
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

// here we have to include the IJobOptionsSvc deprecated header, so we silence the warning
#define GAUDI_INTERNAL_NO_IJOBOPTIONSSVC_H_DEPRECATION 1

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

  void make_lower( std::string& s ) {
    std::transform( s.begin(), s.end(), s.begin(), []( unsigned char c ) { return std::tolower( c ); } );
  }
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

// ============================================================================
StatusCode JobOptionsSvc::addPropertyToCatalogue( const std::string&                  client,
                                                  const Gaudi::Details::PropertyBase& property ) {
  if ( property.type_info() == &typeid( std::string ) ) {
    // relatively convoluted way to strip unneeded quotes.
    Gaudi::Property<std::string> tmp;
    tmp.assign( property );
    set( client + '.' + property.name(), tmp.value() );
  } else {
    set( client + '.' + property.name(), property.toString() );
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
StatusCode JobOptionsSvc::removePropertyFromCatalogue( const std::string& client, const std::string& name ) {
  pop( client + '.' + name );
  return StatusCode::SUCCESS;
}
// ============================================================================
const JobOptionsSvc::PropertiesT* JobOptionsSvc::getProperties( const std::string& client ) const {
  PropertiesT props;

  const std::string key_base      = client + '.';
  const auto        key_base_size = key_base.size();
  for ( const auto& elem : m_options ) {
    std::string_view key = elem.first;
    // for keys that are 'client.name' (and name does not contain '.')
    // we add an entry to the vector
    if ( starts_with( key, key_base ) ) {
      std::string_view name = key.substr( key_base_size );
      if ( name.find( '.' ) == std::string::npos ) {
        props.push_back( getClientProperty( client, static_cast<std::string>( name ) ) );
      }
    }
  }
  // we are supposed to be the owner of the vector, so we need to keep it safe
  return &( m_old_iface_compat_2[client] = std::move( props ) );
}
// ============================================================================
StatusCode JobOptionsSvc::setMyProperties( const std::string& client, IProperty* myInt ) {
  const std::string key_base      = client + '.';
  const auto        key_base_size = key_base.size();

  bool fail = false;
  for ( const auto& elem : m_options ) {
    std::string_view key = elem.first;
    if ( starts_with( key, key_base ) ) {
      const auto name = static_cast<std::string>( key.substr( key_base_size ) );
      // \fixme this has to change if we want nested properties
      // if ( myInt->hasProperty( name ) ) {
      if ( name.find( '.' ) == std::string::npos ) {
        if ( !myInt->setPropertyRepr( name, elem.second ) ) {
          error() << "Unable to set the property '" << name << "'"
                  << " of '" << client << "'. "
                  << "Check option and algorithm names, type and bounds." << endmsg;
          fail = true;
          // throw std::invalid_argument( "cannot set " + name + " of " + client + " to " + elem.second );
        }
      }
    }
  }
  return fail ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

JobOptionsSvc::StorageType::const_iterator JobOptionsSvc::i_find( const std::string& key, bool warn ) const {
  StorageType::const_iterator iter = m_options.find( key );
  if ( iter == m_options.end() ) { // try case insensitive lookup
    std::string l_key = key;
    make_lower( l_key );

    auto l_iter = m_lower_to_correct_case.find( l_key );
    if ( l_iter != m_lower_to_correct_case.end() ) iter = m_options.find( l_iter->second );
    if ( warn && iter != m_options.end() ) {
      warning() << "mismatching case for property name: actual name is " << key << " but " << iter->first
                << " is in the option files" << endmsg;
    }
  }
  return iter;
}

void JobOptionsSvc::i_update_case_insensitive_map( const std::string& key ) {
  std::string l_key = key;
  make_lower( l_key );
  m_lower_to_correct_case[l_key] = key;
}

/// Get the list of clients
std::vector<std::string> JobOptionsSvc::getClients() const {
  std::set<std::string> clients;
  for ( const auto& elem : m_options ) {
    const auto pos = elem.first.rfind( '.' );
    if ( pos != std::string::npos ) clients.emplace( elem.first, 0, pos );
  }
  return {begin( clients ), end( clients )};
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
  const auto key = prefix + '.' + property->name();

  std::tuple<bool, std::string_view> defaultValue{false, ""};
  if ( !has( key ) && !m_globalDefaults.empty() ) { // look for a global default only if it was not set
    std::smatch match;
    for ( const auto& p : m_globalDefaults ) {
      if ( regex_match( key, match, p.first ) ) { defaultValue = {true, p.second}; }
    }
  }

  auto item = i_find( key, true );
  if ( item != m_options.end() && key != item->first ) {
    // Storage is case insensitive, and I want to use the what the property dictates
    auto new_item = m_options.emplace( key, std::move( m_options.find( item->first )->second ) );
    m_options.erase( item );
    item = new_item.first;
  }
  if ( item == m_options.end() ) {
    m_options.emplace( key, *property );
  } else {
    m_options.find( key )->second = *property;
  }

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
