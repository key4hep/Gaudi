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
#ifndef GAUDIKERNEL_IOPAQUEADDRESS_H
#define GAUDIKERNEL_IOPAQUEADDRESS_H

// STL include files
#include <string>

// Experiment specific include files
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/Kernel.h"

// forward declarations
class IRegistry;

/** @class IOpaqueAddress IOpaqueAddress.h GaudiKernel/IOpaqueAddress.h

    Opaque address interface definition.
    The opaque address defines the behaviour necesary to load
    and store transient objects.

    @author M.Frank
    @version 1.0
*/
class GAUDI_API IOpaqueAddress {
public:
  /// destructor
  virtual ~IOpaqueAddress() = default;

  /// Add reference to object
  virtual unsigned long addRef() = 0;
  /// release reference to object
  virtual unsigned long release() = 0;
  /// Retrieve class information from link
  virtual const CLID& clID() const = 0;
  /// Retrieve service type
  virtual long svcType() const = 0;
  /// Update branch name
  virtual IRegistry* registry() const = 0;
  /// Update directory pointer
  virtual void setRegistry( IRegistry* r ) = 0;
  /// Retrieve String parameters
  virtual const std::string* par() const = 0;
  /// Access to generic link parameters
  virtual const unsigned long* ipar() const = 0;
};
#endif // GAUDIKERNEL_IOPAQUEADDRESS_H
