// $Id: $
// ============================================================================
#ifndef GAUDIKERNEL_PROPTRAITS_H 
#define GAUDIKERNEL_PROPTRAITS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <algorithm>
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils 
  {
    // ========================================================================
    /** @struct PropertyTypeTraits
     *  helper structure to define the types for properties 
     *
     *  For implementation of SimpleProperty<TYPE> one needs:
     *
     *    - "default constructor"
     *    - "smart copy-constructor"
     *    - "smart destructor"
     *    - "assign"  
     *    - "less"   
     *
     *  For implementation of SimplePropertyRef<TYPE> one needs:
     * 
     *    - "copy constructor"
     *    - "smart copy-constructor
     *    - "smart destructor"
     *    - "assign"  
     *    - "less"   
     *
     *  For all cases one needs to have valid functions: 
     *
     *    - StatusCode Gaudi::Parsers::parse ( TYPE& , const std::string& ) 
     *    - std::ostream& Gaudi::Utils::toStream ( cost TYPE& , std::ostream& ) 
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-15
     */
    template <class TYPE> 
    struct PropertyTypeTraits 
    {
      // =======================================================================
      typedef TYPE        Type  ;
      typedef const TYPE& PRef  ;
      typedef TYPE*       PVal  ;
      typedef const TYPE* CVal  ;
      // =======================================================================
      /// some kind of default constructor 
      static PVal new_ ()                    { return new Type(     ) ; }
      /// some kind of copy    constructor 
      static PVal new_ ( const Type& right ) { return new Type(right) ; }
      /// "smart" destructor
      static void dele   ( PVal right , const bool own ) 
      { if ( own ) { delete right ; } }
      /// "smart" copy-constructor 
      static PVal copy   ( PVal* right , const bool  own ) 
      {
        if ( !own ) { return right ; }
        return new_ ( *right ) ;
      }    
      /// assignment
      static void assign ( Type& v1    ,       PRef  v2  ) { v1 = v2 ; }
      /// comparison (needed for bounded verifier)
      static bool less ( const Type& v1 , const Type& v2 )  
      { return v1 < v2 ; }
      // ======================================================================
    } ;
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils 
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIKERNEL_PROPTRAITS_H
// ============================================================================
