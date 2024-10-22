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
#include <TH1F.h>

namespace Gaudi::TestSuite::FileSvc {
  class HistogramWriterAlg : public Gaudi::Algorithm {
  public:
    HistogramWriterAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

    StatusCode initialize() override {
      return Gaudi::Algorithm::initialize().andThen( [this]() {
        m_fileSvc = service<Gaudi::Interfaces::IFileSvc>( "FileSvc" );
        if ( !m_fileSvc ) {
          error() << "Failed to retrieve FileSvc" << endmsg;
          return StatusCode::FAILURE;
        }

        m_file = m_fileSvc->getFile( "Histogram" );
        if ( !m_file ) {
          error() << "Failed to retrieve TFile" << endmsg;
          return StatusCode::FAILURE;
        }

        m_hist = std::make_unique<TH1F>( "FileSvcRandomHist", "Random Numbers", 100, 0, 100 );

        return StatusCode::SUCCESS;
      } );
    }

    StatusCode execute( const EventContext& ) const override {
      m_hist->Fill( rand() % 100 );
      return StatusCode::SUCCESS;
    }

    StatusCode finalize() override {
      m_hist->Write();
      return Gaudi::Algorithm::finalize();
    }

  private:
    Gaudi::Interfaces::IFileSvc* m_fileSvc;
    std::shared_ptr<TFile>       m_file;
    std::unique_ptr<TH1F>        m_hist;
  };

  DECLARE_COMPONENT( HistogramWriterAlg )
} // namespace Gaudi::TestSuite::FileSvc
