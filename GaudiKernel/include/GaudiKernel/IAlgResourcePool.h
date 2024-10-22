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
#ifndef GAUDIKERNEL_IALGRESOURCEPOOL_H
#define GAUDIKERNEL_IALGRESOURCEPOOL_H

// Framework includes
#include <GaudiKernel/IInterface.h>

// C++ includes
#include <list>
#include <string_view>

// Forward class declaration
class IAlgorithm;

/** @class IAlgResourcePool IAlgResourcePool.h GaudiKernel/IAlgResourcePool.h

    The IAlgResourcePool is the interface for managing algorithm instances,
    in particular if clones of the same algorithm exist.
    It as well manages the shared resources between different algorithms and
    ensures that not two algorithms with the same non-thread-safe resource
    needs are made available. The actual creation and deletion of algorithm
    instances is done through the IAlgManager service.

    @author Benedikt Hegner
    @version 1.0
*/
class GAUDI_API IAlgResourcePool : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IAlgResourcePool, 1, 0 );

  /// Acquire a certain algorithm using its name
  virtual StatusCode acquireAlgorithm( std::string_view name, IAlgorithm*& algo, bool blocking = false ) = 0;
  /// Release a certain algorithm
  virtual StatusCode releaseAlgorithm( std::string_view name, IAlgorithm*& algo ) = 0;

  /// Get the flat list of algorithms
  virtual std::list<IAlgorithm*> getFlatAlgList() = 0;

  /// Get top list of algorithms
  virtual std::list<IAlgorithm*> getTopAlgList() = 0;

  /// Acquire a certain resource
  virtual StatusCode acquireResource( std::string_view name ) = 0;
  /// Release a certrain resource
  virtual StatusCode releaseResource( std::string_view name ) = 0;
};

#endif // GAUDIKERNEL_IALGRESOURCEPOOL_H
