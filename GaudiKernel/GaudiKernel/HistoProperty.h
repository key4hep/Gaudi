// $Id: HistoProperty.h,v 1.1 2007/09/26 16:13:41 marcocle Exp $
// ============================================================================
#ifndef GAUDIKERNEL_HISTOPROPERTY_H 
#define GAUDIKERNEL_HISTOPROPERTY_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HistoDef.h"
#include "GaudiKernel/Property.h"
// ============================================================================
template <class TYPE>                class BoundedVerifier   ;
template <class TYPE>                class NullVerifier      ;
template <class TYPE,class VERIFIER> class SimpleProperty    ;
template <class TYPE,class VERIFIER> class SimplePropertyRef ;
// ============================================================================
/** Check if the value is OK 
 *  it is a nesessary template specialisation to avoid the compilation error
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-18 
 */
template <>
inline bool 
BoundedVerifier<Gaudi::Histo1DDef>::isValid
( const Gaudi::Histo1DDef* value ) const 
{ 
  return 0 != value && value->ok()
    && ( ( m_hasLowerBound && ( *value       < m_lowerBound ) ) ? false : true ) 
    && ( ( m_hasUpperBound && ( m_upperBound < *value       ) ) ? false : true ) ;
}
// ============================================================================
/** Check if the value is OK 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-18 
 */
template <>
inline bool 
NullVerifier<Gaudi::Histo1DDef>::isValid
( const Gaudi::Histo1DDef* value ) const 
{ 
  return 0 != value && value->ok() ;
}
// ============================================================================
/// the actual type of "histogram property"
typedef SimpleProperty   <Gaudi::Histo1DDef> Histo1DProperty    ;
// ============================================================================
/// the actual type of "histogram property reference"
typedef SimplePropertyRef<Gaudi::Histo1DDef> Histo1DPropertyRef ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIKERNEL_HISTOPROPERTY_H
// ============================================================================
