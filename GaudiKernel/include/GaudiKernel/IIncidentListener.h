/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/Incident.h>
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
