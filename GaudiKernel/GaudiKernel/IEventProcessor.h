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
#ifndef GAUDIKERNEL_IEVENTPROCESSOR_H
#define GAUDIKERNEL_IEVENTPROCESSOR_H 1

// Include files
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IInterface.h>

/** @class IEventProcessor IEventProcessor.h GaudiKernel/IEventProcessor.h
 *  The IEventProcessor is the interface to process events.
 *
 *  @author Markus Frank
 *   @version 2
 */
class GAUDI_API IEventProcessor : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IEventProcessor, 4, 0 );

  virtual EventContext createEventContext() = 0;
  /// Process single event
  virtual StatusCode executeEvent( EventContext&& ctx ) = 0;
  /// Process the maxevt events as a Run
  virtual StatusCode executeRun( int maxevt ) = 0;
  /// Process the next maxevt events
  virtual StatusCode nextEvent( int maxevt ) = 0;
  /// Schedule a stop of the current event processing
  virtual StatusCode stopRun() = 0;
};

#endif // GAUDIKERNEL_IEVENTPROCESSOR_H
