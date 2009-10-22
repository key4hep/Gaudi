#ifndef GAUDIKERNEL_EXTEND_INTERFACES_H
#define GAUDIKERNEL_EXTEND_INTERFACES_H

#include "GaudiKernel/IInterface.h"

// -----------------------------------------------------------------------------
// Interface extension classes
// -----------------------------------------------------------------------------
/// Base class to be used to extend an interface.
/// Version for one base interface.
/// @author Marco Clemencic
template <typename I1>
struct GAUDI_API extend_interfaces1: virtual public I1 {
#ifndef __GCCXML__
  /// MPL set of interfaces extended by this one.
  typedef typename I1::iid::iids::type ext_iids;
#endif
  /// Virtual destructor
  virtual ~extend_interfaces1() {}
};

/// Base class to be used to extend an interface.
/// Version for two base interfaces.
/// @author Marco Clemencic
template <typename I1, typename I2>
struct GAUDI_API extend_interfaces2: virtual public I1, virtual public I2 {
#ifndef __GCCXML__
  /// MPL set of interfaces extended by this one.
  typedef typename mpl::fold<
    typename I1::iid::iids::type,
    typename I2::iid::iids::type,
    mpl::insert<mpl::_1,mpl::_2>
  >::type ext_iids;
#endif
  /// Virtual destructor
  virtual ~extend_interfaces2() {}
};

/// Base class to be used to extend an interface.
/// Version for three base interfaces.
/// @author Marco Clemencic
template <typename I1, typename I2, typename I3>
struct GAUDI_API extend_interfaces3: virtual public I1,
                                     virtual public I2,
                                     virtual public I3 {
#ifndef __GCCXML__
  /// MPL set of interfaces extended by this one.
  typedef typename mpl::fold<
    typename I1::iid::iids::type,
    typename mpl::fold<
        typename I2::iid::iids::type,
        typename I3::iid::iids::type,
        mpl::insert<mpl::_1,mpl::_2>
      >::type,
    mpl::insert<mpl::_1,mpl::_2>
  >::type ext_iids;
#endif
  /// Virtual destructor
  virtual ~extend_interfaces3() {}
};

/// Base class to be used to extend an interface.
/// Version for four base interfaces.
/// @author Marco Clemencic
template <typename I1, typename I2, typename I3, typename I4>
struct GAUDI_API extend_interfaces4: virtual public I1,
                                     virtual public I2,
                                     virtual public I3,
                                     virtual public I4 {
#ifndef __GCCXML__
  /// MPL set of interfaces extended by this one.
  typedef typename mpl::fold<
    typename I1::iid::iids::type,
    typename mpl::fold<
      typename I2::iid::iids::type,
      typename mpl::fold<
        typename I3::iid::iids::type,
        typename I4::iid::iids::type,
        mpl::insert<mpl::_1,mpl::_2>
      >::type,
      mpl::insert<mpl::_1,mpl::_2>
    >::type,
    mpl::insert<mpl::_1,mpl::_2>
  >::type ext_iids;
#endif
  /// Virtual destructor
  virtual ~extend_interfaces4() {}
};


#endif
