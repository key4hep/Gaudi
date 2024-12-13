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
#ifndef RANKERS_H_
#define RANKERS_H_

#include "../PrecedenceRulesGraph.h"
#include "IGraphVisitor.h"

namespace concurrency {

  //--------------------------------------------------------------------------
  class RankerByProductConsumption : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;

    bool visit( AlgorithmNode& ) override;
  };

  //--------------------------------------------------------------------------
  class RankerByCummulativeOutDegree : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;

    bool visit( AlgorithmNode& ) override;
    void reset() override { m_nodesSucceeded = 0; }

    void runThroughAdjacents( boost::graph_traits<precedence::PrecTrace>::vertex_descriptor, precedence::PrecTrace );

    int m_nodesSucceeded{ 0 };
  };

  //--------------------------------------------------------------------------
  class RankerByTiming : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;

    bool visit( AlgorithmNode& ) override;
  };

  //--------------------------------------------------------------------------
  class RankerByEccentricity : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;

    bool visit( AlgorithmNode& ) override;
  };

  //--------------------------------------------------------------------------
  class RankerByDataRealmEccentricity : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;

    bool visit( AlgorithmNode& ) override;

    void reset() override {
      m_currentDepth  = 0;
      m_maxKnownDepth = 0;
    }

    /// Depth-first node parser to calculate node eccentricity
    /// (only within the data realm of the precedence rules graph)
    void recursiveVisit( AlgorithmNode& );

    uint m_currentDepth{ 0 };
    uint m_maxKnownDepth{ 0 };
  };

} // namespace concurrency

#endif /* RANKERS_H_ */
