#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/EventContext.h"

#include "Rtypes.h"
#include "ThreadLocalStorage.h"

namespace
{
  // MacOS X's clang doesn't provide thread_local. So we need to use ROOT's
  // thread-local implementation to operate on this platform.
  EventContext& s_curCtx()
  {
    TTHREAD_TLS_DECL( EventContext, localContext );
    return localContext;
  }
}

namespace Gaudi
{
  namespace Hive
  {
    ContextIdType       currentContextId() { return s_curCtx().slot(); }
    ContextIdType       currentContextEvt() { return s_curCtx().evt(); }
    const EventContext& currentContext() { return s_curCtx(); }

    void setCurrentContextEvt( long int evtN ) { s_curCtx().setEvt( evtN ); }
    void setCurrentContextId( ContextIdType newId ) { s_curCtx().setSlot( newId ); }

    // FIXME: do we need this method?
    void setCurrentContextId( ContextIdType newId, long int evtN ) { s_curCtx().set( evtN, newId ); }

    // FIXME: do we need this method?
    void setCurrentContextId( const EventContext* ctx ) { s_curCtx() = *ctx; }

    void setCurrentContext( const EventContext* ctx ) { s_curCtx() = *ctx; }

    void setCurrentContext( const EventContext& ctx ) { s_curCtx() = ctx; }
  }
}
