/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Interfaces/IFileSvc.h>
#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/StatusCode.h>
#include <TFile.h>
#include <boost/algorithm/string.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class FileSvc : public extends<Service, Gaudi::Interfaces::IFileSvc> {
public:
  // Constructor
  FileSvc( const std::string& name, ISvcLocator* svc );

  virtual StatusCode initialize() override;

  virtual StatusCode finalize() override;

  /** Get a TFile pointer based on an identifier.

   * @param identifier A string identifying the file (2 identifiers can point to the same TFile).
   * @return An shared pointer to a TFile object corresponding to the specified identifier.
   */
  virtual std::shared_ptr<TFile> getFile( const std::string& identifier ) override;

public:
  // Property to map file identifiers to file paths
  Gaudi::Property<std::map<std::string, std::string>> m_config{
      this, "Config", {}, "Map of keywords to file paths for file access" };

private:
  /** Open a file based on a specified path and opening mode.
   *
   * @param filePath The file path.
   * @param option The file opening options to pass to root ("NEW/CREATE", "RECREATE", "UPDATE").
   * @param compress The compression level and algorithm to use for the file.
   * @return A shared_ptr to a newly opened TFile object, or null if the operation fails.
   */
  std::shared_ptr<TFile> openFile( const std::string& filePath, const std::string& option, int compress );

  /** Close all files.
   *
   * @return StatusCode indicating the success or failure of the operation.
   */
  StatusCode closeFiles();

private:
  // Map holding file identifiers to file indices in the m_files vector
  std::unordered_map<std::string, size_t> m_identifiers;

  // Vector to hold all files unique pointers
  std::vector<std::shared_ptr<TFile>> m_files;
};

DECLARE_COMPONENT( FileSvc )

namespace {
  /**
   * Helper funciton to parse the file path to extract parameters for file handling.
   *
   * @param path The full file path potentially containing query-like parameters.
   * @return Map of extracted parameters.
   */
  std::map<std::string, std::string> parseFilePath( const std::string& path ) {
    std::vector<std::string> parts;
    boost::split( parts, path, boost::is_any_of( "?" ) );
    std::map<std::string, std::string> result;
    if ( parts.size() > 1 ) {
      std::vector<std::string> params;
      boost::split( params, parts[1], boost::is_any_of( "&" ) );
      for ( auto& param : params ) {
        std::vector<std::string> kv;
        boost::split( kv, param, boost::is_any_of( "=" ) );
        if ( kv.size() == 2 ) { result[boost::trim_copy( kv[0] )] = boost::trim_copy( kv[1] ); }
      }
    }
    result["mode"] = ( result.find( "mode" ) == result.end() ) ? "CREATE" : result["mode"];
    result["path"] = boost::trim_copy( parts[0] );
    return result;
  }

  /**
   * Helper function to check the consistency of a file configuration map.
   * Specifically, this checks if a file has been configured multiple times
   * with different options.
   *
   * @param configMap The configuration map to be checked.
   * @param svc The FileSvc instance calling this function.
   * @return StatusCode indicating the success or failure of the operation.
   */
  StatusCode checkConfig( const std::map<std::string, std::string>& configMap, const FileSvc& svc ) {
    std::map<std::string, std::map<std::string, std::string>> fileParams;
    for ( const auto& [identifier, path] : configMap ) {
      auto params = parseFilePath( path );

      auto& existingParams = fileParams[params["path"]];
      if ( !existingParams.empty() ) {
        if ( existingParams != params ) {
          svc.error() << "Conflicting configurations for file path: " << params["path"] << endmsg;
          return StatusCode::FAILURE;
        }
      } else {
        existingParams = std::move( params );
      }
    }
    return StatusCode::SUCCESS;
  }

  /**
   * Helper function to find a file's index in the files vector.
   *
   * @param files The vector holding all file pointers.
   * @param filePath The path of the file to look for.
   * @return AN optional index of the file if it exists
   */
  std::optional<size_t> findFileIndex( const std::vector<std::shared_ptr<TFile>>& files, const std::string& filePath ) {
    auto it = std::find_if( files.begin(), files.end(),
                            [&filePath]( const auto& file ) { return file && file->GetName() == filePath; } );
    if ( it != files.end() ) { return std::distance( files.begin(), it ); }
    return std::nullopt;
  }
} // namespace

FileSvc::FileSvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc ) {}

StatusCode FileSvc::initialize() {
  return Service::initialize().andThen( [this]() {
    if ( checkConfig( m_config, *this ).isFailure() ) { return StatusCode::FAILURE; }

    for ( const auto& [identifier, path] : m_config ) {
      auto params = parseFilePath( path );

      // Check if this file is already opened and mapped
      if ( auto fileIndex = findFileIndex( m_files, params["path"] ) ) {
        // File already opened, just map the identifier to the existing index
        m_identifiers[boost::to_lower_copy( identifier )] = *fileIndex;
      } else {
        // File not found, open a new one
        int compress = ( params.find( "compress" ) != params.end() )
                           ? std::stoi( params["compress"] )
                           : ROOT::RCompressionSetting::EDefaults::kUseCompiledDefault;
        if ( auto file = openFile( path, params["mode"], compress ) ) {
          m_files.push_back( std::move( file ) );
          m_identifiers[boost::to_lower_copy( identifier )] = m_files.size() - 1;
        } else {
          error() << "Failed to open file: " << params["path"] << endmsg;
          return StatusCode::FAILURE;
        }
      }
    }

    return StatusCode::SUCCESS;
  } );
}

StatusCode FileSvc::finalize() { return closeFiles(); }

std::shared_ptr<TFile> FileSvc::getFile( const std::string& identifier ) {
  auto it = m_identifiers.find( boost::to_lower_copy( identifier ) );
  if ( it != m_identifiers.end() && it->second < m_files.size() ) { return m_files[it->second]; }
  error() << "No file associated with identifier: " << identifier << endmsg;
  return nullptr;
}

std::shared_ptr<TFile> FileSvc::openFile( const std::string& filePath, const std::string& option, int compress ) {
  auto file = std::make_shared<TFile>( filePath.c_str(), option.c_str(), "", compress );
  if ( !file || file->IsZombie() ) { return nullptr; }
  return file;
}

StatusCode FileSvc::closeFiles() {
  for ( const auto& file : m_files ) {
    if ( file ) { file->Close(); }
  }
  m_files.clear();
  m_identifiers.clear();
  return StatusCode::SUCCESS;
}
