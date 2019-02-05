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
    using iids = typename extend_interfaces_base::ext_iids;
    void* ptr  = Gaudi::iid_cast( tid, iids{}, this );
    return ptr ? ptr : BASE::i_cast( tid );
  }

  /// Implementation of IInterface::queryInterface.
  StatusCode queryInterface( const InterfaceID& ti, void** pp ) override {
    if ( !pp ) return StatusCode::FAILURE;
    using iids = typename extend_interfaces_base::ext_iids;
    *pp        = Gaudi::iid_cast( ti, iids{}, this );
    // if cast failed, try the base class
    if ( !*pp ) return BASE::queryInterface( ti, pp );
    this->addRef();
    return StatusCode::SUCCESS;
  }

  /// Implementation of IInterface::getInterfaceNames.
  std::vector<std::string> getInterfaceNames() const override {
    using iids = typename extend_interfaces_base::ext_iids;
    auto vb    = BASE::getInterfaceNames();
    auto vi    = Gaudi::getInterfaceNames( iids{} );
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
