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
#ifndef PROMOTERS_H_
#define PROMOTERS_H_

#include "../../EventSlot.h"
#include "../PrecedenceRulesGraph.h"
#include "IGraphVisitor.h"

namespace concurrency {

  //--------------------------------------------------------------------------
  class DataReadyPromoter : public IGraphVisitor {
  public:
    /// Constructor
    DataReadyPromoter( EventSlot& slot, const Cause& cause, bool ifTrace = false )
        : m_slot( &slot ), m_cause( cause ), m_trace( ifTrace ){};

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override;

    bool visit( AlgorithmNode& ) override;

    bool visitEnter( DataNode& ) const override;

    bool visit( DataNode& ) override;

    bool visitEnter( ConditionNode& ) const override;

    bool visit( ConditionNode& ) override;

    EventSlot* m_slot;
    Cause      m_cause;
    bool       m_trace;
  };

  //--------------------------------------------------------------------------
  class DecisionUpdater : public IGraphVisitor {
  public:
    /// Constructor
    DecisionUpdater( EventSlot& slot, const Cause& cause, bool ifTrace = false )
        : m_slot( &slot ), m_cause( cause ), m_trace( ifTrace ){};

    using IGraphVisitor::visit;

    bool visit( AlgorithmNode& ) override;

    EventSlot* m_slot;
    Cause      m_cause;
    bool       m_trace;
  };

  //--------------------------------------------------------------------------
  class Supervisor : public IGraphVisitor {
  public:
    /// Constructor
    Supervisor( EventSlot& slot, const Cause& cause, bool ifTrace = false )
        : m_slot( &slot ), m_cause( cause ), m_trace( ifTrace ){};

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( DecisionNode& ) const override;

    bool visit( DecisionNode& ) override;

    bool visitEnter( AlgorithmNode& ) const override;

    bool visit( AlgorithmNode& ) override;

    EventSlot* m_slot;
    Cause      m_cause;
    bool       m_trace;
  };

  //--------------------------------------------------------------------------
  class RunSimulator : public IGraphVisitor {
  public:
    /// Constructor
    RunSimulator( EventSlot& slot, const Cause& cause ) : m_slot( &slot ), m_cause( cause ){};

    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( DecisionNode& ) const override;

    bool visit( DecisionNode& ) override;

    bool visitEnter( AlgorithmNode& ) const override;

    bool visit( AlgorithmNode& ) override;

    void reset() override { m_nodesSucceeded = 0; };

    EventSlot* m_slot;
    Cause      m_cause;
    int        m_nodesSucceeded{0};
  };
} // namespace concurrency

#endif /* PROMOTERS_H_ */
