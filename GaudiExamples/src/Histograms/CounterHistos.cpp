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

namespace Gaudi {
  namespace Examples {
    namespace Counter {

      /// Simple timing of counter based histograms filling.
      template <typename Arithmetic, Gaudi::Accumulators::atomicity Atomicity>
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
          Gaudi::Accumulators::Axis<Arithmetic> axis{100, 0, 1};
          for ( unsigned int iH = 0; iH < m_nHistos; ++iH ) {
            std::ostringstream title;
            title << "Histogram Number " << iH;
            m_histos.emplace_back( this, title.str(), axis );
          }

          return sc;
        }

        StatusCode execute( const EventContext& ) const override {
          for ( unsigned int iT = 0; iT < m_nTracks; ++iT ) {
            for ( auto& h : m_histos ) { h += m_rand(); }
          }
          return StatusCode::SUCCESS;
        }

      private:
        mutable Rndm::Numbers m_rand;

        mutable std::vector<Gaudi::Accumulators::HistogramingCounter<1, Atomicity, Arithmetic>> m_histos;

        Gaudi::Property<unsigned int> m_nHistos{this, "NumHistos", 20, ""};
        Gaudi::Property<unsigned int> m_nTracks{this, "NumTracks", 30, ""};
      };
      using HistoTimingAlgDA = HistoTimingAlg<double, Gaudi::Accumulators::atomicity::full>;
      DECLARE_COMPONENT_WITH_ID( HistoTimingAlgDA, "HistoTimingAlgDA" )
      using HistoTimingAlgIA = HistoTimingAlg<unsigned int, Gaudi::Accumulators::atomicity::full>;
      DECLARE_COMPONENT_WITH_ID( HistoTimingAlgIA, "HistoTimingAlgIA" )
      using HistoTimingAlgD = HistoTimingAlg<double, Gaudi::Accumulators::atomicity::none>;
      DECLARE_COMPONENT_WITH_ID( HistoTimingAlgD, "HistoTimingAlgD" )
      using HistoTimingAlgI = HistoTimingAlg<unsigned int, Gaudi::Accumulators::atomicity::none>;
      DECLARE_COMPONENT_WITH_ID( HistoTimingAlgI, "HistoTimingAlgI" )

      /// Example of algorithm using histograms accumulators.
      class GaudiHistoAlgorithm : public Gaudi::Algorithm {
      public:
        using Gaudi::Algorithm::Algorithm;

