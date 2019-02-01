#ifndef GAUDIKERNEL_IMPLEMENTS_H
#define GAUDIKERNEL_IMPLEMENTS_H

#include "GaudiKernel/IInterface.h"
#include <atomic>

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
  void* i_cast( const InterfaceID& tid ) const override { return Gaudi::iid_cast( tid, iids{}, this ); }
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
  implements( const implements& /*other*/ ) : m_refCount{0} {}
  /** Assignment operator (do not touch the reference count).*/
  implements& operator=( const implements& /*other*/ ) { return *this; }

public:
  /** Reference Interface instance               */
  unsigned long addRef() override { return ++m_refCount; }
  /** Release Interface instance                 */
  unsigned long release() override {
    /* Avoid to decrement 0 */
    auto count = ( m_refCount ? --m_refCount : m_refCount.load() );
    if ( count == 0 ) delete this;
    return count;
  }
  /** Current reference count                    */
  unsigned long refCount() const override { return m_refCount.load(); }

protected:
  /** Reference counter                          */
  std::atomic_ulong m_refCount = {0};
};

template <typename I1>
using implements1 = implements<I1>;
template <typename I1, typename I2>
using implements2 = implements<I1, I2>;
template <typename I1, typename I2, typename I3>
using implements3 = implements<I1, I2, I3>;
template <typename I1, typename I2, typename I3, typename I4>
using implements4 = implements<I1, I2, I3, I4>;

#endif /* GAUDIKERNEL_IMPLEMENTS_H_ */
