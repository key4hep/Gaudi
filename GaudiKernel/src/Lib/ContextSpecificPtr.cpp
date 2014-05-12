#include "GaudiKernel/ContextSpecificPtr.h"
#include "GaudiKernel/ThreadLocalPtr.h"


static THREAD_LOCAL_PTR Gaudi::Hive::ContextIdType s_currentContextId(0);

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
