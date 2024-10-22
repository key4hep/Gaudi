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
#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Accumulators/RootHistogram.h>
#include <Gaudi/Algorithm.h>
#include <Gaudi/FSMCallbackHolder.h>
#include <GaudiKernel/RndmGenerators.h>

#include "../../../GaudiKernel/tests/src/LogHistogram.h"

#include <deque>

namespace Gaudi {
  namespace TestSuite {
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

          Gaudi::Accumulators::Axis<Arithmetic> axis{ 100, 0, 1 };
          for ( unsigned int iH = 0; iH < m_nHistos; ++iH ) {
            std::ostringstream title;
            title << "Histogram Number " << iH;
            m_histos.emplace_back( this, title.str(), title.str(), axis );
          }

          return sc;
        }

        StatusCode execute( const EventContext& ) const override {
          for ( unsigned int iT = 0; iT < m_nTracks; ++iT ) {
            for ( auto& h : m_histos ) { ++h[m_rand()]; }
          }
          return StatusCode::SUCCESS;
        }

      private:
        mutable Rndm::Numbers m_rand;

        mutable std::deque<Gaudi::Accumulators::StaticHistogram<1, Atomicity, Arithmetic>> m_histos;

        Gaudi::Property<unsigned int> m_nHistos{ this, "NumHistos", 20, "" };
        Gaudi::Property<unsigned int> m_nTracks{ this, "NumTracks", 30, "" };
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
      class GaudiHistoAlgorithm : public FSMCallbackHolder<Gaudi::Algorithm> {
      public:
        using Base = FSMCallbackHolder<Gaudi::Algorithm>;
        using Base::Base;

        StatusCode initialize() override {
          return Base::initialize().andThen( [&] { m_custom_gauss_noinit.createHistogram( *this ); } );
        }

