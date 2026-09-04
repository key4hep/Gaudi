/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Interfaces/IFileSvc.h>
#include <Gaudi/details/BranchReadHelper.h>
#include <TFile.h>
#include <TTree.h>
#include <format>
#include <mutex>
#include <string>
#include <unordered_map>

namespace {
  // Extract the name from a path in the TES string by returning the last part after a slash
  auto getNameFromLoc( std::string_view loc ) {
    auto lastSlashPos = loc.find_last_of( '/' );
    return std::string{ lastSlashPos != loc.npos ? loc.substr( lastSlashPos + 1 ) : loc };
  }
} // namespace

namespace Gaudi::NTuple {

  class GenericReader : public Gaudi::Algorithm {
  public:
    using Gaudi::Algorithm::Algorithm;

    StatusCode initialize() override {
      const auto& extraOutputs = extraOutputDeps();
      if ( extraOutputs.empty() ) {
        error() << "No extra output locations specified. Please define extra output for the NTuple reader" << endmsg;
        return StatusCode::FAILURE;
      }

      auto fileSvc = service<Gaudi::Interfaces::IFileSvc>( "FileSvc" );
      if ( !fileSvc ) {
        error() << "Failed to retrieve FileSvc" << endmsg;
        return StatusCode::FAILURE;
      }

      auto file = fileSvc->getFile( m_fileId );
      if ( !file ) {
        error() << "Failed to retrieve TFile with identifier " << m_fileId.value() << endmsg;
        return StatusCode::FAILURE;
      }

      m_tree = file->Get<TTree>( m_ntupleTname.value().c_str() );
      if ( !m_tree ) {
        error() << "Failed to retrieve TTree " << m_ntupleTname.value() << endmsg;
        return StatusCode::FAILURE;
      }

      return connectBranches( extraOutputs );
    }

    StatusCode execute( const EventContext& ctx ) const override {
      std::scoped_lock lock( m_mutex );
      m_tree->GetEntry( ctx.evt() );
      for ( auto& [address, helper] : m_branchReadHelpers ) {
        eventSvc()
            ->registerObject( address, helper.adoptValue().release() )
            .orThrow( std::format( "Failed to register object to location '{}'", address ), name() );
      }
      return StatusCode::SUCCESS;
    }

    StatusCode finalize() override { return Gaudi::Algorithm::finalize(); }

  private:
    Gaudi::Property<std::string> m_fileId{ this, "InputFile", "NTuple",
                                           "Identifier for the TFile to read from" }; // Property to hold the the
                                                                                      // identifier of where the
                                                                                      // TTree will be saved
    Gaudi::Property<std::string> m_ntupleTname{ this, "NTupleName", "", "Name of the TTree" };
    TTree*                       m_tree{ nullptr };
    mutable std::unordered_map<std::string, Gaudi::details::BranchReadHelper> m_branchReadHelpers;
    mutable std::mutex                                                        m_mutex;

    StatusCode connectBranches( const DataObjIDColl& extraOutputs ) {
      m_branchReadHelpers.clear();
      for ( const auto& dep : extraOutputs ) {
        auto branchName = getNameFromLoc( dep.key() );
        auto branch     = m_tree->GetBranch( branchName.c_str() );
        if ( !branch ) {
          error() << "Failed to retrieve TBranch " << branchName << endmsg;
          return StatusCode::FAILURE;
        }
        m_branchReadHelpers.emplace( dep.key(), branch );
      }

      return StatusCode::SUCCESS;
    }
  };

  DECLARE_COMPONENT( GenericReader )

} // namespace Gaudi::NTuple
