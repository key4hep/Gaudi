// $Id:$
// ============================================================================
// Include files
// ============================================================================
#include "Units.h"
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
bool gp::Units::Add(const std::string& name, double value) {
  return Add(name, value, Position());
}
// ============================================================================
bool gp::Units::Add(const std::string& name, double value,
    const Position& pos) {
  std::pair<Container::iterator,bool> result =
      units_.insert(
          Container::value_type(name,ValueWithPosition(value, pos))
      );
  return result.second;
}
// ============================================================================
bool gp::Units::Find(const std::string& name, double& result) const {

  ValueWithPosition r;
  if (Find(name,r)){
    result = r.first;
    return true;
  }
  return false;
}
// ============================================================================
bool gp::Units::Find(const std::string& name,
    ValueWithPosition& result) const {
  Container::const_iterator it = units_.find(name);
  if (it != units_.end()) {
    result = it->second;
    return true;
  }
  return false;
}