        StatusCode execute( const EventContext& ) const override {
          // some random number generators, just to provide numbers
          static Rndm::Numbers Gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
          static Rndm::Numbers Flat( randSvc(), Rndm::Flat( -10.0, 10.0 ) );
          static Rndm::Numbers Gauss3( randSvc(), Rndm::Gauss( 5.0, 1.0 ) );

          // cache some numbers
          const double gauss( Gauss() );
          const double gauss2( Gauss() );
          const double flat( Flat() );
          const double gauss3( Gauss3() );

          // updating histograms
          ++m_gauss[gauss];
          ++m_gaussAbsName[gauss];
          ++m_gaussVflat[{ flat, gauss }];
          ++m_gaussVflatVgauss[{ flat, gauss, gauss2 }];
          ++m_gauss_noato[gauss];
          ++m_gaussVflat_noato[{ flat, gauss }];
          ++m_gaussVflatVgauss_noato[{ flat, gauss, gauss2 }];
          ++m_gauss_int[(int)gauss];
          ++m_gaussVflat_int[{ (int)flat, (int)gauss }];
          ++m_gaussVflatVgauss_int[{ (int)flat, (int)gauss, (int)gauss2 }];

          // weighted cases
          m_gauss_w[gauss] += .5;
          m_gaussVflat_w[{ flat, gauss }] += .5;
          m_gaussVflatVgauss_w[{ flat, gauss, gauss2 }] += .5;

          // using buffers
          auto gauss_buf            = m_gauss_buf.buffer();
          auto gaussVflat_buf       = m_gaussVflat_buf.buffer();
          auto gaussVflatVgauss_buf = m_gaussVflatVgauss_buf.buffer();
          for ( unsigned int i = 0; i < 10; i++ ) {
            ++gauss_buf[gauss];
            ++gaussVflat_buf[{ flat, gauss }];
            ++gaussVflatVgauss_buf[{ flat, gauss, gauss2 }];
          }

          // updating profile histograms
          m_prof_gauss[gauss] += gauss3;
          m_prof_gaussVflat[{ flat, gauss }] += gauss3;
          m_prof_gaussVflatVgauss[{ flat, gauss, gauss2 }] += gauss3;
          m_prof_gauss_noato[gauss] += gauss3;
          m_prof_gaussVflat_noato[{ flat, gauss }] += gauss3;
          m_prof_gaussVflatVgauss_noato[{ flat, gauss, gauss2 }] += gauss3;
          m_prof_gauss_int[(int)gauss] += (int)gauss3;
          m_prof_gaussVflat_int[{ (int)flat, (int)gauss }] += (int)gauss3;
          m_prof_gaussVflatVgauss_int[{ (int)flat, (int)gauss, (int)gauss2 }] += (int)gauss3;

          // weighted profile histograms
          m_prof_gauss_w[gauss] += { gauss3, .5 };
          m_prof_gaussVflat_w[{ flat, gauss }] += { gauss3, .5 };
          m_prof_gaussVflatVgauss_w[{ flat, gauss, gauss2 }] += { gauss3, .5 };

          // using buffers on profile histograms
          auto prof_gauss_buf            = m_prof_gauss_buf.buffer();
          auto prof_gaussVflat_buf       = m_prof_gaussVflat_buf.buffer();
          auto prof_gaussVflatVgauss_buf = m_prof_gaussVflatVgauss_buf.buffer();
          for ( unsigned int i = 0; i < 10; i++ ) {
            prof_gauss_buf[gauss] += gauss3;
            prof_gaussVflat_buf[{ flat, gauss }] += gauss3;
            prof_gaussVflatVgauss_buf[{ flat, gauss, gauss2 }] += gauss3;
          }

          // using CaloHistogram
          m_log_gauss[gauss]++;
          m_log_gaussVflat[{ flat, gauss }]++;

          // using Custom histograms, with title and axis defined in python
          ++m_custom_gauss_noprop[gauss];
          ++m_custom_gauss[gauss];
          ++m_custom_gaussVflat[{ flat, gauss }];
          ++m_custom_gaussVflatVgauss[{ flat, gauss, gauss2 }];
          m_custom_gauss_w[gauss] += .5;
          m_custom_gaussVflat_w[{ flat, gauss }] += .5;
          m_custom_gaussVflatVgauss_w[{ flat, gauss, gauss2 }] += .5;
          m_custom_prof_gauss[gauss] += gauss3;
          m_custom_prof_gaussVflat[{ flat, gauss }] += gauss3;
          m_custom_prof_gaussVflatVgauss[{ flat, gauss, gauss2 }] += gauss3;
          m_custom_prof_gauss_w[gauss] += { gauss3, .5 };
          m_custom_prof_gaussVflat_w[{ flat, gauss }] += { gauss3, .5 };
          m_custom_prof_gaussVflatVgauss_w[{ flat, gauss, gauss2 }] += { gauss3, .5 };
          ++m_custom_gauss_noinit[gauss];

          if ( m_nCalls.nEntries() == 0 ) always() << "Filling Histograms...... Please be patient !" << endmsg;
          ++m_nCalls;
          return StatusCode::SUCCESS;
        }

      private:
        mutable Gaudi::Accumulators::Counter<> m_nCalls{ this, "calls" };

        // Testing regular histograms in all dimensions and features

        // "default" case, that is bins containing doubles and atomic
        mutable Gaudi::Accumulators::StaticHistogram<1> m_gauss{
            this, "Gauss", "Gaussian mean=0, sigma=1, atomic", { 100, -5, 5, "X" } };
        mutable Gaudi::Accumulators::StaticHistogram<2> m_gaussVflat{
            this, "GaussFlat", "Gaussian V Flat, atomic", { { 50, -5, 5, "X" }, { 50, -5, 5, "Y" } } };
        mutable Gaudi::Accumulators::StaticHistogram<3> m_gaussVflatVgauss{
            this,
            "GaussFlatGauss",
            "Gaussian V Flat V Gaussian, atomic",
            { { 10, -5, 5, "X" }, { 10, -5, 5, "Y" }, { 10, -5, 5, "Z" } } };

