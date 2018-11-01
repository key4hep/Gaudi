#include "GaudiKernel/DataHandle.h"

#include <sstream>

using namespace Gaudi;

namespace Gaudi
{
  std::string DataHandle::pythonRepr() const
  {
    std::ostringstream ost;
    ost << "DataHandle(\"" << fullKey() << "||" << std::to_string( mode() ) << "\")";
    return ost.str();
  }
}
