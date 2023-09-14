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
#ifndef GAUDI_OBJECTCONTAINERBASE_H
#define GAUDI_OBJECTCONTAINERBASE_H 1

// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/Kernel.h"

// Forward declarations
class ContainedObject;

/** @class ObjectContainerBase ObjectContainerBase.h GaudiKernel/ObjectContainerBase.h
    ObjectContainerBase is the base class for Gaudi container classes. The main motivation
    is to allow contained object to be removed from the container on deletion and also for
    knowing it own index (e.g. distance, key, ...) in the container.

    @author Pavel Binko
    @author Pere Mato
*/
class GAUDI_API ObjectContainerBase : public DataObject {

protected:
  /// Constructor
  ObjectContainerBase()                                   = default;
  ObjectContainerBase( ObjectContainerBase&& )            = default;
  ObjectContainerBase& operator=( ObjectContainerBase&& ) = default;
  ~ObjectContainerBase() override                         = default;

public:
  /// size_type, to conform the STL container interface
  typedef size_t size_type;

  /// Distance of a given object from the beginning of its container
  virtual long index( const ContainedObject* obj ) const = 0;

  /// Pointer to an object of a given distance
  virtual const ContainedObject* containedObject( long dist ) const = 0;
  virtual ContainedObject*       containedObject( long dist )       = 0;

  /// Number of objects in the container
  virtual size_type numberOfObjects() const = 0;

  /** Virtual functions (forwards to the concrete container definitions)
      Add an object to the container. On success the object's index is
      returned.                                                      */
  virtual long add( ContainedObject* pObject ) = 0;

  /** Release object from the container (the pointer will be removed
      from the container, but the object itself will remain alive).
      If the object was fount it's index is returned.                */
  virtual long remove( ContainedObject* value ) = 0;
};

#endif // GAUDI_OBJECTCONTAINERBASE_H
