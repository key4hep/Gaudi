#include "GaudiKernel/Algorithm.h"
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

  namespace experimental {
    void DataHandle::initialize() {
      auto whiteBoard = m_owner.get().eventSvc();
      if(!whiteBoard) throw std::runtime_error("Failed to access whiteboard");
      m_whiteBoard = whiteBoard.get();
    }
  }
}
