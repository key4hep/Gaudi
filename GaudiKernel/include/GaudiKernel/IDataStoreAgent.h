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

#include <GaudiKernel/Kernel.h>
#include <functional>

class IRegistry;

/** @class IDataStoreAgent IDataStoreAgent.h GaudiKernel/IDataStoreAgent.h

    Generic data agent interface

    @author Markus Frank
*/
class GAUDI_API IDataStoreAgent {
public:
  /// destructor
  virtual ~IDataStoreAgent() = default;

  /** Analyse the data object.
  @return Boolean indicating wether the tree below should be analysed
  */
  virtual bool analyse( IRegistry* pObject, int level ) = 0;
};

namespace details {
  template <typename F>
  class GenericDataStoreAgent final : public IDataStoreAgent {
    F f;

  public:
    template <typename G>
    GenericDataStoreAgent( G&& g ) : f{ std::forward<G>( g ) } {}

    bool analyse( IRegistry* pObj, int level ) override { return std::invoke( f, pObj, level ); }
  };
} // namespace details

template <typename F>
::details::GenericDataStoreAgent<F> makeDataStoreAgent( F&& f ) {
  return { std::forward<F>( f ) };
}
