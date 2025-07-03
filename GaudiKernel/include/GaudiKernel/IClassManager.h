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
#include <string>

class ISvcLocator;

/** @class IClassManager IClassManager.h GaudiKernel/IClassManager.h

    The IClassManager is the interface implemented by the generic Factory in the
    Application Manager to support class management functions.

    @author Pere Mato
*/
class GAUDI_API IClassManager : virtual public IInterface {
public:
  DeclareInterfaceID( IClassManager, 2, 0 );

  /// Declare a sharable library to be used for creating instances of a given algorithm type.
  virtual StatusCode loadModule( const std::string& module, bool fireIncident = true ) = 0; // Sharable library name
};
