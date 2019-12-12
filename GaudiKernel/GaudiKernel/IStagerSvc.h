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
#ifndef INTERFACES_ISTAGERSVC_H
#define INTERFACES_ISTAGERSVC_H

// Include Files
#include "GaudiKernel/IInterface.h"

// Forward declarations
class IIncidentListener;
class Incident;

class GAUDI_API IStagerSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IStagerSvc, 2, 0 );

  /// Retrieve interface ID
  virtual StatusCode getStreams() = 0;
  virtual StatusCode readAhead()  = 0;
  virtual StatusCode initStage()  = 0;
};

#endif // GAUDIKERNEL_STAGERSVC_H
