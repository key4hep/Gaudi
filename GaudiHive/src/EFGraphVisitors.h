#ifndef EFGRAPHVISITORS_H_
#define EFGRAPHVISITORS_H_

#include "IGraphVisitor.h"
#include "ControlFlowManager.h"

namespace concurrency {

  class CFVisitor : public IGraphVisitor {
  public:
    /// Constructor
    CFVisitor(const int& slotNum) {
      m_nodesSucceeded = 0;
      m_slotNum = slotNum;
    };
    /// Destructor
    virtual ~CFVisitor() {};

    virtual bool visitEnter(DecisionNode& node) const;

    virtual bool visit(DecisionNode& node) const;

    virtual bool visitLeave(DecisionNode& node) const;


    virtual bool visitEnter(AlgorithmNode& node) const;

    virtual bool visit(AlgorithmNode& node);

  };

}



#endif /* EFGRAPHVISITORS_H_ */
