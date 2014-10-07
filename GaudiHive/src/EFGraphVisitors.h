#ifndef EFGRAPHVISITORS_H_
#define EFGRAPHVISITORS_H_

#include "IGraphVisitor.h"
#include "ExecutionFlowGraph.h"

namespace concurrency {

  class RunSimulator : public IGraphVisitor {
  public:
    /// Constructor
    RunSimulator(const int& slotNum) {
      m_nodesSucceeded = 0;
      m_slotNum = slotNum;
    };
    /// Destructor
    virtual ~RunSimulator() {};

    virtual bool visitEnter(DecisionNode& node) const;

    virtual bool visit(DecisionNode& node);

    virtual bool visitLeave(DecisionNode& node) const;


    virtual bool visitEnter(AlgorithmNode& node) const;

    virtual bool visit(AlgorithmNode& node);

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
      virtual ~Trigger() {};

      virtual bool visitEnter(DecisionNode& node) const;

      virtual bool visit(DecisionNode& node);

      virtual bool visitLeave(DecisionNode& node) const;


      virtual bool visitEnter(AlgorithmNode& node) const;

      virtual bool visit(AlgorithmNode& node);

    };

  class AlgorithmOnDataOutputRanker : public IGraphVisitor {
    public:
      /// Constructor
      AlgorithmOnDataOutputRanker() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      virtual ~AlgorithmOnDataOutputRanker() {};

      virtual bool visitEnter(DecisionNode& node) const {return true;};

      virtual bool visit(DecisionNode& node) {return true;};

      virtual bool visitLeave(DecisionNode& node) const {return true;};


      virtual bool visitEnter(AlgorithmNode& node) const {return true;};

      virtual bool visit(AlgorithmNode& node);

      };

}



#endif /* EFGRAPHVISITORS_H_ */
