// $Id: ParseHistos.cpp,v 1.3 2008/10/28 14:02:18 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HistoDef.h"
#include "GaudiKernel/HistoProperty.h"
// ============================================================================
// local
// ============================================================================
#include "GaudiKernel/Parsers.icpp"
// ============================================================================
/** @file 
 *  Implementation file for Histogram Parsing functions 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com 
 *  @date 2007-09-20 
 */
// ============================================================================
namespace Gaudi
{
  namespace Parsers 
  {
    // ========================================================================
    /** @class Histo1DGrammar 
     *  The simple grammar for histogram description/definition:
     *
     *  The valid representations of the histogram descriptor are:
     *
     *    - (0.0,1.0,100,'title') 
     *    - (0.0,1.0,100) 
     *    - (0.0,1.0) 
     *    - ('title',0.0,1.0,100)
     *    - ('title',0.0,1.0)
     *    - (0.0,1.0,'title',100)
     *    - (0.0,1.0,'title')
     *  
     *  @see Gaudi::Histo1DDef 
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com 
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-17
     */
    class Histo1DGrammar 
      : public grammar<Histo1DGrammar, 
                       ClosureGrammar<Gaudi::Histo1DDef>::context_t> 
    {
    public:
      // the actual type of the target 
      typedef Gaudi::Histo1DDef ResultT;
    public:
      /// callback. Action when we match first value
      void matchLow   ( const double       low   ) const 
      { this -> val() . setLowEdge  ( low   ) ; }
      void matchHigh  ( const double       high  ) const 
      { this -> val() . setHighEdge ( high  ) ; }
      void matchTitle ( const std::string& title ) const 
      { this -> val() . setTitle    ( title ) ; }
      void matchBins  ( const int          bins  ) const 
      { this -> val() . setBins     ( bins  ) ; }
    public:
      template <typename ScannerT> 
      struct definition 
      {
        typedef Gaudi::Parsers::Histo1DGrammar H1G;
        definition( H1G const &self) 
        {
          val1 =         sG [boost::bind(&H1G::matchTitle , &self , _1 ) ] 
            >>    "," >> rG [boost::bind(&H1G::matchLow   , &self , _1 ) ]   
            >>    "," >> rG [boost::bind(&H1G::matchHigh  , &self , _1 ) ] 
            >> !( "," >> iG [boost::bind(&H1G::matchBins  , &self , _1 ) ] ) ;
          
          val2 =         rG [boost::bind(&H1G::matchLow   , &self , _1 ) ]   
            >>    "," >> rG [boost::bind(&H1G::matchHigh  , &self , _1 ) ] 
            >>    "," >> sG [boost::bind(&H1G::matchTitle , &self , _1 ) ]
            >> !( "," >> iG [boost::bind(&H1G::matchBins  , &self , _1 ) ] ) ;
          
          val3 =         rG [boost::bind(&H1G::matchLow   , &self , _1 ) ]   
            >>    "," >> rG [boost::bind(&H1G::matchHigh  , &self , _1 ) ] 
            >> !( "," >> iG [boost::bind(&H1G::matchBins  , &self , _1 ) ] ) 
            >> !( "," >> sG [boost::bind(&H1G::matchTitle , &self , _1 ) ] ) ;
          
          histogram = "(" >> ( val1 || val2 || val3 ) >> ")";
        }
        RealGrammar<> rG ;
        IntGrammar<>  iG ;
        StringGrammar sG ;
        boost::spirit::rule<ScannerT> const& start() const { return histogram;}
        rule<ScannerT> histogram ;
        rule<ScannerT> val1,val2,val3 ;
      };
    };
  } // end of namespace Gaudi::Parsers 
} // end of namespace Gaudi 
// ============================================================================
/*  helper function, needed for the implementation of "Histogram Property"
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com 
 *  @date 2007-09-17
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( Gaudi::Histo1DDef& histo , 
  const std::string& input ) 
{
  Histo1DGrammar g;
  const bool full =
    boost::spirit::parse
    ( createIterator(input), 
      IteratorT(), 
      g[var(histo)=arg1],
      SkipperGrammar()).full;
  //
  return full && histo.ok() ? StatusCode::SUCCESS : StatusCode::FAILURE ;
}
// ============================================================================
/** helper function, needed for implementation of "Histogram Property"
 *  @param histos the map of the histogram descriptions (output)
 *  @param input the string to be parsed 
 *  @return status code 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com 
 *  @date 2007-09-17
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( std::map<std::string,Gaudi::Histo1DDef>& histos , 
  const std::string&                       input  ) 
{
  MapGrammar<StringGrammar,Histo1DGrammar> g;
  const bool full = boost::spirit::parse
    ( createIterator(input), 
      IteratorT()     ,
      g[var(histos)=arg1],
      SkipperGrammar() ).full;
  //
  if ( !full ) { return StatusCode::FAILURE ; }                 // RETURN 
  //
  for ( std::map<std::string,Gaudi::Histo1DDef>::const_iterator 
          ih = histos.begin() ; histos.end() != ih ; ++ih ) 
  { if ( !ih->second.ok() ) { return StatusCode::FAILURE ; } }  // RETURN 
  //
  return StatusCode::SUCCESS ;                                  // RETURN 
}
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
