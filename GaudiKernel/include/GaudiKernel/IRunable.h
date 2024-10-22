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
#ifndef GAUDIKERNEL_IRUNNABLE_H
#define GAUDIKERNEL_IRUNNABLE_H

#include <GaudiKernel/IInterface.h>

/** @class IRunable IRunable.h GaudiKernel/IRunable.h

    Runable interface definition. Package GaudiKernel

    Abstract interface of an object to be "run".
    Best analogy: java.thread.Thread.run().
    A known user is the application manager, which
    gives control to a runable for execution of the event loop.

   @author Markus Frank
   @version 2.0
*/
class GAUDI_API IRunable : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IRunable, 2, 0 );

  /// Run the class implementation
  virtual StatusCode run() = 0;
};

#endif // GAUDIKERNEL_IRUNNABLE_H
