// $Id: ParseVectorsReal.cpp,v 1.2 2007/05/24 14:39:11 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.2 $
// ============================================================================
// Include files
// ============================================================================
// Local
// ============================================================================
#ifdef WIN32
// Disable warning
//   C4244: '=' : conversion from 'int' to 'float', possible loss of data
// Coming from the parsers(?).
#pragma warning(disable:4244)
#endif
#include "GaudiKernel/Parsers.icpp"
// ============================================================================
// ============================================================================
/** @file
 *
 *  Implementation of the parsing functions for std::vector<TYPE>
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
    StatusCode parse ( vector<float>&              result ,
                       const string&               input  )
    { return parse_real_vector    ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<double>&             result ,
                       const string&               input  )
    { return parse_real_vector    ( result , input ) ; }
    // ========================================================================
    StatusCode parse ( vector<long double>&        result ,
                       const string&               input  )
    { return parse_real_vector    ( result , input ) ; }
    // ========================================================================

  } // end of namespace Parsers
} // end of namespace Gaudi

// ============================================================================
// The END
// ============================================================================


