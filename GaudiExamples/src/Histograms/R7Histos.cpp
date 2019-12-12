/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/RndmGenerators.h>
#include <vector>

// helper type aliases
using RH1D        = ROOT::Experimental::RH1D;
using RH2D        = ROOT::Experimental::RH2D;
using RAxisConfig = ROOT::Experimental::RAxisConfig;

namespace Gaudi {
  namespace Examples {
    namespace R7 {
      /// Simple timing of ROOT7 histograms filling.
      class HistoTimingAlg : public Gaudi::Algorithm {
      public:
        using Gaudi::Algorithm::Algorithm;
        StatusCode initialize() override {
          // must be called first
          const StatusCode sc = Algorithm::initialize();
          if ( sc.isFailure() ) return sc;

          // random number generator
          auto randSvc = service<IRndmGenSvc>( "RndmGenSvc", true );
          if ( !randSvc || !m_rand.initialize( randSvc, Rndm::Flat( 0., 1. ) ) ) {
            error() << "Unable to create Random generator" << endmsg;
            return StatusCode::FAILURE;
          }

          m_histos.reserve( m_nHistos );
          const RAxisConfig xaxis{100, 0, 1};
          for ( unsigned int iH = 0; iH < m_nHistos; ++iH ) {
            std::ostringstream title;
            title << "Histogram Number " << iH;
            m_histos.emplace_back( title.str(), xaxis );
          }

          return sc;
        }

        StatusCode execute( const EventContext& ) const override {
          for ( unsigned int iT = 0; iT < m_nTracks; ++iT ) {
            for ( auto& h : m_histos ) { h.Fill( m_rand() ); }
          }
          return StatusCode::SUCCESS;
        }

      private:
        mutable Rndm::Numbers m_rand;

        mutable std::vector<RH1D> m_histos;

        Gaudi::Property<unsigned int> m_nHistos{this, "NumHistos", 20, ""};
        Gaudi::Property<unsigned int> m_nTracks{this, "NumTracks", 30, ""};
      };
      DECLARE_COMPONENT( HistoTimingAlg )

      /// Example of algorithm using ROOT7 histograms accumulators.
      class GaudiHistoAlgorithm : public Gaudi::Algorithm {
      public:
        using Gaudi::Algorithm::Algorithm;

        StatusCode execute( const EventContext& ) const override {
          // some random number generators, just to provide numbers
          static Rndm::Numbers Gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
          static Rndm::Numbers Flat( randSvc(), Rndm::Flat( -10.0, 10.0 ) );
          static Rndm::Numbers Expo( randSvc(), Rndm::Exponential( 1.0 ) );
          static Rndm::Numbers Breit( randSvc(), Rndm::BreitWigner( 0.0, 1.0 ) );
          static Rndm::Numbers Poisson( randSvc(), Rndm::Poisson( 2.0 ) );
          static Rndm::Numbers Binom( randSvc(), Rndm::Binomial( 8, 0.25 ) );

          // cache some numbers
          const double gauss( Gauss() );
          const double flat( Flat() );
          const double expo( Expo() );
          const double breit( Breit() );
          const double poisson( Poisson() );
          const double binom( Binom() );

          // =============== Histogramming Examples =================================

          m_h_gauss += gauss;
          m_h_expo += expo;
          m_h_breit += breit;
          m_h_poisson += poisson;
          m_h_binom += binom;

          m_h_gaussVflat += {flat, gauss};
          m_h_expoVflat += {flat, expo};
          m_h_binVpois += {poisson, binom};
          m_h_expoVpois += {poisson, expo};

          if ( m_nCalls.nEntries() == 0 ) always() << "Filling Histograms...... Please be patient !" << endmsg;
          ++m_nCalls;
          return StatusCode::SUCCESS;
        }

      private:
        mutable Gaudi::Accumulators::Counter<> m_nCalls{this, "calls"};

        mutable Gaudi::Accumulators::Histogram<RH1D> m_h_gauss{
            this, "Gaussian mean=0, sigma=1", {"Gaussian mean=0, sigma=1", {100, -5, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH1D> m_h_expo{this, "Exponential", {"Exponential", {100, 0, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH1D> m_h_breit{this, "Breit", {"Breit", {100, -5, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH1D> m_h_poisson{this, "Poisson", {"Poisson", {100, -5, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH1D> m_h_binom{this, "Binominal", {"Binominal", {100, -5, 5}}};

        mutable Gaudi::Accumulators::Histogram<RH2D> m_h_gaussVflat{{"Gaussian V Flat", {50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH2D> m_h_expoVflat{{"Exponential V Flat", {50, -5, 5}, {50, 0, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH2D> m_h_binVpois{{"Binom V poisson", {50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::Histogram<RH2D> m_h_expoVpois{{"Expo V poisson", {50, -5, 5}, {50, -5, 5}}};
      };
      DECLARE_COMPONENT( GaudiHistoAlgorithm )
    } // namespace R7
  }   // namespace Examples
} // namespace Gaudi
