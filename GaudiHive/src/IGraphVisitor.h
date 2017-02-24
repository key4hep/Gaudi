#ifndef IGRAPHVISITOR_H_
#define IGRAPHVISITOR_H_

namespace concurrency {

  class DecisionNode;
  class AlgorithmNode;

  class IGraphVisitor {
  public:
    virtual ~IGraphVisitor() {};

    virtual bool visitEnter(DecisionNode&) const = 0;
    virtual bool visit(DecisionNode&) = 0;

    virtual bool visitEnter(AlgorithmNode&) const = 0;
    virtual bool visit(AlgorithmNode& node) = 0;

    virtual void reset() = 0;

  public:
    int m_nodesSucceeded;
    int m_slotNum;
  };
}

#endif /* IGRAPHVISITOR_H_ */
