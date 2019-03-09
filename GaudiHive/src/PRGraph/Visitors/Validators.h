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
  class ActiveSubgraphScout : public IGraphVisitor {
  public:
    /// Constructor
    ActiveSubgraphScout( const EventSlot& slot, const Cause& cause )
        : m_slot( slot ), m_cause( cause ), m_previousNodeName( cause.m_sourceName ){};

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override { return false; };
    bool visitEnter( DataNode& ) const override { return false; };
    bool visitEnter( ConditionNode& ) const override { return false; };

    bool visit( DecisionNode& ) override;

    void reset() override {
      m_active           = false;
      m_previousNodeName = m_cause.m_sourceName;
    };

    bool reply() const { return m_active; };

  private:
    const EventSlot& m_slot;
    const Cause&     m_cause;
    bool             m_active{true};
    std::string      m_previousNodeName;
  };

} // namespace concurrency

#endif /* VALIDATORS_H_ */
