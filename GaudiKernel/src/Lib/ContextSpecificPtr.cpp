#include "GaudiKernel/ContextSpecificPtr.h"
#include "GaudiKernel/ThreadLocalPtr.h"
#include "GaudiKernel/EventContext.h"



static THREAD_LOCAL_PTR Gaudi::Hive::ContextIdType s_currentContextId(0);
static THREAD_LOCAL_PTR long int s_currentContextEvt(-1);

namespace Gaudi {
  namespace Hive {
    ContextIdType currentContextId() {
      return s_currentContextId;
    }
    ContextIdType currentContextEvt() {
      return s_currentContextEvt;
    }
    void setCurrentContextEvt(long int evtN) {
      s_currentContextEvt = evtN;
    }
    void setCurrentContextId(ContextIdType newId) {
      s_currentContextId = newId;
    }
    void setCurrentContextId(ContextIdType newId, long int evtN) {
      s_currentContextId  = newId;
      s_currentContextEvt = evtN;
    }

    void setCurrentContextId(const EventContext* ctx) {
      if (ctx != 0) {
	s_currentContextId = ctx->slot();
	s_currentContextEvt = ctx->evt();
      } else {
	s_currentContextId = 0;
	s_currentContextEvt = -1;
      }
    }

  }
}
