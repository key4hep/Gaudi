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
//====================================================================
//  RootPerfMonSvc implementation
//--------------------------------------------------------------------
//
//  Description: Implementation of the ROOT data storage
//
//  Author     : M.Frank
//
//====================================================================

// Framework include files
#include "RootUtils.h"
#include <GaudiKernel/DataIncident.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/System.h>
#include <RootCnv/RootPerfMonSvc.h>

#include <TBranch.h>
#include <TDirectory.h>
#include <TMap.h>
#include <TObjString.h>
#include <TSystem.h>

using namespace std;
using namespace Gaudi;
typedef const string& CSTR;

#define S_OK StatusCode::SUCCESS
#define S_FAIL StatusCode::FAILURE

// Small routine to issue exceptions
StatusCode RootPerfMonSvc::error( CSTR msg ) {
  if ( m_log ) {
    log() << MSG::ERROR << "Error: " << msg << endmsg;
    return S_FAIL;
  }
  MsgStream m( msgSvc(), name() );
  m << MSG::ERROR << "Error: " << msg << endmsg;
  return S_FAIL;
}

// Initialize the Db data persistency service
StatusCode RootPerfMonSvc::initialize() {
  string     cname;
  StatusCode status = Service::initialize();
  if ( !status.isSuccess() ) return error( "Failed to initialize Service base class." );
  m_log.reset( new MsgStream( msgSvc(), name() ) );
  m_incidentSvc = service( "IncidentSvc" );
  if ( !m_incidentSvc ) return error( "Unable to localize interface from service:IncidentSvc" );

  m_incidentSvc->addListener( this, IncidentType::BeginEvent, 1, false, false );
  m_incidentSvc->addListener( this, "NEW_STREAM", 1, false, false );
  m_incidentSvc->addListener( this, "CONNECTED_OUTPUT", 1, false, false );

  if ( m_ioPerfStats.empty() ) return error( "Performance monitoring file IOPerfStats was not defined." );

  TDirectory::TContext ctxt( nullptr );
  m_perfFile.reset( new TFile( m_ioPerfStats.value().c_str(), "RECREATE" ) );
  if ( !m_perfFile ) return error( "Could not create ROOT file." );

  if ( !( m_perfTree = new TTree( "T", "performance measurement" ) ) ) return error( "Could not create tree." );

  m_perfTree->Branch( "utime", &m_utime, "utime/l" );
  m_perfTree->Branch( "stime", &m_stime, "stime/l" );
  m_perfTree->Branch( "vsize", &m_vsize, "vsize/l" );
  m_perfTree->Branch( "rss", &m_rss, "rss/L" );
  m_perfTree->Branch( "time", &m_time, "time/L" );
  m_eventNumber = 0;
  m_perfTree->Branch( "event_number", &m_eventNumber, "event_number/L" );
  m_perfTree->Branch( "event_type", &m_eventType, "event_type/I" );

  if ( m_setStreams.empty() ) m_setStreams = "undefined";
  if ( m_basketSize.empty() ) m_basketSize = "undefined";
  if ( m_bufferSize.empty() ) m_bufferSize = "undefined";
  if ( m_splitLevel.empty() ) m_splitLevel = "undefined";

  auto map = new TMap();
  map->Add( new TObjString( "streams" ), new TObjString( m_setStreams.value().c_str() ) );
  map->Add( new TObjString( "basket_size" ), new TObjString( m_basketSize.value().c_str() ) );
  map->Add( new TObjString( "buffer_size" ), new TObjString( m_bufferSize.value().c_str() ) );
  map->Add( new TObjString( "split_level" ), new TObjString( m_splitLevel.value().c_str() ) );
  map->Write( "params", TObject::kSingleKey );
  return S_OK;
}

void RootPerfMonSvc::record( EventType eventType ) {
  SysProcStat data;
  m_eventType = eventType;
  m_utime     = data.utime;
  m_stime     = data.stime;
  m_vsize     = data.vsize;
  m_rss       = data.rss;
  m_time      = data.time;
  m_perfTree->Fill();
}

void RootPerfMonSvc::handle( const Incident& incident ) {
  std::string t = incident.type();
  if ( !t.compare( IncidentType::BeginEvent ) ) {
    m_eventNumber++;
    record( EVENT );
    return;
  }
  if ( !t.compare( "CONNECTED_OUTPUT" ) ) { m_outputs.insert( incident.source() ); }
}

// Stop the performance monitoring service
StatusCode RootPerfMonSvc::stop() {
  char text[64];
  record( FSR );
  auto map = new TMap();
  for ( const auto& i : m_outputs ) {
    const char* fn = i.c_str();
    Long_t      id, siz, flags, tim;
    if ( 0 == gSystem->GetPathInfo( fn, &id, &siz, &flags, &tim ) ) {
      ::sprintf( text, "%ld", siz );
      map->Add( new TObjString( fn ), new TObjString( text ) );
    }
  }
  TDirectory::TContext ctxt( m_perfFile.get() );
  map->Write( "Outputs", TObject::kSingleKey );
  return S_OK;
}

// Finalize the performance monitoring service
StatusCode RootPerfMonSvc::finalize() {
  record( FSR );
  log() << MSG::INFO;
  m_log.reset();
  m_incidentSvc.reset();

  m_perfFile->Write();
  m_perfFile->Close();
  m_perfFile.reset();

  return Service::finalize();
}
