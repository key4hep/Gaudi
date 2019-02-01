#ifndef GAUDIKERNEL_THREADLOCALCONTEXT_H
#define GAUDIKERNEL_THREADLOCALCONTEXT_H 1

#include <cstddef>

#include "GaudiKernel/Kernel.h"

class EventContext;

namespace Gaudi {
  namespace Hive {
    /// Internal type used for the ContextId.
    typedef std::size_t ContextIdType;

    /// Return the current context id.
    /// The returned id is valid only within the (sys)Execute method of
    /// algorithms.
    GAUDI_API ContextIdType currentContextId();
    GAUDI_API ContextIdType currentContextEvt();
    GAUDI_API const EventContext& currentContext();

    /// Used by the framework to change the value of the current context id.
    GAUDI_API void setCurrentContextId( ContextIdType newId );
    GAUDI_API void setCurrentContextEvt( long int evtN );
    GAUDI_API void setCurrentContextId( ContextIdType newId, long int evtN );
    GAUDI_API void setCurrentContextId( const EventContext* ctx );
    GAUDI_API void setCurrentContext( const EventContext* ctx );
    GAUDI_API void setCurrentContext( const EventContext& ctx );
  } // namespace Hive
} // namespace Gaudi

#endif
