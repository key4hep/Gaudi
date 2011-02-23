// $Id: ParseVectorsInteger.cpp,v 1.2 2007/05/24 14:39:11 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.2 $ 
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
 *  Implementation of the parsing functions for std::vetctor<TYPE>
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
    StatusCode parse ( vector<short>&              result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<unsigned short>&     result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<int>&                result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<unsigned int>&       result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<long>&               result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<unsigned long>&      result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<long long>&          result , 
                       const string&               input  ) 
    { return parse_integer_vector ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<unsigned long long>& result , 
                       const string&               input  )
    { return parse_integer_vector ( result , input ) ; }

  } // end of namespace Parsers
} // end of namespace Gaudi

// ============================================================================
// The END 
// ============================================================================


