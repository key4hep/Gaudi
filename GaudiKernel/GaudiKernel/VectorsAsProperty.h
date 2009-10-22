// $Id: $
// ============================================================================
#ifndef GAUDIKERNEL_VCTPARSERS_H 
#define GAUDIKERNEL_VCTPARSERS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
#include <map>
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Point4DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Vector4DTypes.h"
// ============================================================================
/** @file 
 *  Declaration of parsing functions for various ROOT::Math objects to allow 
 *  their usage as properties for Gaudi components 
 *  
 *  @attention: this file must be included BEFORE any direct or indirect 
 *              inclusion of GaudiKernel/ToStream.h
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Parsers 
  {
    // ========================================================================
    /** parse 3D-point 
     *
     *  Valid representations of 3D-point:
     *
     *      - a'la python tuple with 3 elements ("canonical")
     *      - a'la python list with 3 elements 
     *      - tuple or list with named ordered fields 
     *
     *  @code
     *
     *    " (1,2,3) " 
     *    " [1,2,3] " 
     *    " [ x : 1, 2, Z:3 ] " 
     *    " [ pX : 1 , PY : 2, 3] " 
     *     
     *  @endcode 
     *
     *  Valid keys for names fields:
     *
     *  @code
     *
     *    "x", "X" , "pX" , "Px" , "PX "
     *    "y", "Y" , "pY" , "Py" , "PY "
     *    "z", "Z" , "pZ" , "Pz" , "PZ "
     *
     *  @endcode 
     *
     *  @attention Named fields must be ordered <code>(x,y,z)</code>
     *
     *  @param result (output) the parsed point 
     *  @param input  (input)  the input string 
     *  @return status code 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    GAUDI_API StatusCode parse 
    ( Gaudi::XYZPoint&   result  , 
      const std::string& input   ) ;
    // ========================================================================
    /** parse 3D-vector 
     *
     *  Valid representations of 3D-vector:
     *
     *      - a'la python tuple with 3 elements ("canonical")
     *      - a'la python list with 3 elements 
     *      - tuple or list with named ordered fields 
     *
     *  @code
     *
     *    " (1,2,3) " 
     *    " [1,2,3] " 
     *    " [ x : 1, 2, Z:3 ] " 
     *    " [ pX : 1 , PY : 2, 3] " 
     *     
     *  @endcode 
     *
     *  Valid keys for names fields:
     *
     *  @code
     *
     *    "x", "X" , "pX" , "Px" , "PX "
     *    "y", "Y" , "pY" , "Py" , "PY "
     *    "z", "Z" , "pZ" , "Pz" , "PZ "
     *
     *  @endcode 
     *
     *  @attention Named fields must be ordered <code>(x,y,z)</code>
     *
     *  @param result (output) the parsed vector
     *  @param input  (input)  the input string 
     *  @return status code 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    GAUDI_API StatusCode parse 
    ( Gaudi::XYZVector&  result  , 
      const std::string& input   ) ;
    // ========================================================================
    /** parse LorentzVector
     *
     *  Valid representations of Lorenzt vector
     *
     *      - a'la python tuple with 4 elements ("canonical")
     *      - a'la python list with 4 elements 
     *      - python/list with inner representation of 3D-point/vector
     *      - tuple or list with named ordered fields 
     *
     *  @code
     *
     *    " (1,2,3,4) "     
     *    " (1,2,3;4) "
     *
     *    " [1,2,3,4] " 
     *    " [1,2,3;4] " 
     *
     *    " [ x:1 ,2,3; e= 4] " 
     *    " [ pX : 1 , PY : 2, 3 , T= 4] " 
     *
     *    " [ ( pX : 1 , PY : 2, 3 ) , 4] " 
     *    " [ ( pX : 1 , PY : 2, 3 ) ; 4] " 
     *
     *    " [ 4 , ( pX : 1 , PY : 2, 3 ) ] " 
     *    " [ 4 ; ( pX : 1 , PY : 2, 3 ) ] " 
     *
     *    " [ [ pX : 1 , PY : 2, 3 ] , 4] " 
     *    " [ [ pX : 1 , PY : 2, 3 ] ; 4] " 
     *
     *    " [ 4 , [ pX : 1 , PY : 2, 3 ] ] " 
     *    " [ 4 ; [ pX : 1 , PY : 2, 3 ] ] " 
     *
     *    " ( ( pX : 1 , PY : 2, 3 ) , 4 )" 
     *    " ( ( pX : 1 , PY : 2, 3 ) ; 4 )" 
     *
     *    " ( 4 , ( pX : 1 , PY : 2, 3 ) )" 
     *    " ( 4 ; ( pX : 1 , PY : 2, 3 ) )" 
     *
     *    " ( [ pX : 1 , PY : 2, 3 ] , 4 )" 
     *    " ( [ pX : 1 , PY : 2, 3 ] ; 4 )" 
     *
     *    " ( 4 , [ pX : 1 , PY : 2, 3 ] )" 
     *    " ( 4 ; [ pX : 1 , PY : 2, 3 ] )" 
     *
     *     
     *  @endcode 
     *
     *  Note that "eenrgy" element can be separated with semicolumn.
     *
     *  Valid keys for names fields:
     *
     *  @code
     *
     *    "x", "X" , "pX" , "Px" , "PX "
     *    "y", "Y" , "pY" , "Py" , "PY "
     *    "z", "Z" , "pZ" , "Pz" , "PZ "
     *    "t", "T" , "e"  , "E" 
     *
     *  @endcode 
     *
     *  @attention Named fields must be ordered <code>(x,y,z)</code>
     *
     *  @param result (output) the parsed lorentz vector  
     *  @param input  (input)  the input string 
     *  @return status code 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    GAUDI_API StatusCode parse 
    ( Gaudi::LorentzVector&  result  , 
      const std::string&     input   ) ;
    // ========================================================================
    /** parse the vector of points 
     *  @param resut (OUTPUT) the parser vector 
     *  @param input (INPIUT) the string to be parsed 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    GAUDI_API StatusCode parse 
    ( std::vector<Gaudi::XYZPoint>&  result  , 
      const std::string&             input   ) ;
    // ========================================================================
    /** parse the vector of vectors 
     *  @param resut (OUTPUT) the parser vector 
     *  @param input (INPIUT) the string to be parsed 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    GAUDI_API StatusCode parse 
    ( std::vector<Gaudi::XYZVector>&  result  , 
      const std::string&              input   ) ;
    // ========================================================================
    /** parse the vector of vectors 
     *  @param resut (OUTPUT) the parser vector 
     *  @param input (INPIUT) the string to be parsed 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    GAUDI_API StatusCode parse 
    ( std::vector<Gaudi::LorentzVector>&  result  , 
      const std::string&                  input   ) ;
    // ========================================================================
  } //                                          end of namespace Gaudi::Parsers 
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
namespace Gaudi
{
  // =========================================================================
  namespace Utils 
  {
    // ========================================================================
    /// print XYZ point 
    GAUDI_API std::ostream& toStream 
    ( const Gaudi::XYZPoint&      obj, std::ostream& s ) ;
    /// print XYZ vector  
    GAUDI_API std::ostream& toStream 
    ( const Gaudi::XYZVector&     obj, std::ostream& s ) ;
    /// print Lorentz vector  
    GAUDI_API std::ostream& toStream 
    ( const Gaudi::LorentzVector& obj, std::ostream& s ) ;
    // ========================================================================
  } // end of namespace Gaudi::Utils
  // ==========================================================================
} // end of namespace Gaudi 
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIKERNEL_VCTPARSERS_H
// ============================================================================
