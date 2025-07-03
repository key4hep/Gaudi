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
#include <Gaudi/Property.h>
#include <GaudiKernel/JobHistory.h>
#include <GaudiKernel/System.h>
#include <iostream>

using std::endl;
using std::ostream;
using std::string;

JobHistory::JobHistory() : m_start_time( 0 ) {

  time( &m_start_time );

  std::string rel;

  if ( ( rel = System::getEnv( "ATLAS_BASE_RELEASE" ) ) != "UNKNOWN" ) {
    m_release_version = rel;
  } else if ( ( rel = System::getEnv( "GAUDIROOT" ) ) != "UNKNOWN" ) {
    m_release_version = rel;
  } else {
    m_release_version = "UNKNOWN";
  }
  m_dir       = System::getEnv( "PWD" );
  m_cmtconfig = System::getEnv( "CMTCONFIG" );

  m_osname     = System::osName();
  m_hostname   = System::hostName();
  m_os_version = System::osVersion();
  m_machine    = System::machineType();

  m_environ = System::getEnv();
}

JobHistory::JobHistory( const std::string& rel, const std::string& os, const std::string& host, const std::string& dir,
                        const std::string& osver, const std::string& mach, const std::string& cmtconfig,
                        const time_t& time )
    : m_release_version( rel )
    , m_dir( dir )
    , m_cmtconfig( cmtconfig )
    , m_osname( os )
    , m_hostname( host )
    , m_os_version( osver )
    , m_machine( mach )
    , m_start_time( time ) {}

JobHistory::~JobHistory() = default;

const CLID& JobHistory::classID() {

  static const CLID CLID_JobHistory = 247994533;
  return CLID_JobHistory;
}

void JobHistory::addProperty( const std::string& key, const std::string& value ) {
  /// \fixme strip optional quotes around the string
  std::string prop_value = value;
  if ( !prop_value.empty() && prop_value[0] == '"' && prop_value[prop_value.size() - 1] == '"' )
    prop_value = prop_value.substr( 1, prop_value.size() - 2 );

  std::string client, prop_name;
  auto        pos = key.rfind( '.' );
  if ( pos == std::string::npos ) {
    prop_name = key;
  } else {
    client    = key.substr( 0, pos );
    prop_name = key.substr( pos + 1 );
  }

  m_ppl.emplace_back( client, std::make_unique<Gaudi::Property<std::string>>( prop_name, prop_value ) );
}

std::ostream& JobHistory::dump( std::ostream& ost, const bool isXML, int /*ind*/ ) const {

  if ( !isXML ) {
    ost << "Release: " << release_version() << endl;
    ost << "OS:      " << os() << endl;
    ost << "OS ver:  " << os_version() << endl;
    ost << "Host:    " << hostname() << endl;
    ost << "Machine: " << machine() << endl;
    ost << "Run dir: " << dir() << endl;
    ost << "CMTCONFIG: " << cmtconfig() << endl;
    ost << "Job start time: " << start_time() << endl << endl;
    ost << "Properties: [" << endl;
    ;
    for ( const auto& ipprop : propertyPairs() ) {
      const auto& client = ipprop.first;
      const auto& prop   = ipprop.second;
      ost << client << ":  ";
      prop->fillStream( ost );
      ost << endl;
    }
    ost << "]" << endl;
    for ( const auto& itr : environment() ) ost << itr << endl;
  } else {
  }

  return ost;
}
