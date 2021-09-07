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
#ifndef GAUDIKERNEL_EXTENDS_H
#define GAUDIKERNEL_EXTENDS_H

#include "GaudiKernel/IInterface.h"

/// Base class used to extend a class implementing other interfaces.
/// Version for one interface.
/// @author Marco Clemencic
template <typename BASE, typename... Interfaces>
class GAUDI_API extends : public BASE, virtual public extend_interfaces<Interfaces...> {

public:
  /// Typedef to this class.
  using base_class = extends;
  /// Typedef to the base of this class.
  using extend_interfaces_base = extend_interfaces<Interfaces...>;
  /// forward constructor(s)
  using BASE::BASE;

  /// Implementation of IInterface::i_cast.
  void* i_cast( const InterfaceID& tid ) const override {
    using iids_ = typename extend_interfaces_base::ext_iids;
    void* ptr   = Gaudi::iid_cast( tid, iids_{}, this );
    return ptr ? ptr : BASE::i_cast( tid );
  }

  /// Implementation of IInterface::queryInterface.
  StatusCode queryInterface( const InterfaceID& ti, void** pp ) override {
    if ( !pp ) return StatusCode::FAILURE;
    using iids_ = typename extend_interfaces_base::ext_iids;
    *pp         = Gaudi::iid_cast( ti, iids_{}, this );
    // if cast failed, try the base class
    if ( !*pp ) return BASE::queryInterface( ti, pp );
    this->addRef();
    return StatusCode::SUCCESS;
  }

  /// Implementation of IInterface::getInterfaceNames.
  std::vector<std::string> getInterfaceNames() const override {
    using iids_ = typename extend_interfaces_base::ext_iids;
    auto vb     = BASE::getInterfaceNames();
    auto vi     = Gaudi::getInterfaceNames( iids_{} );
    // start with base, and move the rest...
    vb.insert( vb.end(), std::make_move_iterator( vi.begin() ), std::make_move_iterator( vi.end() ) );
    return vb;
  }
};

template <typename BASE, typename I1>
using extends1 = extends<BASE, I1>;
template <typename BASE, typename I1, typename I2>
using extends2 = extends<BASE, I1, I2>;
template <typename BASE, typename I1, typename I2, typename I3>
using extends3 = extends<BASE, I1, I2, I3>;
template <typename BASE, typename I1, typename I2, typename I3, typename I4>
using extends4 = extends<BASE, I1, I2, I3, I4>;

#endif /* GAUDIKERNEL_EXTENDS_H */
