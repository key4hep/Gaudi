/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

namespace concurrency {

  class DecisionNode;
  class AlgorithmNode;
  class DataNode;
  class ConditionNode;

  class IGraphVisitor {
  public:
    virtual ~IGraphVisitor() = default;

    virtual bool visitEnter( DecisionNode& ) const { return true; }
    virtual bool visit( DecisionNode& ) { return true; }

    virtual bool visitEnter( AlgorithmNode& ) const { return true; }
    virtual bool visit( AlgorithmNode& ) { return true; }

    virtual bool visitEnter( DataNode& ) const { return true; }
    virtual bool visit( DataNode& ) { return true; }

    virtual bool visitEnter( ConditionNode& ) const { return true; }
    virtual bool visit( ConditionNode& ) { return true; }

    virtual void reset() {}
  };
} // namespace concurrency
