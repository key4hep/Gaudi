/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/EventContext.h"

#include "Rtypes.h"
#include "ThreadLocalStorage.h"

namespace {
  // MacOS X's clang doesn't provide thread_local. So we need to use ROOT's
  // thread-local implementation to operate on this platform.
  EventContext& s_curCtx() {
    TTHREAD_TLS_DECL( EventContext, localContext );
    return localContext;
  }
} // namespace

namespace Gaudi {
  namespace Hive {
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
  } // namespace Hive
} // namespace Gaudi
