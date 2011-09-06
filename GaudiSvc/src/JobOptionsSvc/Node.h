// $Id:$
#ifndef JOBOPTIONSVC_NODE_H_
#define JOBOPTIONSVC_NODE_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
// ============================================================================
// BOOST:
// ============================================================================

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/unused.hpp>
#include <boost/range/iterator_range.hpp>
// ============================================================================
#include "Position.h"
#include "Iterator.h"
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class Node {
public:
 enum NodeType {kRoot,kInclude, kIdentifier, kProperty, kOperation,
  kValue, kAssign, kEqual, kPlusEqual, kMinusEqual, kVector, kMap, kPair,
  kSimple, kString, kReal, kBool, kUnits, kUnit, kCondition, kIfdef, kIfndef,
  kElse, kPrintOptions, kPrintOn, kPrintOff, kShell, kPrintTree, kDumpFile,
  kPropertyRef};
  NodeType type;
  std::string value;
  std::vector<Node> children;
  Position position;

  Node(): type(kRoot){}
  std::string name() const;
  std::string ToString() const;
  std::string ToString(int indent) const;
};
// ============================================================================
class NodeOperations {
public:
  struct value {};
  template<typename A, typename B = boost::fusion::unused_type,
    typename C = boost::fusion::unused_type,
    typename D = boost::fusion::unused_type>
  struct result {
    typedef void type;
  };
  //---------------------------------------------------------------------------
  void operator()(Node& node, Node::NodeType type) const {
    node.type = type;
  }

  void operator()(Node& node, Node& child) const {
    node.children.push_back(child);
  }

  void operator()(Node& node, const std::string& val) const {
    node.value = val;
  }

  void operator()(Node& node, boost::iterator_range<Iterator> range) const {
    node.value = boost::copy_range<std::string>(range);
  }

  void operator()(Node& node, bool val) const {
    node.value = val?"1":"0";
  }

  void operator()(Node& node, const Iterator& iter) const {
    const IteratorPosition& pos = iter.get_position();
    node.position = Position(pos.file, pos.line, pos.column);
  }

  //---------------------------------------------------------------------------
};
// ============================================================================
}  /* Gaudi */ }  /* Parsers */
// ============================================================================
BOOST_FUSION_ADAPT_STRUCT(
        Gaudi::Parsers::Node,
        (Gaudi::Parsers::Node::NodeType, type)
        (std::string, value)
        (std::vector<Gaudi::Parsers::Node>, children)
)
// ============================================================================
#endif  // JOBOPTIONSVC_NODE_H_
