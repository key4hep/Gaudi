// $Id: $
// ============================================================================
#ifndef GAUDIKERNEL_CARRAYASPROPERTY_H 
#define GAUDIKERNEL_CARRAYASPROPERTY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <algorithm>
// ============================================================================
// GauidKernel
// ============================================================================
#include "GaudiKernel/PropertyTypeTraits.h"
#include "GaudiKernel/PropertyVerifier.h"
// ============================================================================
/** @file GaudiKenrel/CArrayAsProperty.h
 *
 *  Collection of utilities, which allows to use C-arrays 
 *  as property for Gaudi-components 
 *
 *  - streamers 
 *  - parsers 
 *  - property type traits 
 *  - property verifiers 
 *
 *  @attention this file must be "included" before GaudiKernel/Property.h
 * 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-16
 */
// ============================================================================
// 1) Streamers : value  -> string , done in GausiKernel/ToStream.h
// ============================================================================
// 2) Parsers   : string -> value  , done in GaudiKernel/Parsers.h
// ============================================================================
// 3) assigements and copy 
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils 
  {
    // ========================================================================
    template <class T, unsigned int N>
    struct PropertyTypeTraits<T(&)[N]>
    {
      // ======================================================================
      typedef       T(&Type) [N] ;
      typedef       T(&PRef) [N] ;
      typedef       T(*PVal) [N] ;
      typedef const T(&CRef) [N] ;
      typedef const T(*CVal) [N] ;
      // ======================================================================
    public:
      // ======================================================================
      /// some kind of "default" constructor 
      static PVal new_ () { return new T[N] ; }
      /// some kind of "copy" constructor 
      static PVal new_ ( Type right )
        { 
          PVal tmp = new_ () ;
          assign ( tmp , right ) ;
          return tmp ;
        }
      /// some kind of "smart"-pseudo-constructor 
      static PVal copy ( PVal* right , const bool  own ) 
        {
          if ( !own ) { return right ; }
          return new_ ( *right ) ;
        }
      /// "smart" pseudo-destructor 
      static void dele   ( PVal right , const bool own ) 
        { if ( own ) { delete[] right ; } }
      // ======================================================================
      // assigenements 
      static void assign ( Type v1 , PRef v2 ) { std::copy ( v2 , v2 + N , v1 ) ; }
      static void assign ( Type v1 , CRef v2 ) { std::copy ( v2 , v2 + N , v1 ) ; }
      /// comparison (needed for bounded verifier)
      static bool less   ( Type v1 , Type v2 )  
        { return std::lexicographical_compare ( v1 , v1 + N , v2 , v2 + N ) ; }
      static bool less   ( Type v1 , CRef v2 )  
        { return std::lexicographical_compare ( v1 , v1 + N , v2 , v2 + N ) ; }
      static bool less   ( CRef v1 , CRef v2 )  
        { return std::lexicographical_compare ( v1 , v1 + N , v2 , v2 + N ) ; }
      static bool less   ( CRef v1 , Type v2 )  
        { return std::lexicographical_compare ( v1 , v1 + N , v2 , v2 + N ) ; }
      // ======================================================================
    } ;
    // ========================================================================
    /// specialization for arrays 
    template <class T, unsigned int N>
    struct PropertyTypeTraits<T[N]> : public PropertyTypeTraits<T(&)[N]>
    {} ;
    // ========================================================================
    /// specialiation for const-arrays 
    template <class T, unsigned int N>
    struct PropertyTypeTraits< const T(&)[N]> 
    {} ;
    /// specialisation for C-strings
    template <unsigned int N>
    struct PropertyTypeTraits<char(&)[N]> 
    {} ;    
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils 
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// property verifier 
// ============================================================================
/// specialization of Bounded verifier for for C-arrays
template< class T, unsigned int N>
class BoundedVerifier<T[N]> : PropertyVerifier<T[N]> 
{    
  // ==========================================================================
  typedef Gaudi::Utils::PropertyTypeTraits<T[N]>  Traits ;
  // Abstract derived class
  // ==========================================================================
public:
  /// Constructors
  BoundedVerifier() 
    : m_hasLowerBound ( false )
    , m_hasUpperBound ( false )
  {}
  /// Destructor
  virtual ~BoundedVerifier() { }
  
  /// Check if the value is within bounds
  bool isValid ( const typename Traits::CVal value ) const 
  { 
    return   
      ( ( m_hasLowerBound && Traits::less ( *value , m_lowerBound ) ) ? false : true ) 
      &&
      ( ( m_hasUpperBound && Traits::less ( m_upperBound , *value ) ) ? false : true ) ;
  }
  /// Return if it has a lower bound
  bool        hasLower() const { return m_hasLowerBound; }
  /// Return if it has a lower bound
  bool        hasUpper() const { return m_hasUpperBound; }
  /// Return the lower bound value
  typename Traits::CRef lower()    const { return m_lowerBound; }
  /// Return the upper bound value
  typename Traits::CRef upper()    const { return m_upperBound; }
  
  /// Set lower bound value
  void setLower( typename Traits::CRef value ) { m_hasLowerBound = true; Traits::assign ( m_lowerBound , value ) ; }
  /// Set upper bound value
  void setUpper( typename Traits::CRef value ) { m_hasUpperBound = true; Traits::assign ( m_upperBound , value ) ; }
  /// Clear lower bound value
  void clearLower()  { m_hasLowerBound = false; }
  /// Clear upper bound value
  void clearUpper()  { m_hasUpperBound = false; }
  
  /// Set both bounds (lower and upper) at the same time
  void setBounds( typename Traits::CRef lower, typename Traits::CRef upper) 
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
  // ==========================================================================
  /// Data and Function Members for This Class Implementation.
  /// Data members
  bool    m_hasLowerBound  ;
  bool    m_hasUpperBound  ;
  const T m_lowerBound [N] ;
  const T m_upperBound [N] ;
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_CARRAYASPROPERTY_H
// ============================================================================
