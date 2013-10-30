#include "GaudiKernel/ContextSpecificPtr.h"

static thread_local Gaudi::Hive::ContextIdType s_currentContextId(0);

namespace Gaudi {
  namespace Hive {
    ContextIdType currentContextId() {
      return s_currentContextId;
    }
    void setCurrentContextId(ContextIdType newId) {
      s_currentContextId = newId;
    }

  }
}
