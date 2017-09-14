#ifndef IGRAPHVISITOR_H_
#define IGRAPHVISITOR_H_

#include "EventSlot.h"

namespace concurrency {

  class DecisionNode;
  class AlgorithmNode;
  class DataNode;
  class ConditionNode;

  class IGraphVisitor {
  public:
    virtual ~IGraphVisitor() = default;

    virtual bool visitEnter(DecisionNode&) const {return true;};
    virtual bool visit(DecisionNode&) {return true;};

    virtual bool visitEnter(AlgorithmNode&) const {return true;};
    virtual bool visit(AlgorithmNode&) {return true;};

    virtual bool visitEnter(DataNode&) const {return true;};
    virtual bool visit(DataNode&) {return true;};

    virtual bool visitEnter(ConditionNode&) const {return true;};
    virtual bool visit(ConditionNode&) {return true;};

    virtual void reset() {m_nodesSucceeded = 0;};

  public:
    int m_nodesSucceeded;
    EventSlot* m_slot;
  };
}

#endif /* IGRAPHVISITOR_H_ */
