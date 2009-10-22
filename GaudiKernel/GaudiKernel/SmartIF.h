// $Id: SmartIF.h,v 1.10 2008/10/27 19:22:20 marcocle Exp $
#ifndef GAUDI_SMARTIF_H
#define GAUDI_SMARTIF_H 1

// Framework include files
#include "GaudiKernel/IInterface.h"

/** @class SmartIF SmartIF.h GaudiKernel/SmartIF.h
 *
 * Small smart pointer class with automatic reference counting for IInterface.
 *
 * SmartIF simplifies the interaction with components in Gaudi by implementing
 * an automatic reference counting and the casting (via IInterface::queryInterface).
 *
 * @author Markus Frank
 * @author Sebastien Ponce
 * @author Marco Clemencic
 */
template <class TYPE> class SmartIF {
private:
  /// Pointer to the instance
  TYPE* m_interface;
public:
  // ---------- Construction and destruction ----------
  /// Default constructor.
  inline SmartIF(): m_interface(0) {}
  /// Standard constructor from pointer.
  inline SmartIF(TYPE* ptr): m_interface(ptr) {
    if (m_interface) m_interface->addRef();
  }
  /// Standard constructor from any (IInterface-derived) pointer.
  template <class OTHER>
  inline SmartIF(OTHER* ptr): m_interface(0) {
    if (ptr) reset(ptr);
  }
  /// Copy constructor.
  inline SmartIF(const SmartIF& rhs): m_interface(rhs.get()) {
    if (m_interface) m_interface->addRef();
  }
  /// Constructor from another SmartIF, with a different type.
  /// @note it cannot replace the copy constructor.
  template <class T>
  inline explicit SmartIF(const SmartIF<T>& rhs): m_interface(0) {
    reset(rhs.get());
  }
  /// Standard Destructor.
  inline ~SmartIF() { reset(); }

  // ---------- Boolean and comparison methods ----------
  /// Allow for check if smart pointer is valid.
  inline bool isValid() const { return m_interface != 0; }

  // ---------- Pointer access methods ----------
  /// Automatic conversion to pointer.
  /// It is also used by the compiler for automatic conversion to boolean.
  inline operator TYPE* () const { return m_interface; }
  /// Dereference operator
  inline TYPE* operator->() const { return m_interface; }
  /// Dereference operator
  inline TYPE& operator *() const { return *m_interface; }
  /// Get interface pointer
  inline TYPE* get() const { return m_interface; }
#if !defined(GAUDI_V22_API) && !defined(NEW_SMARTIF)
  /// Get reference to the pointer
  inline TYPE*& pRef() {
    return m_interface;
  }
#endif

  // ---------- Cast methods ----------
  /// Set the internal pointer to the passed one disposing of the old one.
  /// Version for pointers of the same type of the managed ones (no call to
  /// queryInterface needed).
  inline void reset(TYPE* ptr = 0) {
    if (ptr == m_interface) return;
    if (m_interface) m_interface->release();
    if (ptr) {
      m_interface = ptr;
      m_interface->addRef();
    } else {
      m_interface = 0;
    }
  }
  /// Set the internal pointer to the passed one disposing of the old one.
  /// Version for pointers of types inheriting from IInterface.
  template <class OTHER>
  inline void reset(OTHER* ptr) {
    if (static_cast<IInterface*>(ptr) == static_cast<IInterface*>(m_interface)) return;
    if (m_interface) m_interface->release();
    if (ptr) {
      ptr->queryInterface(TYPE::interfaceID(), pp_cast<void>(&m_interface)).ignore();
    } else {
      m_interface = 0;
    }
  }

  // ---------- Special hacks ----------
  /// Assignment operator from IInterface pointer.
  /// It allows things like
  /// <code>
  /// SmartIF<T> x;
  /// x = 0;
  /// </code>
  inline SmartIF& operator = (IInterface* ptr) {
    reset(ptr);
    return *this;
  }
  /// Assignment operator.
  inline SmartIF& operator = (const SmartIF& rhs) {
    reset(rhs.get());
    return *this;
  }
  /// Assignment operator from a different SmartIF.
  /// @note it cannot replace the assignment operator.
  template <class T>
  inline SmartIF& operator = (const SmartIF<T>& rhs) {
    reset(rhs.get());
    return *this;
  }
};

#if defined(_MSC_VER) && (_MSC_VER < 1500)
/// Ugly hack to allow a check like
/// <code>
///  0 == smartIf;
/// </code>
/// To work on Windows (VisualC 7.1).
/// Actually, VC7 will user operator==(TYPE*,TYPE*) when the left operand is a pointer,
/// but it does not understands that 0 should be interpreted as (TYPE*)0.
template <class TYPE>
inline bool operator == (long lhs, const SmartIF<TYPE> &rhs) {
  return rhs == reinterpret_cast<TYPE*>(lhs);
}
/// Ugly hack to allow a check like
/// <code>
///  0 != smartIf;
/// </code>
/// To work on Windows (VisualC 7.1).
/// Actually, VC7 will user operator!=(TYPE*,TYPE*) when the left operand is a pointer,
/// but it does not understands that 0 should be interpreted as (TYPE*)0.
template <class TYPE>
inline bool operator != (long lhs, const SmartIF<TYPE> &rhs) {
  return rhs != reinterpret_cast<TYPE*>(lhs);
}
#endif
#endif // GAUDI_SMARTIF_H
