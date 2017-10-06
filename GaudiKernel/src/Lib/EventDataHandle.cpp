#include <stdexcept>
#include "GaudiKernel/EventDataHandle.h"
#include "GaudiKernel/IDataHandleHolder.h"

namespace Gaudi
{
  namespace experimental
  {
    void EventDataHandle::registerToOwner(IDataHandleHolder& owner,
                                          Mode accessMode)
    {
      switch(accessMode) {
        case Mode::Read:
          owner.registerEventInput(*this);
          break;
        case Mode::Write:
          owner.registerEventOutput(*this);
          break;
        default:
          throw std::runtime_error("Unsupported access mode");
      }
    }

    void EventDataHandle::initialize(const IDataHandleHolder& owner) {
      auto whiteBoard = owner.eventSvc();
      if(!whiteBoard) throw std::runtime_error("Failed to access whiteboard");
      m_whiteBoard = whiteBoard.get();
    }
  }
}
