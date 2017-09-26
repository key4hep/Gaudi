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
template <class TYPE>
class SmartIF
{
private:
  /// Pointer to the instance
  TYPE* m_interface = nullptr;

public:
  // ---------- Construction and destruction ----------
  /// Default constructor.
  inline SmartIF() = default;
  /// Standard constructor from pointer.
  inline SmartIF( TYPE* ptr ) : m_interface( ptr )
  {
    if ( m_interface ) m_interface->addRef();
  }
  /// Standard constructor from any (IInterface-derived) pointer.
  template <class OTHER>
  inline SmartIF( OTHER* ptr )
  {
    if ( ptr ) reset( ptr );
  }
  /// Copy constructor.
  inline SmartIF( const SmartIF& rhs ) : m_interface( rhs.get() )
  {
    if ( m_interface ) m_interface->addRef();
  }
  /// Move constructor
  inline SmartIF( SmartIF&& rhs ) : m_interface( rhs.m_interface ) { rhs.m_interface = nullptr; }
  /// Move assignement
  inline SmartIF& operator=( SmartIF&& rhs )
  {
    if ( m_interface ) m_interface->release();
    m_interface     = rhs.m_interface;
    rhs.m_interface = nullptr;
    return *this;
  }

  /// Constructor from another SmartIF, with a different type.
  /// @note it cannot replace the copy constructor.
  template <class T>
  inline explicit SmartIF( const SmartIF<T>& rhs )
  {
    reset( rhs.get() );
  }
  /// Standard Destructor.
  inline ~SmartIF() { reset(); }

  // ---------- Boolean and comparison methods ----------
  /// Allow for check if smart pointer is valid.
  inline bool isValid() const { return m_interface != nullptr; }

  inline explicit operator bool() const { return isValid(); }
  inline bool operator!() const { return !isValid(); }

  // ---------- Pointer access methods ----------
  /// Automatic conversion to pointer.
  /// It is also used by the compiler for automatic conversion to boolean.
  inline operator TYPE*() const { return m_interface; }
  /// Dereference operator
  inline TYPE* operator->() const { return m_interface; }
  /// Dereference operator
  inline TYPE& operator*() const { return *m_interface; }
  /// Get interface pointer
  inline TYPE* get() const { return m_interface; }
#if !defined( GAUDI_V22_API ) && !defined( NEW_SMARTIF )
  /// Get reference to the pointer
  inline TYPE*& pRef() { return m_interface; }
#endif

  // ---------- Cast methods ----------
  /// Set the internal pointer to the passed one disposing of the old one.
  /// Version for pointers of the same type of the managed ones (no call to
  /// queryInterface needed).
  inline void reset( TYPE* ptr = nullptr )
  {
    if ( ptr == m_interface ) return;
    if ( m_interface ) m_interface->release();
    m_interface = ptr;
    if ( m_interface ) m_interface->addRef();
  }
  /// Set the internal pointer to the passed one disposing of the old one.
  /// Version for pointers of types inheriting from IInterface.
  template <class OTHER>
  inline void reset( OTHER* ptr )
  {
    if ( static_cast<IInterface*>( ptr ) == static_cast<IInterface*>( m_interface ) ) return;
    if ( m_interface ) m_interface->release();
    if ( ptr ) {
      ptr->queryInterface( TYPE::interfaceID(), pp_cast<void>( &m_interface ) ).ignore();
    } else {
      m_interface = nullptr;
    }
  }

  /// return a new SmartIF instance to another interface
  template <typename IFace>
  SmartIF<IFace> as() const
  {
    return SmartIF<IFace>{*this};
  }

  // ---------- Special hacks ----------
  /// Assignment operator from IInterface pointer.
  /// It allows things like
  /// <code>
  /// SmartIF<T> x;
  /// x = 0;
  /// </code>
  inline SmartIF& operator=( IInterface* ptr )
  {
    reset( ptr );
    return *this;
  }
  /// Assignment operator.
  inline SmartIF& operator=( const SmartIF& rhs )
  {
    reset( rhs.get() );
    return *this;
  }
  /// Assignment operator from a different SmartIF.
  /// @note it cannot replace the assignment operator.
  template <class T>
  inline SmartIF& operator=( const SmartIF<T>& rhs )
  {
    reset( rhs.get() );
    return *this;
  }
};

// helper function to turn a pointer to an interface into
// the corresponding SmartIF -- this avoids having to type
// the typename twice, and thus insures consistency
template <typename IFace>
SmartIF<IFace> make_SmartIF( IFace* iface )
{
  return SmartIF<IFace>{iface};
}

#endif // GAUDI_SMARTIF_H
