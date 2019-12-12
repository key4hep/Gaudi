#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/RndmGenerators.h>
#include <boost/histogram.hpp>
#include <vector>

// helper type aliases
using BH1D = decltype( boost::histogram::make_histogram( boost::histogram::axis::regular<>{10, 0., 10.} ) );
using BH2D = decltype( boost::histogram::make_histogram( boost::histogram::axis::regular<>{10, 0., 10.},
                                                         boost::histogram::axis::regular<>{10, 0., 10.} ) );

namespace Gaudi {
  namespace Examples {
    namespace Boost {
      /// Simple timing of Boost.Histograms filling.
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
          for ( unsigned int iH = 0; iH < m_nHistos; ++iH ) {
            m_histos.emplace_back( boost::histogram::make_histogram( boost::histogram::axis::regular<>{100, 0., 1.} ) );
          }

          return sc;
        }

        StatusCode execute( const EventContext& ) const override {
          for ( unsigned int iT = 0; iT < m_nTracks; ++iT ) {
            for ( auto& h : m_histos ) { h( m_rand() ); }
          }
          return StatusCode::SUCCESS;
        }

      private:
        mutable Rndm::Numbers m_rand;

        mutable std::vector<BH1D> m_histos;

        Gaudi::Property<unsigned int> m_nHistos{this, "NumHistos", 20, ""};
        Gaudi::Property<unsigned int> m_nTracks{this, "NumTracks", 30, ""};
      };
      DECLARE_COMPONENT( HistoTimingAlg )

      /// Example of algorithm using Boost.Histograms accumulators.
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

          m_h_gauss( gauss );
          m_h_expo( expo );
          m_h_breit( breit );
          m_h_poisson( poisson );
          m_h_binom( binom );

          m_h_gaussVflat( flat, gauss );
          m_h_expoVflat( flat, expo );
          m_h_binVpois( poisson, binom );
          m_h_expoVpois( poisson, expo );

          if ( m_nCalls.nEntries() == 0 ) always() << "Filling Histograms...... Please be patient !" << endmsg;
          ++m_nCalls;
          return StatusCode::SUCCESS;
        }

      private:
        mutable Gaudi::Accumulators::Counter<> m_nCalls{this, "calls"};

        mutable Gaudi::Accumulators::Histogram<BH1D> m_h_gauss{
            this, "Gaussian mean=0, sigma=1",
            boost::histogram::make_histogram( boost::histogram::axis::regular<>{100, -5, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH1D> m_h_expo{
            this, "Exponential", boost::histogram::make_histogram( boost::histogram::axis::regular<>{100, 0, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH1D> m_h_breit{
            this, "Breit", boost::histogram::make_histogram( boost::histogram::axis::regular<>{100, -5, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH1D> m_h_poisson{
            this, "Poisson", boost::histogram::make_histogram( boost::histogram::axis::regular<>{100, -5, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH1D> m_h_binom{
            this, "Binominal", boost::histogram::make_histogram( boost::histogram::axis::regular<>{100, -5, 5} )};

        mutable Gaudi::Accumulators::Histogram<BH2D> m_h_gaussVflat{
            this, "Gaussian V Flat",
            boost::histogram::make_histogram( boost::histogram::axis::regular<>{50, -5, 5},
                                              boost::histogram::axis::regular<>{50, -5, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH2D> m_h_expoVflat{
            this, "Exponential V Flat",
            boost::histogram::make_histogram( boost::histogram::axis::regular<>{50, -5, 5},
                                              boost::histogram::axis::regular<>{50, 0, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH2D> m_h_binVpois{
            this, "Binom V poisson",
            boost::histogram::make_histogram( boost::histogram::axis::regular<>{50, -5, 5},
                                              boost::histogram::axis::regular<>{50, -5, 5} )};
        mutable Gaudi::Accumulators::Histogram<BH2D> m_h_expoVpois{
            this, "Expo V poisson",
            boost::histogram::make_histogram( boost::histogram::axis::regular<>{50, -5, 5},
                                              boost::histogram::axis::regular<>{50, -5, 5} )};
      };
      DECLARE_COMPONENT( GaudiHistoAlgorithm )
    } // namespace Boost
  }   // namespace Examples
} // namespace Gaudi
