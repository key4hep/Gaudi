#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include "../../EventSlot.h"
#include "../PrecedenceRulesGraph.h"
#include "IGraphVisitor.h"

#include <map>
#include <set>
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
    void        reset() override {
      m_foundViolations = false;
      m_status.clear();
    }

  private:
    std::ostringstream m_status{"  No 'Concurrent'/'Prompt' contradictions found"};
    bool               m_foundViolations{false};
  };

  //--------------------------------------------------------------------------
  class ActiveLineageScout : public IGraphVisitor {
  public:
    /// Constructor
    ActiveLineageScout( const EventSlot* slot, const ControlFlowNode& node )
        : m_slot( slot ), m_startNode( node ), m_previousNodeName( node.name() ){};

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override { return false; };
    bool visitEnter( DataNode& ) const override { return false; };
    bool visitEnter( ConditionNode& ) const override { return false; };

    bool visit( DecisionNode& ) override;

    void reset() override {
      m_active           = true;
      m_previousNodeName = m_startNode.name();
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
      m_previousNodeName = m_startNode.name();
    };

    bool reply() const override { return m_active && m_foundEntryPoint; };

    void visitParents( DecisionNode& ) override;

  private:
    bool m_foundEntryPoint{true};
  };

  //--------------------------------------------------------------------------
  class ConditionalLineageFinder : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( DataNode& ) const override { return false; };
    bool visitEnter( ConditionNode& ) const override { return false; };

    bool visit( DecisionNode& ) override;
    bool visit( AlgorithmNode& ) override;

    bool positive() const { return m_positive; };
    bool negative() const { return m_negative; };

    void reset() override {
      m_positive = false;
      m_negative = false;
    };

  private:
    bool m_positive{false};
    bool m_negative{false};
  };

  //--------------------------------------------------------------------------
  class ProductionAmbiguityFinder : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override { return false; };
    bool visitEnter( DecisionNode& ) const override { return false; };

    bool visit( DataNode& ) override;
    bool visit( ConditionNode& ) override;

    std::string reply() const;
    bool        passed() const {
      return std::all_of( m_unconditionalProducers.begin(), m_unconditionalProducers.end(),
                          []( const std::pair<DataNode*, std::set<AlgorithmNode*, CompareNodes<AlgorithmNode*>>>& pr ) {
                            return pr.second.size() == 1;
                          } );
    };
    void reset() override {
      m_foundViolations = false;
      m_conditionalProducers.clear();
      m_unconditionalProducers.clear();
    };

  private:
    bool m_foundViolations{false};

    template <typename T>
    struct CompareNodes {
      bool operator()( const T& a, const T& b ) const { return a->name() < b->name(); }
    };

    using visitor_book =
        std::map<DataNode*, std::set<AlgorithmNode*, CompareNodes<AlgorithmNode*>>, CompareNodes<DataNode*>>;
    visitor_book m_conditionalProducers;
    visitor_book m_unconditionalProducers;
  };

} // namespace concurrency

#endif /* VALIDATORS_H_ */
