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

namespace Gaudi::Tests::Histograms {
  namespace Directories {
    struct HistoGroupsAlg : Gaudi::Functional::Consumer<void()> {
      using Base     = Gaudi::Functional::Consumer<void()>;
      using MyHist_t = Gaudi::Accumulators::Histogram<1>;

      using Base::Base;

      mutable MyHist_t m_hist0{this, "Top", "Top title", {1, 0, 1}};
      mutable MyHist_t m_hist1{this, "Group/First", "First title", {1, 0, 1}};
      mutable MyHist_t m_hist2{this, "Group/Second", "Second title", {1, 0, 1}};
      mutable MyHist_t m_hist3{this, "Group/SubGroup/Third", "Third title", {1, 0, 1}};

      void operator()() const override {
        ++m_hist0[0.5];
        ++m_hist1[0.5];
        ++m_hist2[0.5];
        ++m_hist3[0.5];
      }
    };
    DECLARE_COMPONENT( HistoGroupsAlg )
  } // namespace Directories
} // namespace Gaudi::Tests::Histograms
