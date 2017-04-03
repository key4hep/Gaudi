#ifndef IGRAPHVISITOR_H_
#define IGRAPHVISITOR_H_

#include "EventSlot.h"

namespace concurrency {

  class DecisionNode;
  class AlgorithmNode;
  class DataNode;

  class IGraphVisitor {
  public:
    virtual ~IGraphVisitor() = default;

    virtual bool visitEnter(DecisionNode&) const = 0;
    virtual bool visit(DecisionNode&) = 0;

    virtual bool visitEnter(AlgorithmNode&) const = 0;
    virtual bool visit(AlgorithmNode&) = 0;

    virtual bool visitEnter(DataNode&) const = 0;
    virtual bool visit(DataNode&) = 0;

    virtual void reset() = 0;

  public:
    int m_nodesSucceeded;
    EventSlot* m_slot;
  };
}

#endif /* IGRAPHVISITOR_H_ */
