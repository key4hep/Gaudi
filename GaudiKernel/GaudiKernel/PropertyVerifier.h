// $Id:$
// ============================================================================
#ifndef GAUDIKERNEL_PROPERTYVERIFIER_H
#define GAUDIKERNEL_PROPERTYVERIFIER_H
// ============================================================================
// Include files
// ============================================================================
// STD& STL
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/PropertyTypeTraits.h"
// ============================================================================
/**********************************************************
 * Verifier Classes are used with Property Classes to
 * provide validation criteria for Property values.
 *
 **********************************************************/
/** @class PropertyVerifier GaudiKernel/PropertyVerifier.h
 *
 *  Templated Verifier base class
 *  @author CTDay
 */
template< class T >
class PropertyVerifier
{
public:
  // Data and Function Members for Collaborators.
  // Constructors - compiler generated default is ok
  // Destructor
  virtual ~PropertyVerifier() { }

  // Copy Constructor - compiler generated default is ok
  // Assignment Operator - compiler generated default is ok
  // Accessor members (const)
  virtual bool isValid ( const typename Gaudi::Utils::PropertyTypeTraits<T>::CVal value ) const = 0;
};
// ============================================================================
/** @class NullVerifier GaudiKernel/PropertyVerifier.h
 *
 *  Default, always true verifier
 *
 *  @author CTDay
 */
template< class T >
class NullVerifier : public PropertyVerifier< T >
{
public:
  // Data and Function Members for Collaborators.
  // Constructors - compiler generated default is ok
  // Destructor
  virtual ~NullVerifier() { }

  // Copy Constructor - compiler generated default is ok
  // Assignment Operator - compiler generated default is ok

  // Accessor members (const)
  virtual bool isValid
  ( const typename Gaudi::Utils::PropertyTypeTraits<T>::CVal /* val */ ) const
  { return true; }

};
// ============================================================================
/** @class BoundedVerifier PropertyVerifier.h GaudiKernel/PropertyVerifier.h
 *  @author CTDay
 */
template< class T >
class BoundedVerifier : public PropertyVerifier< T > {     // Abstract derived class
public:
  /// Constructors
  BoundedVerifier()
    : m_hasLowerBound( false ),
      m_hasUpperBound( false ),
      m_lowerBound( T() ),
      m_upperBound( T() ) { }

  /// Destructor
  virtual ~BoundedVerifier() { }

  /// Check if the value is within bounds
  bool isValid( const typename Gaudi::Utils::PropertyTypeTraits<T>::CVal value ) const
  {
    return
      (( m_hasLowerBound && ( *value < m_lowerBound ) ) ? false : true ) &&
      (( m_hasUpperBound && ( *value > m_upperBound ) ) ? false : true )  ;
  }

  /// Return if it has a lower bound
  bool        hasLower() const { return m_hasLowerBound; }
  /// Return if it has a lower bound
  bool        hasUpper() const { return m_hasUpperBound; }
  /// Return the lower bound value
  const T&    lower()    const { return m_lowerBound; }
  /// Return the upper bound value
  const T&    upper()    const { return m_upperBound; }

  /// Set lower bound value
  void setLower( const T& value ) { m_hasLowerBound = true; m_lowerBound = value; }
  /// Set upper bound value
  void setUpper( const T& value ) { m_hasUpperBound = true; m_upperBound = value; }
  /// Clear lower bound value
  void clearLower()  { m_hasLowerBound = false; m_lowerBound = T(); }
  /// Clear upper bound value
  void clearUpper()  { m_hasUpperBound = false; m_upperBound = T(); }

  /// Set both bounds (lower and upper) at the same time
  void setBounds( const T& lower, const T& upper)
  {
    setLower( lower );
    setUpper( upper );
  }

  /// Clear both bounds (lower and upper) at the same time
  void clearBounds()
  {
    clearLower();
    clearUpper();
  }

private:
  /// Data and Function Members for This Class Implementation.
  /// Data members
  bool  m_hasLowerBound;
  bool  m_hasUpperBound;
  T     m_lowerBound;
  T     m_upperBound;
};
// ============================================================================

// =======================================================================
// The END
// =======================================================================
#endif    // GAUDIKERNEL_PROPERTYVERIFIER_H
// =======================================================================

