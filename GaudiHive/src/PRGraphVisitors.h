#ifndef PRGRAPHVISITORS_H_
#define PRGRAPHVISITORS_H_

#include "IGraphVisitor.h"
#include "PrecedenceRulesGraph.h"

namespace concurrency {

  class DataReadyPromoter : public IGraphVisitor {
    public:
      /// Constructor
      DataReadyPromoter(EventSlot& slot) {
        m_nodesSucceeded = 0;
        m_slot = &slot;
      };

      using IGraphVisitor::visit;
      using IGraphVisitor::visitEnter;

      bool visit(AlgorithmNode& node) override;

      bool visitEnter(DataNode& node) const override;

      bool visit(DataNode& node) override;

      bool visitEnter(ConditionNode&) const override;

      bool visit(ConditionNode&) override;

  };

  class DecisionUpdater : public IGraphVisitor {
    public:
      /// Constructor
      DecisionUpdater(EventSlot& slot) {
        m_nodesSucceeded = 0;
        m_slot = &slot;
      };

      using IGraphVisitor::visit;

      bool visit(AlgorithmNode& node) override;

  };


  class Supervisor : public IGraphVisitor {
    public:
      /// Constructor
	  Supervisor(EventSlot& slot) {
        m_nodesSucceeded = 0;
        m_slot = &slot;
      };

      using IGraphVisitor::visit;
      using IGraphVisitor::visitEnter;

      bool visitEnter(DecisionNode& node) const override;

      bool visit(DecisionNode& node) override;

      bool visitEnter(AlgorithmNode& node) const override;

      bool visit(AlgorithmNode& node) override;

  };

  class RankerByProductConsumption : public IGraphVisitor {
    public:
      /// Constructor
      RankerByProductConsumption() {
        m_nodesSucceeded = 0;
        m_slot = nullptr;
      };

      using IGraphVisitor::visit;

      bool visit(AlgorithmNode& node) override;

  };

  class RankerByCummulativeOutDegree : public IGraphVisitor {
    public:
      /// Constructor
      RankerByCummulativeOutDegree() {
        m_nodesSucceeded = 0;
        m_slot = nullptr;
      };

      using IGraphVisitor::visit;

      bool visit(AlgorithmNode& node) override;

      void runThroughAdjacents(boost::graph_traits<boost::ExecPlan>::vertex_descriptor vertex, boost::ExecPlan graph);

  };

  class RankerByTiming : public IGraphVisitor {
    public:
      /// Constructor
      RankerByTiming() {
        m_nodesSucceeded = 0;
        m_slot = nullptr;
      };

      using IGraphVisitor::visit;

      bool visit(AlgorithmNode& node) override;

  };

  class RankerByEccentricity : public IGraphVisitor {
    public:
      /// Constructor
      RankerByEccentricity() {
        m_nodesSucceeded = 0;
        m_slot = nullptr;
      };

      using IGraphVisitor::visit;

      bool visit(AlgorithmNode& node) override;

  };

  class RankerByDataRealmEccentricity : public IGraphVisitor {
    public:
      /// Constructor
      RankerByDataRealmEccentricity() {
        m_nodesSucceeded = 0;
        m_slot = nullptr;
        m_currentDepth = 0;
        m_maxKnownDepth = 0;
      };

      using IGraphVisitor::visit;

      bool visit(AlgorithmNode& node) override;

      void reset() override {
        m_nodesSucceeded = 0;
        m_currentDepth = 0;
        m_maxKnownDepth = 0;
      }

      /// Depth-first node parser to calculate node eccentricity
      /// (only within the data realm of the precedence rules graph)
      void recursiveVisit(AlgorithmNode& node);

      uint m_currentDepth;
      uint m_maxKnownDepth;

  };

  class RunSimulator : public IGraphVisitor {
  public:
    /// Constructor
    RunSimulator(EventSlot& slot) {
      m_nodesSucceeded = 0;
      m_slot = &slot;
    };

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter(DecisionNode& node) const override;

    bool visit(DecisionNode& node) override;

    bool visitEnter(AlgorithmNode& node) const override;

    bool visit(AlgorithmNode& node) override;

  };

}



#endif /* PRGRAPHVISITORS_H_ */
