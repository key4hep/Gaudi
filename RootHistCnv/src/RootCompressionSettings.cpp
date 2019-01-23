// STD
#include <stdexcept>

// local
#include "RootCompressionSettings.h"

// ROOT
#include "Compression.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RootCompressionSettings
//
// 2013-10-24 : Chris Jones
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RootHistCnv::RootCompressionSettings::RootCompressionSettings( const std::string& settings )
    : m_level( ROOT::CompressionSettings( ROOT::kZLIB, 1 ) )
{
  const std::string::size_type idx = settings.find( ':' );
  if ( idx != std::string::npos ) {
    // Get compression algorithm type
    const std::string           alg      = settings.substr( 0, idx );
    ROOT::ECompressionAlgorithm alg_code = ROOT::kUseGlobalSetting;
    if ( alg == "ZLIB" ) {
      alg_code = ROOT::kZLIB;
    } else if ( alg == "LZMA" ) {
      alg_code = ROOT::kLZMA;
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
