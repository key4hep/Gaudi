#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/IDataHandleHolder.h"

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
    void DataHandle::registerToOwner(IDataHandleHolder& owner,
                                     Mode accessMode,
                                     DataStore /* store */)
    {
      switch(accessMode) {
        case Mode::Read:
          owner.registerInput(*this);
          break;
        case Mode::Write:
          owner.registerOutput(*this);
          break;
        default:
          throw std::runtime_error("Unsupported access mode");
      }
    }

    void DataHandle::initialize(const IDataHandleHolder& owner) {
      auto whiteBoard = owner.eventSvc();
      if(!whiteBoard) throw std::runtime_error("Failed to access whiteboard");
      m_whiteBoard = whiteBoard.get();
    }
  }
}
