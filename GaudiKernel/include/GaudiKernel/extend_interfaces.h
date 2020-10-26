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
#ifndef GAUDIKERNEL_EXTEND_INTERFACES_H
#define GAUDIKERNEL_EXTEND_INTERFACES_H

template <typename... Interfaces>
struct extend_interfaces;

#include "GaudiKernel/IInterface.h"

// -----------------------------------------------------------------------------
// Interface extension classes
// -----------------------------------------------------------------------------
/// Base class to be used to extend an interface.
template <typename... Interfaces>
struct GAUDI_API extend_interfaces : virtual public Interfaces... {
  /// take union of the ext_iids of all Interfaces...
  using ext_iids = typename Gaudi::interface_list_cat<typename Interfaces::ext_iids...>::type;
};

template <typename I1>
using extend_interfaces1 = extend_interfaces<I1>;
template <typename I1, typename I2>
using extend_interfaces2 = extend_interfaces<I1, I2>;
template <typename I1, typename I2, typename I3>
using extend_interfaces3 = extend_interfaces<I1, I2, I3>;
template <typename I1, typename I2, typename I3, typename I4>
using extend_interfaces4 = extend_interfaces<I1, I2, I3, I4>;

#endif
