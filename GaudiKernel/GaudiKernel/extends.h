#ifndef GAUDIKERNEL_EXTENDS_H
#define GAUDIKERNEL_EXTENDS_H

#include "GaudiKernel/IInterface.h"

/// Base class used to extend a class implementing other interfaces.
/// Version for one interface.
/// @author Marco Clemencic
template <typename BASE, typename I1>
struct GAUDI_API extends1: public BASE, virtual public extend_interfaces1<I1> {
  /// Typedef to this class.
  typedef extends1 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces1<I1> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  /// Templated constructor with variadic arguments.
  template <typename... Args> extends1(Args&&... args) : BASE(std::forward<Args>(args)...) {}

  /// Implementation of IInterface::i_cast.
  virtual void *i_cast(const InterfaceID &tid) const {
    void *ptr = nullptr;
#ifndef __GCCXML__
    interfaceMatch<base_class> matcher(this,tid,ptr);
    mpl::for_each<interfaces>(matcher);
#endif
    if (!ptr) {
      ptr = BASE::i_cast(tid);
    }
    return ptr;
  }

  /// Implementation of IInterface::queryInterface.
  virtual StatusCode queryInterface(const InterfaceID &ti, void** pp){
    if (!pp) return StatusCode::FAILURE;
    *pp = 0;
#ifndef __GCCXML__
    interfaceMatch<base_class> matcher(this,ti,*pp);
    mpl::for_each<interfaces>(matcher);
    if (!*pp) { // cast failed, try the base class
      return BASE::queryInterface(ti,pp);
    }
    this->addRef();
#endif
    return StatusCode::SUCCESS;
  }

  /// Implementation of IInterface::getInterfaceNames.
  virtual std::vector<std::string> getInterfaceNames() const {
    std::vector<std::string> v = BASE::getInterfaceNames(); // start from the base
#ifndef __GCCXML__
    AppendInterfaceName appender(v);
    mpl::for_each<interfaces>(appender);
#endif
    return v;
  }

  /// Virtual destructor
  ~extends1() override = default;
};

/// Base class used to extend a class implementing other interfaces.
/// Version for two interfaces.
/// @author Marco Clemencic
template <typename BASE, typename I1, typename I2>
struct GAUDI_API extends2: public BASE, virtual public extend_interfaces2<I1,I2> {
  /// Typedef to this class.
  typedef extends2 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces2<I1,I2> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  /// Templated constructor with variadic arguments.
  template <typename... Args> extends2(Args&&... args) : BASE( std::forward<Args>(args)... ) {}

  /// Implementation of IInterface::i_cast.
  virtual void *i_cast(const InterfaceID &tid) const {
    void *ptr = nullptr;
#ifndef __GCCXML__
    interfaceMatch<base_class> matcher(this,tid,ptr);
    mpl::for_each<interfaces>(matcher);
#endif
    if (!ptr) {
      ptr = BASE::i_cast(tid);
    }
    return ptr;
  }

  /// Implementation of IInterface::queryInterface.
  virtual StatusCode queryInterface(const InterfaceID &ti, void** pp){
    if (!pp) return StatusCode::FAILURE;
    *pp = 0;
#ifndef __GCCXML__
    interfaceMatch<base_class> matcher(this,ti,*pp);
    mpl::for_each<interfaces>(matcher);
    if (!*pp) { // cast failed, try the base class
      return BASE::queryInterface(ti,pp);
    }
    this->addRef();
#endif
    return StatusCode::SUCCESS;
  }

  /// Implementation of IInterface::getInterfaceNames.
  virtual std::vector<std::string> getInterfaceNames() const {
    std::vector<std::string> v = BASE::getInterfaceNames(); // start from the base
#ifndef __GCCXML__
    AppendInterfaceName appender(v);
    mpl::for_each<interfaces>(appender);
#endif
    return v;
  }

  /// Virtual destructor
  ~extends2() override = default;
};

/// Base class used to extend a class implementing other interfaces.
/// Version for three interfaces.
/// @author Marco Clemencic
template <typename BASE, typename I1, typename I2, typename I3>
struct GAUDI_API extends3: public BASE, virtual public extend_interfaces3<I1,I2,I3> {
  /// Typedef to this class.
  typedef extends3 base_class;
  /// Typedef to the base of this class.
  typedef extend_interfaces3<I1,I2,I3> extend_interfaces_base;
#ifndef __GCCXML__
  /// MPL set of all the implemented interfaces.
  typedef typename extend_interfaces_base::ext_iids interfaces;
#endif

  /// Templated constructor with variadic arguments.
  template <typename... Args> extends3(Args&&... args) : BASE( std::forward<Args>(args)... ) {}

  /// Implementation of IInterface::i_cast.
  virtual void *i_cast(const InterfaceID &tid) const {
    void *ptr = nullptr;
#ifndef __GCCXML__
    interfaceMatch<base_class> matcher(this,tid,ptr);
    mpl::for_each<interfaces>(matcher);
#endif
    if (!ptr) {
      ptr = BASE::i_cast(tid);
    }
    return ptr;
  }

  /// Implementation of IInterface::queryInterface.
  virtual StatusCode queryInterface(const InterfaceID &ti, void** pp){
    if (!pp) return StatusCode::FAILURE;
    *pp = 0;
#ifndef __GCCXML__
    interfaceMatch<base_class> matcher(this,ti,*pp);
    mpl::for_each<interfaces>(matcher);
    if (!*pp) { // cast failed, try the base class
      return BASE::queryInterface(ti,pp);
    }
    this->addRef();
#endif
    return StatusCode::SUCCESS;
  }

  /// Implementation of IInterface::getInterfaceNames.
  virtual std::vector<std::string> getInterfaceNames() const {
    std::vector<std::string> v = BASE::getInterfaceNames(); // start from the base
#ifndef __GCCXML__
    AppendInterfaceName appender(v);
    mpl::for_each<interfaces>(appender);
#endif
    return v;
  }

  /// Virtual destructor
  ~extends3() override = default;
};

#endif /* GAUDIKERNEL_EXTENDS_H */
