// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IIncidentSvc.h,v 1.5 2007/05/24 17:04:41 marcocle Exp $
#ifndef INTERFACES_IINCIDENTSVC_H
#define INTERFACES_IINCIDENTSVC_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include <string>

// Forward declarations
class IIncidentListener;
class Incident;

/** @class IIncidentSvc IIncidentSvc.h GaudiKernel/IIncidentSvc.h

    The interface implemented by the IncidentSvc service. It is used by
    Listeners to declare themselves that they are interested by Incidents
    of a certain type and by Incident originators.

    @author Pere Mato
*/
class GAUDI_API IIncidentSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IIncidentSvc,2,0);

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

  /** Fire an Incident
      @param Incident being fired
  */
  virtual void fireIncident( const Incident& incident ) = 0;

};

#endif // GAUDIKERNEL_IINCIDENTSVC_H
