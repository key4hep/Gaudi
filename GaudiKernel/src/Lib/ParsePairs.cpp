// $Id: ParsePairs.cpp,v 1.3 2007/11/15 14:01:06 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.3 $ 
// ============================================================================
// Include files
// ============================================================================
// Local
// ============================================================================
#include "GaudiKernel/Parsers.icpp"
// ============================================================================
/** @file 
 *
 *  Implementation of the parser function for std::pair<TYPE1,TYPE2> 
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
    // pair< TYPE1 , TYPE2 > 
    // ========================================================================
    StatusCode parse ( pair<double,double>& result , const string& input )
    {
      return parse
        ( createIterator(input), 
          IteratorT(),
          PairGrammar < RealGrammar<double> , RealGrammar <double> >()
          [var(result)=arg1],
          SkipperGrammar()).full;
    }
    // ===========================================================================
    StatusCode parse ( pair<int,int>& result        , const string& input )
    {
      return parse
        ( createIterator(input), 
          IteratorT(),
          PairGrammar < IntGrammar<int> , IntGrammar <int> >()[var(result)=arg1],
          SkipperGrammar()).full;
    }
    // ========================================================================
  } // end of namespace Parsers
} // end of namespace Gaudi
// ============================================================================
// The END 
// ============================================================================


