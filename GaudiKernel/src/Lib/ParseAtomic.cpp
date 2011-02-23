// $Id: ParseAtomic.cpp,v 1.4 2008/10/28 14:02:18 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.4 $ 
// ============================================================================
// Include files
// ============================================================================
// Local
// ============================================================================
#include "GaudiKernel/Parsers.icpp"
// ============================================================================

// ============================================================================
/** @file 
 *
 *  Implementation of the parsing function for atomic primitive types 
 *  
 *  For adding new type you should have only grammar class which recognize 
 *  this type. Then by combining this grammar with such already implemented 
 *  grammars  like VectorGrammar, MapGrammar, PairGrammar you can very simply
 *  implement container types which will work with your type.
 *
 *  If you want your type can be handled in property file - see ValueGrammar
 *  in JobOptionsSvc which lays in GaudiSvc.
 *
 *  @author Alexander MAZUROV  Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-05-12 
 */
// ============================================================================
namespace Gaudi
{ 
  namespace Parsers
  {
    // ========================================================================
    StatusCode parse ( char&               result  , 
                       const string&       input   )
    { return parse_char    ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( unsigned char&      result  , 
                       const string&       input   )
    { return parse_char    ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( signed char&        result  , 
                       const string&       input   )
    { return parse_char    ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( short&              result , 
                       const string&       input  )
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( unsigned short&     result , 
                       const string&       input  )
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( int&                result ,
                       const string&       input  )
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( unsigned int&       result , 
                       const string&       input  )
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( long&               result , 
                       const string&       input  )
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( unsigned long&      result , 
                       const string&       input  )
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( long long&          result ,
                       const string&       input  ) 
    { return parse_integer ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( unsigned long long& result , 
                       const string&       input  )
    { return parse_integer ( result , input ) ; }    
    // ========================================================================
    StatusCode parse ( bool&               result , 
                       const string&       input  )
    {
      BoolGrammar g;
      return parse( 
          createIterator(input), 
          IteratorT(),
          g[var(result)=arg1]).full;
    }
    // ========================================================================
    StatusCode parse ( float&              result , 
                       const string&       input  ) 
    { return parse_real ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( double&             result ,
                       const string&       input  )
    { return parse_real ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( long double&        result , 
                       const string&       input  )
    { return parse_real ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( string&             result , 
                       const string&       input  )
    {
      StringGrammar g;
      if ( !parse(
              createIterator(input), 
              IteratorT(),
              g[var(result)=arg1]).full ){ result = input ; }
      /// @attention always!!!
      return true ;
    }
  } // end of namespace Parsers
} // end of namespace Gaudi

// ============================================================================
// The END 
// ============================================================================


