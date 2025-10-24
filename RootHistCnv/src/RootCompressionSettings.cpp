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
// STD
#include <stdexcept>

// local
#include "RootCompressionSettings.h"

// ROOT
#include <Compression.h>

//-----------------------------------------------------------------------------
// Implementation file for class : RootCompressionSettings
//
// 2013-10-24 : Chris Jones
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RootHistCnv::RootCompressionSettings::RootCompressionSettings( const std::string& settings )
    : m_level( ROOT::CompressionSettings( ROOT::RCompressionSetting::EAlgorithm::kZLIB, 1 ) ) {
  const std::string::size_type idx = settings.find( ':' );
  if ( idx != std::string::npos ) {
    // Get compression algorithm type
    const std::string                              alg      = settings.substr( 0, idx );
    ROOT::RCompressionSetting::EAlgorithm::EValues alg_code = ROOT::RCompressionSetting::EAlgorithm::kUseGlobal;
    if ( alg == "ZLIB" ) {
      alg_code = ROOT::RCompressionSetting::EAlgorithm::kZLIB;
    } else if ( alg == "LZMA" ) {
      alg_code = ROOT::RCompressionSetting::EAlgorithm::kLZMA;
    } else if ( alg == "LZ4" ) {
      alg_code = ROOT::RCompressionSetting::EAlgorithm::kLZ4;
    } else if ( alg == "ZSTD" ) {
      alg_code = ROOT::RCompressionSetting::EAlgorithm::kZSTD;
    } else {
      throw std::runtime_error( "ERROR: Unknown ROOT compression algorithm:" + alg );
    }

    // get compression level
    const std::string slev = settings.substr( idx + 1 );
    const int         ilev = std::stoi( slev );

    // set the level
    m_level = ROOT::CompressionSettings( alg_code, ilev );
  }
}

//=============================================================================
