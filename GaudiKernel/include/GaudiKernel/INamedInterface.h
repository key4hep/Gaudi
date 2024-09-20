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
#ifndef GAUDIKERNEL_INAMEDINTERFACE_H
#define GAUDIKERNEL_INAMEDINTERFACE_H

// Include files
#include <GaudiKernel/IInterface.h>
#include <string>

/**
 * @class INamedInterface INamedInterface.h GaudiKernel/INamedInterface.h
 *
 * IInterface compliant class extending IInterface with the name() method.
 *
 * @author Charles Leggett
 */
class GAUDI_API INamedInterface : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( INamedInterface, 1, 0 );

  /// Retrieve the name of the instance.
  virtual const std::string& name() const = 0;
};

#endif
