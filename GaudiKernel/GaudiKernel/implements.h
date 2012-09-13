#ifndef GAUDIKERNEL_IMPLEMENTS_H
#define GAUDIKERNEL_IMPLEMENTS_H

#include "GaudiKernel/IInterface.h"

#ifndef __GCCXML__
/// Helper class for the cast used in the MPL for_each algorithm in the
/// implementation of queryInterface.
/// @author Marco Clemencic
template <typename T>
struct GAUDI_LOCAL interfaceMatch {
  /// InterfaceID for the requested interface.
  const InterfaceID &target;
  /// Pointer to be filled.
  void *&ptr;
  /// Value of this.
  const T *instance;
  interfaceMatch(const T *_instance, const InterfaceID &_tid, void * &_ptr):
    target(_tid),
    ptr(_ptr),
    instance(_instance){}

  template <typename IID>
  inline void operator() (IID) {
    if ((!ptr) && target.versionMatch(IID::interfaceID())) {
      ptr = const_cast<void*>(reinterpret_cast<const void*>(static_cast<const typename IID::iface_type*>(instance)));
    }
  }
};

/// Helper class for the cast used in the MPL for_each algorithm in the implementation of query_interface.
/// @author Marco Clemencic
struct GAUDI_LOCAL AppendInterfaceName {
  /// vector to be filled.
  std::vector<std::string> &v;
  AppendInterfaceName(std::vector<std::string> &_v): v(_v) {}

  template <typename IID>
  inline void operator() (IID) { v.push_back(IID::name()); }
};

#endif

#define _refcounting_implementation_ \
public: \
  /** Reference Interface instance               */ \
  virtual unsigned long addRef() { return ++m_refCount; } \
  /** Release Interface instance                 */ \
  virtual unsigned long release() { \
    /* Avoid to decrement 0 */ \
    const unsigned long count = (m_refCount) ? --m_refCount : m_refCount; \
    if(count == 0) delete this; \
    return count; \
  } \
  /** Current reference count                    */ \
  virtual unsigned long refCount() const { return m_refCount; } \
protected: \
  /** Reference counter                          */ \
  unsigned long m_refCount; \
private:

#ifndef __GCCXML__
#define _helper_common_implementation_(N) \
  public: \
  /**Implementation of IInterface::i_cast. */ \
  virtual void *i_cast(const InterfaceID &tid) const { \
    void *ptr = 0; \
    interfaceMatch<implements##N> matcher(this,tid,ptr); \
    mpl::for_each<interfaces>(matcher); \
    return ptr; \
  } \
  /** Implementation of IInterface::queryInterface. */ \
  virtual StatusCode queryInterface(const InterfaceID &ti, void** pp){ \
    if (!pp) return StatusCode::FAILURE; \
    *pp = 0; \
    interfaceMatch<implements##N> matcher(this,ti,*pp); \
    mpl::for_each<interfaces>(matcher); \
    if (!*pp) { /* cast failed */ \
      return StatusCode::FAILURE; \
    } \
    this->addRef(); \
    return StatusCode::SUCCESS; \
  } \
  /** Implementation of IInterface::getInterfaceNames. */ \
  virtual std::vector<std::string> getInterfaceNames() const { \
    std::vector<std::string> v; /* temporary storage */ \
    AppendInterfaceName appender(v); \
    mpl::for_each<interfaces>(appender); \
    return v; \
  } \
  /** Default constructor */ \
  implements##N():m_refCount(0) {} \
  /** Copy constructor */ \
  implements##N(const implements##N &/*other*/):m_refCount(0) {} \
  /** Assignment operator (do not touch the reference count).*/ \
  implements##N& operator=(const implements##N &/*other*/) { return *this; } \
  /** Virtual destructor */ \
  virtual ~implements##N() {} \
  _refcounting_implementation_
#else
#define _helper_common_implementation_(N) \
  public: \
  /**Implementation of IInterface::i_cast. */ \
  virtual void *i_cast(const InterfaceID &tid) const { \
    return 0; \
  } \
  /** Implementation of IInterface::queryInterface. */ \
  virtual StatusCode queryInterface(const InterfaceID &ti, void** pp){ \
    if (!pp) return StatusCode::FAILURE; \
    *pp = 0; \
    return StatusCode::SUCCESS; \
  } \
  /** Implementation of IInterface::getInterfaceNames. */ \
  virtual std::vector<std::string> getInterfaceNames() const { \
    std::vector<std::string> v; /* temporary storage */ \
    return v; \
  } \
  /** Default constructor */ \
  implements##N():m_refCount(0) {} \
  /** Copy constructor */ \
  implements##N(const implements##N &/*other*/):m_refCount(0) {} \
  /** Assignment operator (do not touch the reference count).*/ \
  implements##N& operator=(const implements##N &/*other*/) { return *this; } \
  /** Virtual destructor */ \
  virtual ~implements##N() {} \
  _refcounting_implementation_
#endif

/// Base class used to implement the interfaces.
/// Version for one interface.
/// @author Marco Clemencic
template <typename I1>
struct GAUDI_API implements1: virtual public extend_interfaces1<I1> {
  /// Typedef to this class.
  typedef implements1 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces1<I1> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  _helper_common_implementation_(1)
};

/// Base class used to implement the interfaces.
/// Version for two interfaces.
/// @author Marco Clemencic
template <typename I1, typename I2>
struct GAUDI_API implements2: virtual public extend_interfaces2<I1,I2> {
  /// Typedef to this class.
  typedef implements2 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces2<I1,I2> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  _helper_common_implementation_(2)
};

/// Base class used to implement the interfaces.
/// Version for three interfaces.
/// @author Marco Clemencic
template <typename I1, typename I2, typename I3>
struct GAUDI_API implements3: virtual public extend_interfaces3<I1,I2,I3> {
  /// Typedef to this class.
  typedef implements3 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces3<I1,I2,I3> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  _helper_common_implementation_(3)
};

/// Base class used to implement the interfaces.
/// Version for four interfaces.
/// @author Marco Clemencic
template <typename I1, typename I2, typename I3, typename I4>
struct GAUDI_API implements4: virtual public extend_interfaces4<I1,I2,I3,I4> {
  /// Typedef to this class.
  typedef implements4 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces4<I1,I2,I3,I4> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  _helper_common_implementation_(4)
};

// Undefine helper macros
#undef _refcounting_implementation_
#undef _helper_common_implementation_

#endif /* GAUDIKERNEL_IMPLEMENTS_H_ */
