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

// Include files
#include <GaudiKernel/IStateful.h>

/** @class IComponentManager IComponentManager.h GaudiKernel/IComponentManager.h

    @author Marco Clemencic
*/
class GAUDI_API IComponentManager : virtual public IStateful {
public:
  /// InterfaceID
  DeclareInterfaceID( IComponentManager, 1, 0 );

  /// Basic interface id of the managed components.
  virtual const InterfaceID& componentBaseInterface() const = 0;

  /// Function to call to update the outputLevel of the components (after a change in MessageSvc).
  virtual void outputLevelUpdate() = 0;
};
