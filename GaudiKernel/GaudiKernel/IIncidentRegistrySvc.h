#ifndef INTERFACES_IINCIDENTREGISTRYSVC_H
#define INTERFACES_IINCIDENTREGISTRYSVC_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/EventContextHash.h"
#include <string>
#include <vector>
#include <memory>
#include "tbb/concurrent_queue.h"

// Forward declarations
class IIncidentListener;
class Incident;


/** @class IIncidentRegistrySvc IIncidentRegistrySvc.h GaudiKernel/IIncidentRegistrySvc.h

    The modified IIncidentSvc api to match task based multithreaded
    incident processing . It is used by Listeners to declare
    themselves that they are interested by Incidents of a certain type
    and by Incident originators. Producers fire incidents in the specified event context
    IncidentProcAlgs consume the produced incidents and call listeners

    @author Sami Kama
*/

class GAUDI_API IIncidentRegistrySvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IIncidentRegistrySvc,1,0);

  /** Add listener
      @param lis Listener address
      @param type Incident type
      @param priority  Priority in handling incident
  */
  virtual void addListener(IIncidentListener* lis,
                           const std::string& type = "",
                           long priority = 0,
                           bool rethrow = false,
                           bool singleShot = false ) = 0;
  /** Remove listener
      @param lis Listener address
      @param type Incident type
  */
  virtual void removeListener(IIncidentListener* lis,
                              const std::string& type = "" ) = 0;

  //@TODO: return vector by value instead...
  virtual void getListeners(std::vector<IIncidentListener*>& lis,
                            const std::string& type = "",
			    const EventContext *ctx=0) const = 0;

  class IncidentPack{
  public:
    std::vector<std::unique_ptr<Incident>> incidents;
    std::vector<tbb::concurrent_queue<IIncidentListener*>> listeners;
    IncidentPack(IncidentPack&& o):incidents(std::move(o.incidents)),listeners(std::move(o.listeners)){};
    IncidentPack(){};
  };
  /**
     Get next incident
   */
  virtual IIncidentRegistrySvc::IncidentPack getNextIncident(const EventContext* ctx)=0;
  /** Fire an Incident, Incident ownership has to be passed to the
      service since it is going to be accessed asynchronously 
      @param Incident being fired
  */
  virtual void fireIncident( std::unique_ptr<Incident> incident,
			     const EventContext* ctx ) = 0;
  
};

#endif // GAUDIKERNEL_IINCIDENTREGISTRYSVC_H
