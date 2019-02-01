// ============================================================================
#ifndef GAUDIPYTHON_INTERFACE_H
#define GAUDIPYTHON_INTERFACE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <functional>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/SmartIF.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================

namespace GaudiPython {
  /** @struct Interface Interface.h GaudiPython/Interface.h
   *
   *  Minor mofidication of original Pere's structure
   *  GaudiPython::Interface
   *  This helper class is nesessary to perform C++
   *  castings from python.
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date   2005-08-03
   */
  template <class TYPE>
  struct Interface {
    /** the only one important method
     *   @param in  input interface
     *   @return resutl of "cast"
     */
    TYPE* operator()( const IInterface* in ) const {
      return SmartIF<TYPE>( TYPE::interfaceID(), const_cast<IInterface*>( in ) );
    }

    /** the only one important method (static)
     *   @param in  input interface
     *   @return resutl of "cast"
     */
    static TYPE* cast( const IInterface* in ) { return SmartIF<TYPE>( const_cast<IInterface*>( in ) ); }

    /** the only one important method (static)
     *   @param iid unique identifier of 'target' interface
     *   @param in  input interface
     *   @return resutl of "cast"
     */
    static TYPE* cast( const InterfaceID& iid, const IInterface* in ) {
      return SmartIF<TYPE>( iid, const_cast<IInterface*>( in ) );
    }
  };

  template <>
  struct Interface<IInterface> {
    typedef IInterface TYPE;
    /** the only one important method
     *   @param in  input interface
     *   @return resutl of "cast"
     */
    TYPE* operator()( const IInterface* in ) const { return SmartIF<TYPE>( const_cast<IInterface*>( in ) ); }

    /** the only one important method (static)
     *   @param in  input interface
     *   @return resutl of "cast"
     */
    static TYPE* cast( const IInterface* in ) { return SmartIF<TYPE>( const_cast<IInterface*>( in ) ); }

    /** the only one important method (static)
     *   @param iid unique identifier of 'target' interface
     *   @param in  input interface
     *   @return resutl of "cast"
     */
    static TYPE* cast( const InterfaceID& /* iid */, const IInterface* in ) {
      return SmartIF<TYPE>( const_cast<IInterface*>( in ) );
    }
  };

} // end of namespace GaudiPython

#endif // GAUDIPYTHON_INTERFACE_H
