#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include "../../EventSlot.h"
#include "../PrecedenceRulesGraph.h"
#include "IGraphVisitor.h"

#include <sstream>

namespace concurrency {

  //--------------------------------------------------------------------------
  class NodePropertiesValidator : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override { return false; };
    bool visitEnter( DataNode& ) const override { return false; };
    bool visitEnter( ConditionNode& ) const override { return false; };

    bool visit( DecisionNode& ) override;

    std::string reply() const { return m_status.str(); };
    bool        passed() const { return !m_foundViolations; };

  private:
    std::ostringstream m_status{"  No 'Concurrent'/'Prompt' contradictions found"};
    bool               m_foundViolations{false};
  };

  //--------------------------------------------------------------------------
  class ActiveLineageScout : public IGraphVisitor {
  public:
    /// Constructor
    ActiveLineageScout( const EventSlot* slot, const ControlFlowNode& node )
        : m_slot( slot ), m_startNode( node ), m_previousNodeName( node.getNodeName() ){};

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override { return false; };
    bool visitEnter( DataNode& ) const override { return false; };
    bool visitEnter( ConditionNode& ) const override { return false; };

    bool visit( DecisionNode& ) override;

    void reset() override {
      m_active           = true;
      m_previousNodeName = m_startNode.getNodeName();
    };

    virtual bool reply() const { return m_active; };

    virtual void visitParents( DecisionNode& );

  protected:
    const EventSlot*       m_slot;
    const ControlFlowNode& m_startNode;
    bool                   m_active{true};
    std::string            m_previousNodeName;
  };

  //--------------------------------------------------------------------------
  class SubSlotScout final : public ActiveLineageScout {
  public:
    /// Constructor
    SubSlotScout( const EventSlot* slot, const ControlFlowNode& node )
        : ActiveLineageScout( slot, node ), m_foundEntryPoint( slot->parentSlot == nullptr ){};

    void reset() override {
      m_active = true;

      // Only look for an entry point if we're in a sub-slot
      m_foundEntryPoint  = ( m_slot->parentSlot == nullptr );
      m_previousNodeName = m_startNode.getNodeName();
    };

    bool reply() const override { return m_active && m_foundEntryPoint; };

    void visitParents( DecisionNode& ) override;

  private:
    bool m_foundEntryPoint{true};
  };
} // namespace concurrency

#endif /* VALIDATORS_H_ */