        StatusCode execute( const EventContext& ) const override {
          // some random number generators, just to provide numbers
          static Rndm::Numbers Gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
          static Rndm::Numbers Flat( randSvc(), Rndm::Flat( -10.0, 10.0 ) );

          // cache some numbers
          const double gauss( Gauss() );
          const double gauss2( Gauss() );
          const double flat( Flat() );
          const double flat2( Flat() );

          // updating histograms
          m_gauss += gauss;
          m_gaussVflat += {flat, gauss};
          m_gaussVflatVgauss += {flat, gauss, gauss2};
          m_gauss_noato += gauss;
          m_gaussVflat_noato += {flat, gauss};
          m_gaussVflatVgauss_noato += {flat, gauss, gauss2};
          m_gauss_int += (int)gauss;
          m_gaussVflat_int += {(int)flat, (int)gauss};
          m_gaussVflatVgauss_int += {(int)flat, (int)gauss, (int)gauss2};
          m_gauss_w += {gauss, .5};
          m_gaussVflat_w += {{flat, gauss}, .5};
          m_gaussVflatVgauss_w += {{flat, gauss, gauss2}, .5};
          auto gauss_buf      = m_gauss_buf.buffer();
          auto gaussVflat_buf = m_gaussVflat_buf.buffer();
          auto gaussVflatVgauss_buf = m_gaussVflatVgauss_buf.buffer();
          for ( unsigned int i = 0; i < 10; i++ ) {
            gauss_buf += gauss;
            gaussVflat_buf += {flat, gauss};
            gaussVflatVgauss_buf += {flat, gauss, gauss2};
          }

          // updating profile histograms
          m_prof_gauss += {gauss, flat};
          m_prof_gaussVflat += {flat, gauss, flat};
          m_prof_gaussVflatVgauss += {flat, gauss, gauss2, flat2};
          m_prof_gauss_noato += {gauss, flat};
          m_prof_gaussVflat_noato += {flat, gauss, flat2};
          m_prof_gaussVflatVgauss_noato += {flat, gauss, gauss2, flat2};
          m_prof_gauss_int += {(int)gauss, (int)flat};
          m_prof_gaussVflat_int += {(int)flat, (int)gauss, (int)flat2};
          m_prof_gaussVflatVgauss_int += {(int)flat, (int)gauss, (int)gauss2, (int)flat2};
          m_prof_gauss_w += {{gauss, flat}, .5};
          m_prof_gaussVflat_w += {{flat, gauss, flat2}, .5};
          m_prof_gaussVflatVgauss_w += {{flat, gauss, gauss2, flat2}, .5};
          auto prof_gauss_buf      = m_prof_gauss_buf.buffer();
          auto prof_gaussVflat_buf = m_prof_gaussVflat_buf.buffer();
          auto prof_gaussVflatVgauss_buf = m_prof_gaussVflatVgauss_buf.buffer();
          for ( unsigned int i = 0; i < 10; i++ ) {
            prof_gauss_buf += {gauss, flat};
            prof_gaussVflat_buf += {flat, gauss, flat2};
            prof_gaussVflatVgauss_buf += {flat, gauss, gauss2, flat2};
          }

          if ( m_nCalls.nEntries() == 0 ) always() << "Filling Histograms...... Please be patient !" << endmsg;
          ++m_nCalls;
          return StatusCode::SUCCESS;
        }

      private:
        mutable Gaudi::Accumulators::Counter<> m_nCalls{this, "calls"};

        // Testing regular histograms in all dimensions and features

