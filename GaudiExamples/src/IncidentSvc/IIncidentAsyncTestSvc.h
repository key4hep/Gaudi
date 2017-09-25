#ifndef GAUDIEXAMPLES_IINCIDENTASYNCTESTSVC_H_
#define GAUDIEXAMPLES_IINCIDENTASYNCTESTSVC_H_

#include "GaudiKernel/IInterface.h"
#include <mutex>

class EventContext;
/** @class IncidentRegistryTestListener IncidentListenerTest.h
 *
 */
class GAUDI_API IIncidentAsyncTestSvc : virtual public IInterface
{

public:
  DeclareInterfaceID( IIncidentAsyncTestSvc, 1, 0 );
  virtual void getData( uint64_t* data, EventContext* ctx = 0 ) const = 0;
};

#endif /*GAUDIEXAMPLES_IINCIDENTASYNCTESTSVC_H_*/
