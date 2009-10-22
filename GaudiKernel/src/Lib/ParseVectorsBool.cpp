// $Id: ParseVectorsBool.cpp,v 1.4 2008/10/28 14:02:18 marcocle Exp $
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
    StatusCode parse ( vector<bool>&               result , 
                       const string&               input  )
    {
      VectorGrammar<BoolGrammar> g;
      return parse(
            createIterator(input), 
            IteratorT(),
            g[var(result)=arg1],
            SkipperGrammar()).full;
    }
    // ========================================================================

  } // end of namespace Parsers
} // end of namespace Gaudi

// ============================================================================
// The END 
// ============================================================================


