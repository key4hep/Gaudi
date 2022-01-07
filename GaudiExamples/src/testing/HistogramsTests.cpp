/***********************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators/Histogram.h>
#include <GaudiAlg/Consumer.h>
#include <type_traits>

namespace Gaudi::Tests::Histograms::CustomAxis {
  enum class Category { Simple, Complex, Bad, Wrong };
}

namespace Gaudi::Accumulators {
  template <>
  struct Axis<Gaudi::Tests::Histograms::CustomAxis::Category> {
    using Category = Gaudi::Tests::Histograms::CustomAxis::Category;

    Axis() = default;
    /// number of bins for this Axis
    unsigned int nBins = 4;
    /// min and max values on this axis
    std::underlying_type_t<Category> minValue = 0, maxValue = 4;
    /// title of this axis
    std::string title{ "Category" };
    /// labels for the bins
    std::vector<std::string> labels{ "Simple", "Complex", "Bad", "Wrong" };

    unsigned int index( Category value ) const { return static_cast<unsigned int>( value ) + 1; }
  };
} // namespace Gaudi::Accumulators

namespace Gaudi::Tests::Histograms {
  namespace Directories {
    using MyHist_t = Gaudi::Accumulators::Histogram<1>;

    struct HistoGroupsTool : AlgTool {
      using AlgTool::AlgTool;

      mutable MyHist_t m_hist0{ this, "Top", "Top title", { 1, 0, 1 } };
      mutable MyHist_t m_hist1{ this, "Group/First", "First title", { 1, 0, 1 } };
      mutable MyHist_t m_hist2{ this, "Group/Second", "Second title", { 1, 0, 1 } };
      mutable MyHist_t m_hist3{ this, "Group/SubGroup/Third", "Third title", { 1, 0, 1 } };

      void fillHistos() const {
        ++m_hist0[0.5];
        ++m_hist1[0.5];
        ++m_hist2[0.5];
        ++m_hist3[0.5];
      }
    };
    DECLARE_COMPONENT( HistoGroupsTool )

    struct HistoGroupsAlg : Gaudi::Functional::Consumer<void()> {
      using Base = Gaudi::Functional::Consumer<void()>;
      using Base::Base;

      mutable MyHist_t m_hist0{ this, "Top", "Top title", { 1, 0, 1 } };
      mutable MyHist_t m_hist1{ this, "Group/First", "First title", { 1, 0, 1 } };
      mutable MyHist_t m_hist2{ this, "Group/Second", "Second title", { 1, 0, 1 } };
      mutable MyHist_t m_hist3{ this, "Group/SubGroup/Third", "Third title", { 1, 0, 1 } };

      ToolHandle<HistoGroupsTool> m_tool{ this, "Tool", "Gaudi::Tests::Histograms::Directories::HistoGroupsTool/Tool" };

      void operator()() const override {
        ++m_hist0[0.5];
        ++m_hist1[0.5];
        ++m_hist2[0.5];
        ++m_hist3[0.5];
        m_tool->fillHistos();
      }
    };
    DECLARE_COMPONENT( HistoGroupsAlg )
  } // namespace Directories
  namespace AxesLabels {
    struct HistWithLabelsAlg : Gaudi::Functional::Consumer<void()> {
      using Base     = Gaudi::Functional::Consumer<void()>;
      using MyHist_t = Gaudi::Accumulators::Histogram<1, Gaudi::Accumulators::atomicity::full, int>;

      using Base::Base;

      mutable MyHist_t m_hist{
          this, "hist", "Histogram title", { 5, 0, 5, "axis title", { "a", "b", "c", "d", "e" } } };

      void operator()() const override {
        for ( int i : { 1, 2, 3, 4, 5 } ) m_hist[i - 1] += i;
      }
    };
    DECLARE_COMPONENT( HistWithLabelsAlg )
  } // namespace AxesLabels
  namespace CustomAxis {
    struct EnumAxisAlg : Gaudi::Functional::Consumer<void()> {
      using Base = Gaudi::Functional::Consumer<void()>;
      using Base::Base;

      mutable Gaudi::Accumulators::Histogram<1, Gaudi::Accumulators::atomicity::full, Category> m_hist{
          this, "Categories", "", Gaudi::Accumulators::Axis<Category>{} };

      void operator()() const override {
        ++m_hist[Category::Simple];
        m_hist[Category::Complex] += 2;
        m_hist[Category::Bad] += 3;
        m_hist[Category::Wrong] += 4;
      }
    };
    DECLARE_COMPONENT( EnumAxisAlg )
  } // namespace CustomAxis
  namespace MultiDimLayout {
    // Simple algorithm used to check https://gitlab.cern.ch/gaudi/Gaudi/-/issues/212
    struct TestAlg : Gaudi::Functional::Consumer<void()> {
      using Base = Gaudi::Functional::Consumer<void()>;
      using Base::Base;

      mutable Gaudi::Accumulators::Histogram<1> m_h1{ this, "h1", "", { 10, 0, 10 } };
      mutable Gaudi::Accumulators::Histogram<2> m_h2{ this, "h2", "", { 10, 0, 10 }, { 10, 0, 10 } };
      mutable Gaudi::Accumulators::Histogram<3> m_h3{ this, "h3", "", { 10, 0, 10 }, { 10, 0, 10 }, { 10, 0, 10 } };

      void operator()() const override {
        int value = 0;
        // fill 1, 2 and 3 dimensional histograms with different values in each bin
        for ( double x = -0.5; x < 11; x += 1.0 ) {
          m_h1[x] += ++value;
          for ( double y = -0.5; y < 11; y += 1.0 ) {
            m_h2[{ x, y }] += ++value;
            for ( double z = -0.5; z < 11; z += 1.0 ) { m_h3[{ x, y, z }] += ++value; }
          }
        }
      }
    };
    DECLARE_COMPONENT( TestAlg )
  } // namespace MultiDimLayout
} // namespace Gaudi::Tests::Histograms
