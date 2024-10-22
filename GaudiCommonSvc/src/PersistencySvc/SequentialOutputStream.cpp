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
// standard headers
#include <limits>

// boost
#include <boost/filesystem.hpp>

// Framework include files
#include "SequentialOutputStream.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/DataStoreItem.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/MsgStream.h>

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( SequentialOutputStream )

using namespace std;
namespace bf = boost::filesystem;

//=============================================================================
StatusCode SequentialOutputStream::writeObjects() {
  try {
    makeFilename();
  } catch ( const GaudiException& except ) {
    error() << except.message() << endmsg;
    return StatusCode::FAILURE;
  }
  return OutputStream::writeObjects();
}

//=============================================================================
StatusCode SequentialOutputStream::execute() {
  // Clear any previously existing item list
  clearSelection();
  // Test whether this event should be output
  if ( isEventAccepted() ) {
    StatusCode sc = writeObjects();
    clearSelection();
    ++m_events;
    return sc;
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
void SequentialOutputStream::makeFilename() {
  if ( m_events % m_eventsPerFile != 0 ) return;

  bf::path outputPath( m_outputName.value() );
  string   filename  = outputPath.filename().string();
  bf::path dir       = outputPath.parent_path();
  string   stem      = outputPath.stem().string();
  string   extension = outputPath.extension().string();

  if ( !dir.empty() ) {
    if ( !bf::exists( dir ) ) {
      stringstream stream;
      stream << "Directory " << dir << " does not exist.";
      throw GaudiException( stream.str(), "error", StatusCode::FAILURE );
    }
  }

  if ( m_numericFilename ) {
    if ( m_events == 0 ) {
      try {
        m_iFile = std::stoul( stem );
      } catch ( const std::invalid_argument& /* cast */ ) {
        string msg = "Filename " + filename + " is not a number, which was needed.";
        throw GaudiException( msg, "error", StatusCode::FAILURE );
      }
    }
    string       iFile  = std::to_string( m_iFile );
    unsigned int length = 0;

    if ( stem.length() > iFile.length() ) { length = stem.length() - iFile.length(); }

    stringstream name;
    if ( !dir.empty() ) name << dir << "/";
    for ( unsigned int i = 0; i < length; ++i ) { name << "0"; }
    name << iFile << extension;
    m_outputName = name.str();
  } else {
    if ( m_iFile != 1 ) {
      size_t pos = stem.rfind( "_" );
      stem       = stem.substr( 0, pos );
    }

    string iFile = std::to_string( m_iFile );

    unsigned int length = 0;
    if ( m_nNumbersAdded > iFile.length() ) { length = m_nNumbersAdded - iFile.length(); }

    stringstream name;
    name << dir << "/" << stem;
    for ( unsigned int i = 0; i < length; ++i ) {
      if ( i == 0 ) name << "_";
      name << "0";
    }
    name << iFile << extension;
    m_outputName = name.str();
  }
  ++m_iFile;
}
