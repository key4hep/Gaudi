// $Id:$
#ifndef JOBOPTIONSVC_UNITS_H_
#define JOBOPTIONSVC_UNITS_H_
// ============================================================================
// Include files
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <map>
// ============================================================================
#include "Position.h"
// ============================================================================

// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class Units {
 public:
  typedef std::map<std::string, std::pair<double, Position> > Container;
  typedef Container::mapped_type ValueWithPosition;

  bool Add(const std::string& name, double value);
  bool Add(const std::string& name, double value, const Position& pos);

  bool Find(const std::string& name, ValueWithPosition& result) const;
  bool Find(const std::string& name, double& result) const;
  std::string ToString() const;
 private:
  Container units_;
};

// ============================================================================
}  /* Gaudi */ }  /* Parsers */
// ============================================================================

#endif /* UNITS_H_ */
