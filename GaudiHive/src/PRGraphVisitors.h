#ifndef PRGRAPHVISITORS_H_
#define PRGRAPHVISITORS_H_

#include "IGraphVisitor.h"
#include "PrecedenceRulesGraph.h"

namespace concurrency {

  class RunSimulator : public IGraphVisitor {
  public:
    /// Constructor
    RunSimulator(const int& slotNum) {
      m_nodesSucceeded = 0;
      m_slotNum = slotNum;
    };
    /// Destructor
    ~RunSimulator() override {}

    bool visitEnter(DecisionNode& node) const override;

    bool visit(DecisionNode& node) override;

    bool visitLeave(DecisionNode& node) const override;


    bool visitEnter(AlgorithmNode& node) const override;

    bool visit(AlgorithmNode& node) override;


    void reset() override { m_nodesSucceeded = 0; }

  };

  /** A visitor, performing full top-down traversals of a graph
   *
   */
  class Trigger : public IGraphVisitor {
    public:
      /// Constructor
      Trigger(const int& slotNum) {
        m_nodesSucceeded = 0;
        m_slotNum = slotNum;
      };
      /// Destructor
      ~Trigger() override {}

      bool visitEnter(DecisionNode& node) const override;

      bool visit(DecisionNode& node) override;

      bool visitLeave(DecisionNode& /*node*/) const override {return true;};


      bool visitEnter(AlgorithmNode& node) const override;

      bool visit(AlgorithmNode& node) override;


      void reset() override { m_nodesSucceeded = 0; }

    };


  class Supervisor : public IGraphVisitor {
    public:
      /// Constructor
	  Supervisor(const int& slotNum) {
        m_nodesSucceeded = 0;
        m_slotNum = slotNum;
      };
      /// Destructor
      ~Supervisor() override {}

      bool visitEnter(DecisionNode& node) const override;

      bool visit(DecisionNode& node) override;

      bool visitLeave(DecisionNode& /*node*/) const override {return true;};


      bool visitEnter(AlgorithmNode& node) const override;

      bool visit(AlgorithmNode& node) override;


      void reset() override { m_nodesSucceeded = 0; }

    };

  class RankerByProductConsumption : public IGraphVisitor {
    public:
      /// Constructor
      RankerByProductConsumption() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      ~RankerByProductConsumption() override {}

      bool visitEnter(DecisionNode&) const override {return true;}

      bool visit(DecisionNode&) override {return true;}\

      bool visitLeave(DecisionNode&) const override {return true;}


      bool visitEnter(AlgorithmNode&) const override {return true;}

      bool visit(AlgorithmNode& node) override;


      void reset() override { m_nodesSucceeded = 0; }

      };

  class RankerByCummulativeOutDegree : public IGraphVisitor {
    public:
      /// Constructor
      RankerByCummulativeOutDegree() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      ~RankerByCummulativeOutDegree() override {}

      bool visitEnter(DecisionNode&) const override {return true;}

      bool visit(DecisionNode&) override {return true;}

      bool visitLeave(DecisionNode&) const override {return true;}


      bool visitEnter(AlgorithmNode&) const override {return true;}

      bool visit(AlgorithmNode& node) override;


      void reset() override { m_nodesSucceeded = 0; }

      void runThroughAdjacents(boost::graph_traits<boost::ExecPlan>::vertex_descriptor vertex, boost::ExecPlan graph);

      };

  class RankerByTiming : public IGraphVisitor {
    public:
      /// Constructor
      RankerByTiming() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      ~RankerByTiming() override {}

      bool visitEnter(DecisionNode&) const override {return true;}

      bool visit(DecisionNode&) override {return true;}

      bool visitLeave(DecisionNode&) const override {return true;}


      bool visitEnter(AlgorithmNode&) const override {return true;}

      bool visit(AlgorithmNode& node) override;


      void reset() override { m_nodesSucceeded = 0; }

      };

  class RankerByEccentricity : public IGraphVisitor {
    public:
      /// Constructor
      RankerByEccentricity() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      ~RankerByEccentricity() override {}

      bool visitEnter(DecisionNode&) const override {return true;}

      bool visit(DecisionNode&) override {return true;}

      bool visitLeave(DecisionNode&) const override {return true;}


      bool visitEnter(AlgorithmNode&) const override {return true;}

      bool visit(AlgorithmNode& node) override;


      void reset() override { m_nodesSucceeded = 0; }

      };

  class RankerByDataRealmEccentricity : public IGraphVisitor {
    public:
      /// Constructor
      RankerByDataRealmEccentricity() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
        m_currentDepth = 0;
        m_maxKnownDepth = 0;
      };
      /// Destructor
      ~RankerByDataRealmEccentricity() override {}

      bool visitEnter(DecisionNode&) const override {return true;}

      bool visit(DecisionNode&) override {return true;}

      bool visitLeave(DecisionNode&) const override {return true;}


      bool visitEnter(AlgorithmNode&) const override {return true;}

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

}



#endif /* PRGRAPHVISITORS_H_ */
