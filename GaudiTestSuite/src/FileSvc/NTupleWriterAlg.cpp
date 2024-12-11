/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Interfaces/IFileSvc.h>
#include <TTree.h>

namespace Gaudi::TestSuite::FileSvc {
  class NTupleWriterAlg : public Gaudi::Algorithm {
  public:
    NTupleWriterAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

    StatusCode initialize() override {
      return Gaudi::Algorithm::initialize().andThen( [this]() {
        m_fileSvc = service<Gaudi::Interfaces::IFileSvc>( "FileSvc" );
        if ( !m_fileSvc ) {
          error() << "Failed to retrieve FileSvc" << endmsg;
          return StatusCode::FAILURE;
        }

        m_file = m_fileSvc->getFile( "Ntuple" );
        if ( !m_file ) {
          error() << "Failed to retrieve TFile" << endmsg;
          return StatusCode::FAILURE;
        }

        m_tree = std::make_unique<TTree>( "FileSvcNTuple", "An example NTuple" ).release();
        m_tree->Branch( "value", &m_value );

        return StatusCode::SUCCESS;
      } );
    }

    StatusCode execute( const EventContext& ) const override {
      m_tree->Fill();
      return StatusCode::SUCCESS;
    }

    StatusCode finalize() override {
      m_tree->Write();
      return Gaudi::Algorithm::finalize();
    }

  private:
    Gaudi::Interfaces::IFileSvc* m_fileSvc{ nullptr };
    std::shared_ptr<TFile>       m_file;
    TTree*                       m_tree{ nullptr };
    int                          m_value{ 9 };
  };

  DECLARE_COMPONENT( NTupleWriterAlg )
} // namespace Gaudi::TestSuite::FileSvc
