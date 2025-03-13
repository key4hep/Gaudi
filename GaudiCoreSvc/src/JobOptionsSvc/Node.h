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
#ifndef JOBOPTIONSVC_NODE_H_
#define JOBOPTIONSVC_NODE_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// BOOST:
// ============================================================================

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/unused.hpp>
#include <boost/range/iterator_range.hpp>
// ============================================================================
#include "Iterator.h"
#include "Position.h"
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    struct Node final {
      enum NodeType {
        kRoot,
        kInclude,
        kIdentifier,
        kProperty,
        kOperation,
        kValue,
        kAssign,
        kEqual,
        kPlusEqual,
        kMinusEqual,
        kVector,
        kMap,
        kPair,
        kSimple,
        kString,
        kReal,
        kBool,
        kUnits,
        kUnit,
        kCondition,
        kIfdef,
        kIfndef,
        kElse,
        kPrintOptions,
        kPrintOn,
        kPrintOff,
        kShell,
        kPrintTree,
        kDumpFile,
        kPropertyRef,
        number_of_node_types
      };
      NodeType          type = kRoot;
      std::string       value;
      std::vector<Node> children;
      Position          position;

      Node() = default;
      std::string name() const;
      std::string ToString( int indent = 0 ) const;
    };
    // ============================================================================
    struct NodeOperations final {
      struct value {};
      //---------------------------------------------------------------------------
      void operator()( Node& node, Node::NodeType type ) const { node.type = type; }

      void operator()( Node& node, Node child ) const { node.children.push_back( std::move( child ) ); }

      void operator()( Node& node, std::string val ) const { node.value = std::move( val ); }

      void operator()( Node& node, boost::iterator_range<Iterator> range ) const {
        node.value = boost::copy_range<std::string>( range );
      }

      void operator()( Node& node, bool val ) const { node.value = val ? "1" : "0"; }

      void operator()( Node& node, const Iterator& iter ) const {
        const IteratorPosition& pos = iter.get_position();
        node.position               = Position( pos.file, pos.line, pos.column );
      }

      //---------------------------------------------------------------------------
    };
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
BOOST_FUSION_ADAPT_STRUCT( Gaudi::Parsers::Node,
                           ( Gaudi::Parsers::Node::NodeType,
                             type )( std::string, value )( std::vector<Gaudi::Parsers::Node>, children ) )
// ============================================================================
#endif // JOBOPTIONSVC_NODE_H_
