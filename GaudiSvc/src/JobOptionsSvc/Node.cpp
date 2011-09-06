// $Id:$
// ============================================================================
// Includes:
// ============================================================================
#include "Node.h"
// ============================================================================
// Boost:
// ============================================================================
#include <boost/foreach.hpp>
#include <boost/format.hpp>
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
void tab(std::string* str, int indent) {
  for (int i = 0; i < indent; ++i)
      *str += ' ';
}

std::string names[100];
// ============================================================================
std::string gp::Node::name() const {
    static bool init = false;
  if (!init) { // lazy init
      names[kRoot]        = "root";
      names[kInclude]     = "include";
      names[kIdentifier]  = "identifier";
      names[kProperty]    = "property";
      names[kOperation]   = "operation";
      names[kValue]       = "value";
      names[kAssign]      = "assign";
      names[kEqual]       = "equal";
      names[kPlusEqual]   = "plus_equal";
      names[kMinusEqual]  = "minus_equal";
      names[kVector]      = "vector";
      names[kMap]         = "map";
      names[kPair]        = "pair";
      names[kSimple]      = "simple";
      names[kString]      = "string";
      names[kReal]        = "real";
      names[kBool]        = "bool";
      names[kUnits]       = "units";
      names[kUnit]        = "unit";
      names[kCondition]   = "condition";
      names[kIfdef]       = "ifdef";
      names[kIfndef]      = "ifndef";
      names[kElse]        = "else";
      names[kPrintOptions] = "print_options";
      names[kPrintOn]     = "print_on";
      names[kPrintOff]    = "print_off";
      names[kShell]       = "shell";
      names[kPrintTree]   = "print_tree";
      names[kDumpFile]    = "dump_file";
      names[kPropertyRef] = "property_ref";
  }
  return names[type];
}

// ============================================================================
std::string gp::Node::ToString() const {
    return ToString(0);
}
// ============================================================================
std::string gp::Node::ToString(int indent) const {

    std::string result = "";
    tab(&result, indent);
    result += "<" + name();
    if (value != "")
        result += " value=\"" + value + "\"";
    if (position.line() != 0)
        result += " line=\"" + str(boost::format("%1%") % position.line())
          + "\"";
    if (position.column() != 0)
        result += " column=\"" + str(boost::format("%1%") % position.column())
          + "\"";

    if (children.size() == 0) {
        result += "/>\n";
    } else {
        result += ">\n";
        BOOST_FOREACH(Node const& node, children)
                    {
                        result += node.ToString(indent + 2);
                    }
        tab(&result, indent);
        result += str(boost::format("</%1%>\n") % name());
    }
    return result;
}
// ============================================================================