        // "default" case, that is bins containing doubles and atomic
        mutable Gaudi::Accumulators::HistogramingCounter<1> m_gauss{
                                                                    this, "Gaussian mean=0, sigma=1, atomic", {100, -5, 5}};
        mutable Gaudi::Accumulators::HistogramingCounter<2> m_gaussVflat{
                                                                         this, "Gaussian V Flat, atomic", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::HistogramingCounter<3> m_gaussVflatVgauss{
                                                                               this, "Gaussian V Flat V Gaussian, atomic", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // non atomic versions
        mutable Gaudi::Accumulators::HistogramingCounter<1, Gaudi::Accumulators::atomicity::none> m_gauss_noato{
                                                                                                                this, "Gaussian mean=0, sigma=1, non atomic", {100, -5, 5}};
        mutable Gaudi::Accumulators::HistogramingCounter<2, Gaudi::Accumulators::atomicity::none> m_gaussVflat_noato{
                                                                                                                     this, "Gaussian V Flat, non atomic", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::HistogramingCounter<3, Gaudi::Accumulators::atomicity::none> m_gaussVflatVgauss_noato{
                                                                                                                           this, "Gaussian V Flat V Gaussian, non atomic", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // using integers
        mutable Gaudi::Accumulators::HistogramingCounter<1, Gaudi::Accumulators::atomicity::full, int>
        m_gauss_int{this, "Gaussian mean=0, sigma=1, integer values", {10, -5, 5}};
        mutable Gaudi::Accumulators::HistogramingCounter<2, Gaudi::Accumulators::atomicity::full, int>
        m_gaussVflat_int{this, "Gaussian V Flat, integer values", {{10, -5, 5}, {10, -5, 5}}};
        mutable Gaudi::Accumulators::HistogramingCounter<3, Gaudi::Accumulators::atomicity::full, int>
        m_gaussVflatVgauss_int{this, "Gaussian V Flat V Gaussian, interger values", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // weighted version, "default" case
        mutable Gaudi::Accumulators::WeightedHistogramingCounter<1> m_gauss_w{
                                                                      this, "Gaussian mean=0, sigma=1, weighted", {100, -5, 5}};
        mutable Gaudi::Accumulators::WeightedHistogramingCounter<2> m_gaussVflat_w{
                                                                           this, "Gaussian V Flat, weighted", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::WeightedHistogramingCounter<3> m_gaussVflatVgauss_w{
                                                                                 this, "Gaussian V Flat V Gaussian, weighted", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // "default" case, dedicated to testing buffers
        mutable Gaudi::Accumulators::HistogramingCounter<1> m_gauss_buf{
                                                                        this, "Gaussian mean=0, sigma=1, buffered", {100, -5, 5}};
        mutable Gaudi::Accumulators::HistogramingCounter<2> m_gaussVflat_buf{
                                                                             this, "Gaussian V Flat, buffered", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::HistogramingCounter<3> m_gaussVflatVgauss_buf{
                                                                                   this, "Gaussian V Flat V Gaussian, buffered", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // Testing profiling histograms in all dimensions and features

        // "default" case, that is bins containing doubles and atomic
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<1> m_prof_gauss{
            this, "Gaussian mean=0, sigma=1, atomic", {100, -5, 5}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<2> m_prof_gaussVflat{
                                                                                     this, "Gaussian V Flat, atomic", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<3> m_prof_gaussVflatVgauss{
                                                                                           this, "Gaussian V Flat V Gaussian, atomic", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // non atomic versions
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<1, Gaudi::Accumulators::atomicity::none> m_prof_gauss_noato{
            this, "Gaussian mean=0, sigma=1, non atomic", {100, -5, 5}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<2, Gaudi::Accumulators::atomicity::none> m_prof_gaussVflat_noato{
                                                                                                                                 this, "Gaussian V Flat, non atomic", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<3, Gaudi::Accumulators::atomicity::none> m_prof_gaussVflatVgauss_noato{
                                                                                                                                       this, "Gaussian V Flat V Gaussian, non atomic", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // using integers internally
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<1, Gaudi::Accumulators::atomicity::full, int>
            m_prof_gauss_int{this, "Gaussian mean=0, sigma=1, integer values", {10, -5, 5}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<2, Gaudi::Accumulators::atomicity::full, int>
        m_prof_gaussVflat_int{this, "Gaussian V Flat, integer values", {{10, -5, 5}, {10, -5, 5}}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<3, Gaudi::Accumulators::atomicity::full, int>
        m_prof_gaussVflatVgauss_int{this, "Gaussian V Flat V Gaussian, interger values", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // weighted version, "default" case
        mutable Gaudi::Accumulators::WeightedProfileHistogramingCounter<1> m_prof_gauss_w{
            this, "Gaussian mean=0, sigma=1, weighted", {100, -5, 5}};
        mutable Gaudi::Accumulators::WeightedProfileHistogramingCounter<2> m_prof_gaussVflat_w{
                                                                                       this, "Gaussian V Flat, weighted", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::WeightedProfileHistogramingCounter<3> m_prof_gaussVflatVgauss_w{
                                                                                                     this, "Gaussian V Flat V Gaussian, weighted", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};

        // "default" case, dedicated to testing buffers
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<1> m_prof_gauss_buf{
            this, "Gaussian mean=0, sigma=1, buffered", {100, -5, 5}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<2> m_prof_gaussVflat_buf{
                                                                                         this, "Gaussian V Flat, buffered", {{50, -5, 5}, {50, -5, 5}}};
        mutable Gaudi::Accumulators::ProfileHistogramingCounter<3> m_prof_gaussVflatVgauss_buf{
                                                                                               this, "Gaussian V Flat V Gaussian, buffered", {{10, -5, 5}, {10, -5, 5}, {10, -5, 5}}};
        
      };
      DECLARE_COMPONENT( GaudiHistoAlgorithm )
    } // namespace Counter
  }   // namespace Examples
} // namespace Gaudi
