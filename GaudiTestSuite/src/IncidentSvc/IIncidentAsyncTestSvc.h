/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIEXAMPLES_IINCIDENTASYNCTESTSVC_H_
#define GAUDIEXAMPLES_IINCIDENTASYNCTESTSVC_H_

#include <GaudiKernel/IInterface.h>
#include <mutex>

class EventContext;
/** @class IncidentRegistryTestListener IncidentListenerTest.h
 *
 */
class GAUDI_API IIncidentAsyncTestSvc : virtual public IInterface {

public:
  DeclareInterfaceID( IIncidentAsyncTestSvc, 1, 0 );
  virtual void getData( uint64_t* data, EventContext* ctx = 0 ) const = 0;
};

#endif /*GAUDIEXAMPLES_IINCIDENTASYNCTESTSVC_H_*/
