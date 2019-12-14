/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include "../../EventSlot.h"
#include "../PrecedenceRulesGraph.h"
#include "IGraphVisitor.h"

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

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
                          []( const auto& pr ) { return pr.second.size() == 1; } );
    };
    void reset() override {
      m_foundViolations = false;
      m_conditionalProducers.clear();
      m_unconditionalProducers.clear();
    };

  private:
    bool m_foundViolations{false};

    using visitor_book =
        std::map<DataNode*, std::set<AlgorithmNode*, CompareNodes<AlgorithmNode*>>, CompareNodes<DataNode*>>;
    visitor_book m_conditionalProducers;
    visitor_book m_unconditionalProducers;
  };

  /**@class TarjanSCCFinder Validators.h
   *
   * The visitor implements the Tarjan algorithm for searching strongly
   * connected components in the data flow realm of precedence rules graph.
   *
   */
  class TarjanSCCFinder : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( ConditionNode& ) const override { return false; };
    bool visitEnter( DecisionNode& ) const override { return false; };
    bool visitEnter( AlgorithmNode& node ) const override {
      // check if the node was already visited
      return m_lowlinks.find( &node ) != m_lowlinks.end() ? false : true;
    }

    bool visit( AlgorithmNode& nodeAt ) override;

    bool passed() const {
      return m_scc.empty() ||
             !std::any_of( m_scc.begin(), m_scc.end(), []( const auto& pr ) { return pr.second.size() > 1; } );
    };

    std::string reply();

    void reset() override {
      m_nodes_count = 0;
      m_stack.clear();
      m_lowlinks.clear();
      m_scc.clear();
      m_status = std::ostringstream{"  No strongly connected components found in DF realm"};
    };

  private:
    bool on_stack( const AlgorithmNode& node ) const {
      return std::find( m_stack.begin(), m_stack.end(), &node ) != m_stack.end() ? true : false;
    }

    unsigned int m_nodes_count{0};

    std::unordered_map<AlgorithmNode*, std::pair<unsigned int, unsigned int>> m_lowlinks;
    std::map<unsigned int, std::vector<AlgorithmNode*>>                       m_scc;
    std::vector<AlgorithmNode*>                                               m_stack;

    std::ostringstream m_status{"  No strongly connected components found in DF realm"};
  };

} // namespace concurrency

#endif /* VALIDATORS_H_ */
