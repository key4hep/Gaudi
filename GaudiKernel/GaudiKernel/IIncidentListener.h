#ifndef GAUDIKERNEL_IINCIDENTLISTENER_H
#define GAUDIKERNEL_IINCIDENTLISTENER_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/Incident.h"
#include <string>

class EventContext;

/** @class IIncidentListener IIncidentListener.h GaudiKernel/IIncidentListener.h
    The interface implemented by any class wanting to listen to Incidents.
    @author Pere Mato
*/
class GAUDI_API IIncidentListener : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IIncidentListener, 2, 0 );

  /// Inform that a new incident has occurred
  virtual void handle( const Incident& ) = 0;
};

#endif // GAUDIKERNEL_IINCIDENTLISTENER_H
