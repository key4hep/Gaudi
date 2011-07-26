// $Id:$
#ifndef JOBOPTIONSVC_INCLUDED_FILES_H_
#define JOBOPTIONSVC_INCLUDED_FILES_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <map>
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class Position;
// ============================================================================
class IncludedFiles {
 public:
  typedef std::map<std::string, Position> Container;
 public:
  bool AddFile(const std::string& filename, const Position& from);
  bool IsIncluded(const std::string& filename) const;
  bool GetPosition(const std::string& filename, const Position** pos) const;
 private:
  Container container_;
};
// ============================================================================
}  /* Gaudi */ } /* Parsers */
// ============================================================================
#endif  //  JOBOPTIONSVC_INCLUDED_FILES_H_
