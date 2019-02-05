// ============================================================================
// Includes:
// ============================================================================
#include "Node.h"
#include <algorithm>
#include <numeric>
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================

std::vector<std::string> init_names() {
  std::vector<std::string> n( gp::Node::NodeType::number_of_node_types );
  n[gp::Node::NodeType::kRoot]         = "root";
  n[gp::Node::NodeType::kInclude]      = "include";
  n[gp::Node::NodeType::kIdentifier]   = "identifier";
  n[gp::Node::NodeType::kProperty]     = "property";
  n[gp::Node::NodeType::kOperation]    = "operation";
  n[gp::Node::NodeType::kValue]        = "value";
  n[gp::Node::NodeType::kAssign]       = "assign";
  n[gp::Node::NodeType::kEqual]        = "equal";
  n[gp::Node::NodeType::kPlusEqual]    = "plus_equal";
  n[gp::Node::NodeType::kMinusEqual]   = "minus_equal";
  n[gp::Node::NodeType::kVector]       = "vector";
  n[gp::Node::NodeType::kMap]          = "map";
  n[gp::Node::NodeType::kPair]         = "pair";
  n[gp::Node::NodeType::kSimple]       = "simple";
  n[gp::Node::NodeType::kString]       = "string";
  n[gp::Node::NodeType::kReal]         = "real";
  n[gp::Node::NodeType::kBool]         = "bool";
  n[gp::Node::NodeType::kUnits]        = "units";
  n[gp::Node::NodeType::kUnit]         = "unit";
  n[gp::Node::NodeType::kCondition]    = "condition";
  n[gp::Node::NodeType::kIfdef]        = "ifdef";
  n[gp::Node::NodeType::kIfndef]       = "ifndef";
  n[gp::Node::NodeType::kElse]         = "else";
  n[gp::Node::NodeType::kPrintOptions] = "print_options";
  n[gp::Node::NodeType::kPrintOn]      = "print_on";
  n[gp::Node::NodeType::kPrintOff]     = "print_off";
  n[gp::Node::NodeType::kShell]        = "shell";
  n[gp::Node::NodeType::kPrintTree]    = "print_tree";
  n[gp::Node::NodeType::kDumpFile]     = "dump_file";
  n[gp::Node::NodeType::kPropertyRef]  = "property_ref";
  return n;
}

// ============================================================================
std::string gp::Node::name() const {
  static const auto names = init_names();
  return names[type];
}

// ============================================================================
std::string gp::Node::ToString( int indent ) const {

  std::string result( indent, ' ' );
  result += "<" + name();
  if ( !value.empty() ) result += " value=\"" + value + "\"";
  if ( position.line() != 0 ) result += " line=\"" + std::to_string( position.line() ) + "\"";
  if ( position.column() != 0 ) result += " column=\"" + std::to_string( position.column() ) + "\"";
  if ( children.empty() ) {
    result += "/>\n";
  } else {
    result = std::accumulate( std::begin( children ), std::end( children ), result + ">\n",
                              [&]( std::string s, const gp::Node& node ) { return s + node.ToString( indent + 2 ); } )
                 .append( ' ', indent ) +
             "</" + name() + ">\n";
  }
  return result;
}
// ============================================================================