        // non atomic versions
        mutable Gaudi::Accumulators::StaticHistogram<1, Gaudi::Accumulators::atomicity::none> m_gauss_noato{
            this, "GaussNA", "Gaussian mean=0, sigma=1, non atomic", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticHistogram<2, Gaudi::Accumulators::atomicity::none> m_gaussVflat_noato{
            this, "GaussFlatNA", "Gaussian V Flat, non atomic", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticHistogram<3, Gaudi::Accumulators::atomicity::none> m_gaussVflatVgauss_noato{
            this,
            "GaussFlatGaussNA",
            "Gaussian V Flat V Gaussian, non atomic",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // using integers
        mutable Gaudi::Accumulators::StaticHistogram<1, Gaudi::Accumulators::atomicity::full, int> m_gauss_int{
            this, "GaussInt", "Gaussian mean=0, sigma=1, integer values", { 10, -5, 5 } };
        mutable Gaudi::Accumulators::StaticHistogram<2, Gaudi::Accumulators::atomicity::full, int> m_gaussVflat_int{
            this, "GaussFlatInt", "Gaussian V Flat, integer values", { { 10, -5, 5 }, { 10, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticHistogram<3, Gaudi::Accumulators::atomicity::full, int>
            m_gaussVflatVgauss_int{ this,
                                    "GaussFlatGaussInt",
                                    "Gaussian V Flat V Gaussian, interger values",
                                    { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // weighted version, "default" case
        mutable Gaudi::Accumulators::StaticWeightedHistogram<1> m_gauss_w{
            this, "GaussW", "Gaussian mean=0, sigma=1, weighted", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticWeightedHistogram<2> m_gaussVflat_w{
            this, "GaussFlatW", "Gaussian V Flat, weighted", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticWeightedHistogram<3> m_gaussVflatVgauss_w{
            this,
            "GaussFlatGaussW",
            "Gaussian V Flat V Gaussian, weighted",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // "default" case, dedicated to testing buffers
        mutable Gaudi::Accumulators::StaticHistogram<1> m_gauss_buf{
            this, "GaussBuf", "Gaussian mean=0, sigma=1, buffered", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticHistogram<2> m_gaussVflat_buf{
            this, "GaussFlatBuf", "Gaussian V Flat, buffered", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticHistogram<3> m_gaussVflatVgauss_buf{
            this,
            "GaussFlatGaussBuf",
            "Gaussian V Flat V Gaussian, buffered",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // Testing profiling histograms in all dimensions and features

        // "default" case, that is bins containing doubles and atomic
        mutable Gaudi::Accumulators::StaticProfileHistogram<1> m_prof_gauss{
            this, "ProfGauss", "Profile, Gaussian mean=0, sigma=1, atomic", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<2> m_prof_gaussVflat{
            this, "ProfGaussFlat", "Profile, Gaussian V Flat, atomic", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<3> m_prof_gaussVflatVgauss{
            this,
            "ProfGaussFlatGauss",
            "Profile, Gaussian V Flat V Gaussian, atomic",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // non atomic versions
        mutable Gaudi::Accumulators::StaticProfileHistogram<1, Gaudi::Accumulators::atomicity::none> m_prof_gauss_noato{
            this, "ProfGaussNA", "Profile, Gaussian mean=0, sigma=1, non atomic", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<2, Gaudi::Accumulators::atomicity::none>
            m_prof_gaussVflat_noato{
                this, "ProfGaussFlatNA", "Profile, Gaussian V Flat, non atomic", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<3, Gaudi::Accumulators::atomicity::none>
            m_prof_gaussVflatVgauss_noato{ this,
                                           "ProfGaussFlatGaussNA",
                                           "Profile, Gaussian V Flat V Gaussian, non atomic",
                                           { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // using integers internally
        mutable Gaudi::Accumulators::StaticProfileHistogram<1, Gaudi::Accumulators::atomicity::full, int>
            m_prof_gauss_int{
                this, "ProfGaussInt", "Profile, Gaussian mean=0, sigma=1, integer values", { 10, -5, 5 } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<2, Gaudi::Accumulators::atomicity::full, int>
            m_prof_gaussVflat_int{ this,
                                   "ProfGaussFlatInt",
                                   "Profile, Gaussian V Flat, integer values",
                                   { { 10, -5, 5 }, { 10, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<3, Gaudi::Accumulators::atomicity::full, int>
            m_prof_gaussVflatVgauss_int{ this,
                                         "ProfGaussFlatGaussInt",
                                         "Profile, Gaussian V Flat V Gaussian, interger values",
                                         { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // weighted version, "default" case
        mutable Gaudi::Accumulators::StaticWeightedProfileHistogram<1> m_prof_gauss_w{
            this, "ProfGaussW", "Profile, Gaussian mean=0, sigma=1, weighted", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticWeightedProfileHistogram<2> m_prof_gaussVflat_w{
            this, "ProfGaussFlatW", "Profile, Gaussian V Flat, weighted", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticWeightedProfileHistogram<3> m_prof_gaussVflatVgauss_w{
            this,
            "ProfGaussFlatGaussW",
            "Profile, Gaussian V Flat V Gaussian, weighted",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // "default" case, dedicated to testing buffers
        mutable Gaudi::Accumulators::StaticProfileHistogram<1> m_prof_gauss_buf{
            this, "ProfGaussBuf", "Profile, Gaussian mean=0, sigma=1, buffered", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<2> m_prof_gaussVflat_buf{
            this, "ProfGaussFlatBuf", "Profile, Gaussian V Flat, buffered", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticProfileHistogram<3> m_prof_gaussVflatVgauss_buf{
            this,
            "ProfGaussFlatGaussBuf",
            "Profile, Gaussian V Flat V Gaussian, buffered",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // Custom histogram, with log or log log behavior
        mutable Accumulators::LogHistogram<1> m_log_gauss{ this, "LogGauss", "Log, Gaussian", { 5, 0, 2 } };
        mutable Accumulators::LogHistogram<2, Accumulators::atomicity::full, float> m_log_gaussVflat{
            this, "LogGaussFlat", "LogLog, Gaussian V Flat", { { 5, 0, 2 }, { 5, 0, 2 } } };

        // Histogram in an absolute location
        mutable Gaudi::Accumulators::StaticHistogram<1> m_gaussAbsName{
            this, "/TopDir/SubDir/Gauss", "Gaussian mean=0, sigma=1, atomic", { 100, -5, 5, "X" } };

        // Custom histograms, specifying title and axis in python
        mutable Gaudi::Accumulators::Histogram<1> m_custom_gauss_noprop{
            this, "CustomGaussNoProp", "Gaussian mean=0, sigma=1, atomic", { 100, -5, 5, "X" } };
        mutable Gaudi::Accumulators::Histogram<1> m_custom_gauss{ this, "CustomGauss" };
        mutable Gaudi::Accumulators::Histogram<2> m_custom_gaussVflat{ this, "CustomGaussFlat" };
        mutable Gaudi::Accumulators::Histogram<3> m_custom_gaussVflatVgauss{ this, "CustomGaussFlatGauss" };

        mutable Gaudi::Accumulators::WeightedHistogram<1> m_custom_gauss_w{ this, "CustomGaussW" };
        mutable Gaudi::Accumulators::WeightedHistogram<2> m_custom_gaussVflat_w{ this, "CustomGaussFlatW" };
        mutable Gaudi::Accumulators::WeightedHistogram<3> m_custom_gaussVflatVgauss_w{ this, "CustomGaussFlatGaussW" };

        mutable Gaudi::Accumulators::ProfileHistogram<1> m_custom_prof_gauss{ this, "CustomProfGauss" };
        mutable Gaudi::Accumulators::ProfileHistogram<2> m_custom_prof_gaussVflat{ this, "CustomProfGaussFlat" };
        mutable Gaudi::Accumulators::ProfileHistogram<3> m_custom_prof_gaussVflatVgauss{ this,
                                                                                         "CustomProfGaussFlatGauss" };

        mutable Gaudi::Accumulators::WeightedProfileHistogram<1> m_custom_prof_gauss_w{ this, "CustomProfGaussW" };
        mutable Gaudi::Accumulators::WeightedProfileHistogram<2> m_custom_prof_gaussVflat_w{ this,
                                                                                             "CustomProfGaussFlatW" };
        mutable Gaudi::Accumulators::WeightedProfileHistogram<3> m_custom_prof_gaussVflatVgauss_w{
            this, "CustomProfGaussFlatGaussW" };

        // Checking DoNotInitialize property of CustomHistogram
        mutable Gaudi::Accumulators::Histogram<1> m_custom_gauss_noinit{ this, "CustomGaussNoInit", "", {}, true };
      };
      DECLARE_COMPONENT( GaudiHistoAlgorithm )

      /// Example of algorithm using root histograms accumulators.
      class GaudiRootHistoAlgorithm : public FSMCallbackHolder<Gaudi::Algorithm> {
      public:
        using Base = FSMCallbackHolder<Gaudi::Algorithm>;
        using Base::Base;

        StatusCode execute( const EventContext& ) const override {
          // some random number generators, just to provide numbers
          static Rndm::Numbers Gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
          static Rndm::Numbers Flat( randSvc(), Rndm::Flat( -10.0, 10.0 ) );

          // cache some numbers
          const double gauss( Gauss() );
          const double gauss2( Gauss() );
          const double flat( Flat() );

          // updating histograms
          ++m_gauss[gauss];
          ++m_gaussVflat[{ flat, gauss }];
          ++m_gaussVflatVgauss[{ flat, gauss, gauss2 }];

          // using buffers
          auto gauss_buf            = m_gauss_buf.buffer();
          auto gaussVflat_buf       = m_gaussVflat_buf.buffer();
          auto gaussVflatVgauss_buf = m_gaussVflatVgauss_buf.buffer();
          for ( unsigned int i = 0; i < 10; i++ ) {
            ++gauss_buf[gauss];
            ++gaussVflat_buf[{ flat, gauss }];
            ++gaussVflatVgauss_buf[{ flat, gauss, gauss2 }];
          }

          // custom histograms, with title and axis defined in python
          ++m_custom_gauss[gauss];
          ++m_custom_gaussVflat[{ flat, gauss }];
          ++m_custom_gaussVflatVgauss[{ flat, gauss, gauss2 }];

          if ( m_nCalls.nEntries() == 0 ) always() << "Filling Histograms...... Please be patient !" << endmsg;
          ++m_nCalls;
          return StatusCode::SUCCESS;
        }

      private:
        mutable Gaudi::Accumulators::Counter<> m_nCalls{ this, "calls" };

        // Testing Root histograms in all dimensions

        // "default" case, that is bins containing doubles and atomic
        mutable Gaudi::Accumulators::StaticRootHistogram<1> m_gauss{
            this, "Gauss", "Gaussian mean=0, sigma=1, atomic", { 100, -5, 5, "X" } };
        mutable Gaudi::Accumulators::StaticRootHistogram<2> m_gaussVflat{
            this, "GaussFlat", "Gaussian V Flat, atomic", { { 50, -5, 5, "X" }, { 50, -5, 5, "Y" } } };
        mutable Gaudi::Accumulators::StaticRootHistogram<3> m_gaussVflatVgauss{
            this,
            "GaussFlatGauss",
            "Gaussian V Flat V Gaussian, atomic",
            { { 10, -5, 5, "X" }, { 10, -5, 5, "Y" }, { 10, -5, 5, "Z" } } };

        // "default" case, dedicated to testing buffers
        mutable Gaudi::Accumulators::StaticRootHistogram<1> m_gauss_buf{
            this, "GaussBuf", "Gaussian mean=0, sigma=1, buffered", { 100, -5, 5 } };
        mutable Gaudi::Accumulators::StaticRootHistogram<2> m_gaussVflat_buf{
            this, "GaussFlatBuf", "Gaussian V Flat, buffered", { { 50, -5, 5 }, { 50, -5, 5 } } };
        mutable Gaudi::Accumulators::StaticRootHistogram<3> m_gaussVflatVgauss_buf{
            this,
            "GaussFlatGaussBuf",
            "Gaussian V Flat V Gaussian, buffered",
            { { 10, -5, 5 }, { 10, -5, 5 }, { 10, -5, 5 } } };

        // Custom histogram cases, specifying title and axis in python
        mutable Gaudi::Accumulators::RootHistogram<1> m_custom_gauss{ this, "CustomGauss",
                                                                      "Gaussian mean=0, sigma=1, atomic" };
        mutable Gaudi::Accumulators::RootHistogram<2> m_custom_gaussVflat{ this, "CustomGaussFlat",
                                                                           "Gaussian V Flat, atomic" };
        mutable Gaudi::Accumulators::RootHistogram<3> m_custom_gaussVflatVgauss{ this, "CustomGaussFlatGauss",
                                                                                 "Gaussian V Flat V Gaussian, atomic" };
      };
      DECLARE_COMPONENT( GaudiRootHistoAlgorithm )
    } // namespace Counter
  }   // namespace TestSuite
} // namespace Gaudi
