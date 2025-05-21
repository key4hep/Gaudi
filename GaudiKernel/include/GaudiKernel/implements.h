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
#include <atomic>
#include <cassert>

/// Base class used to implement the interfaces.
template <typename... Interfaces>
struct GAUDI_API implements : virtual public extend_interfaces<Interfaces...> {
  /// Typedef to this class.
  using base_class = implements<Interfaces...>;
  /// Typedef to the base of this class.
  using extend_interfaces_base = extend_interfaces<Interfaces...>;
  using iids                   = typename extend_interfaces_base::ext_iids;

public:
  /**Implementation of IInterface::i_cast. */
  void const* i_cast( const InterfaceID& tid ) const override { return Gaudi::iid_cast( tid, iids{}, this ); }
  /** Implementation of IInterface::queryInterface. */
  StatusCode queryInterface( const InterfaceID& ti, void** pp ) override {
    if ( !pp ) return StatusCode::FAILURE;
    *pp = Gaudi::iid_cast( ti, iids{}, this );
    if ( !*pp ) return StatusCode::FAILURE; /* cast failed */
    this->addRef();
    return StatusCode::SUCCESS;
  }
  /** Implementation of IInterface::getInterfaceNames. */
  std::vector<std::string> getInterfaceNames() const override { return Gaudi::getInterfaceNames( iids{} ); }
  /** Default constructor */
  implements() = default;
  /** Copy constructor (zero the reference count) */
  implements( const implements& /*other*/ ) : m_refCount{ 0 } {}
  /** Assignment operator (do not touch the reference count).*/
  implements& operator=( const implements& /*other*/ ) { return *this; } // cppcheck-suppress operatorEqVarError

public:
  /** Reference Interface instance               */
  unsigned long addRef() const override { return ++m_refCount; }
  /** Release Interface instance                 */
  unsigned long release() const override {
    if ( auto count = decRef() ) {
      return count;
    } else {
      delete this;
      return 0;
    }
  }
  /** Current reference count                    */
  unsigned long refCount() const override { return m_refCount.load(); }

protected:
  unsigned long decRef() const override {
    auto count = m_refCount.load();
    // thread-safe decrement, but make sure we don't go below 0
    // (if the last two references are being released at the same time, this guarantees that
    // one decrements m_refCount from 2 to 1 and the other from 1 to 0)
    while ( count > 0 && !m_refCount.compare_exchange_weak( count, count - 1 ) ) { assert( count > 0 ); }
    // when we reach this point, we managed to set m_refCount to "count - 1", so no need to read it again
    return count - 1;
  }

  /** Reference counter                          */
  mutable std::atomic_ulong m_refCount = { 0 };
};

template <typename I1>
using implements1 = implements<I1>;
template <typename I1, typename I2>
using implements2 = implements<I1, I2>;
template <typename I1, typename I2, typename I3>
using implements3 = implements<I1, I2, I3>;
template <typename I1, typename I2, typename I3, typename I4>
using implements4 = implements<I1, I2, I3, I4>;
