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
#ifndef INTERFACES_IINCIDENTSVC_H
#define INTERFACES_IINCIDENTSVC_H

// Include Files
#include "GaudiKernel/EventContextHash.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/Incident.h"
#include <memory>
#include <string>
#include <vector>
// Forward declarations
class IIncidentListener;
// class Incident;

/** @class IIncidentSvc IIncidentSvc.h GaudiKernel/IIncidentSvc.h

    The interface implemented by the IncidentSvc service. It is used by
    Listeners to declare themselves that they are interested by Incidents
    of a certain type and by Incident originators.

    @author Pere Mato
*/
class GAUDI_API IIncidentSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IIncidentSvc, 2, 0 );

  /** Add listener
      @param lis Listener address
      @param type Incident type
      @param priority  Priority in handling incident
  */
  virtual void addListener( IIncidentListener* lis, const std::string& type = "", long priority = 0,
                            bool rethrow = false, bool singleShot = false ) = 0;
  /** Remove listener
      @param lis Listener address
      @param type Incident type
  */
  virtual void removeListener( IIncidentListener* lis, const std::string& type = "" ) = 0;

  //@TODO: return vector by value instead...
  virtual void getListeners( std::vector<IIncidentListener*>& lis, const std::string& type = "" ) const = 0;

  /** Fire an Incident
      @param Incident being fired
  */
  virtual void fireIncident( const Incident& incident ) = 0;

  class IncidentPack {
  public:
    std::vector<std::unique_ptr<Incident>>       incidents;
    std::vector<std::vector<IIncidentListener*>> listeners;
    IncidentPack( IncidentPack&& o ) : incidents( std::move( o.incidents ) ), listeners( std::move( o.listeners ) ){};
    IncidentPack& operator=( IncidentPack&& o ) {
      incidents = std::move( o.incidents );
      listeners = std::move( o.listeners );
      return *this;
    };
    IncidentPack(){};
  };

  virtual IIncidentSvc::IncidentPack getIncidents( const EventContext* ctx ) = 0;
  /** Fire an Incident, Incident ownership has to be passed to the
      service since it is going to be accessed asynchronously
      @param Incident being fired
  */
  virtual void fireIncident( std::unique_ptr<Incident> incident ) = 0;
};

#endif // GAUDIKERNEL_IINCIDENTSVC_H
