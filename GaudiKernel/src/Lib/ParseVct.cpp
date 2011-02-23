// $Id: $
// ============================================================================
// Include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <map>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/VectorsAsProperty.h"
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/ToStream.h"
// ============================================================================
// Local
// ============================================================================
#include "GaudiKernel/Parsers.icpp"
#include "GaudiKernel/Grammars.h"
#include "GaudiKernel/GrammarsForVectors.h"
// ============================================================================
/** @file 
 * Implementation file for vector parsers 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
namespace 
{
  // ==========================================================================
  typedef std::map<std::string,double> MAP   ;
  
  template <unsigned int N>
  inline MAP::const_iterator find 
  ( const MAP&           m         , 
    const std::string  (&keys)[N]  ) 
  {
    for ( unsigned int i =  0 ; i < N ; ++ i ) 
    {
      MAP::const_iterator found = m.find ( keys[i] ) ;
      if ( m.end() != found ) { return found ; }
    }
    return m.end() ;  
  }
  // ==========================================================================
  const std::string s_X[] = { "x" , "X" , "px" , "pX" , "Px" , "PX" } ;
  const std::string s_Y[] = { "y" , "Y" , "py" , "pY" , "Py" , "PY" } ;
  const std::string s_Z[] = { "z" , "Z" , "pz" , "pZ" , "Pz" , "PZ" } ;
  const std::string s_E[] = { "t" , "T" ,  "e" ,  "E"               } ;
  // ==========================================================================  
}
// ============================================================================
/*  parse 3D-point 
 *  @param result (output) the parsed point 
 *  @param input  (input)  the input string 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( Gaudi::XYZPoint&   result  , 
  const std::string& input   ) 
{
  // the grammar 
  Pnt3DGrammar<RealGrammar<double>,Gaudi::XYZPoint> g ;
  
  if ( parse ( input.begin () , 
               input.end   () ,
               g[var(result)=arg1],SkipperGrammar()).full ) 
  { return StatusCode::SUCCESS ; } // RETURN 
  
  // try map:
  MAP m ;
  StatusCode sc = parse ( m , input ) ;
  
  if ( sc.isFailure () ) { return sc                  ; } // RETURN 
  if ( 3 != m.size  () ) { return StatusCode::FAILURE ; } // REUTRN
  
  MAP::const_iterator x = find ( m , s_X ) ;
  MAP::const_iterator y = find ( m , s_Y ) ;
  MAP::const_iterator z = find ( m , s_Z ) ;
  
  if ( m.end() == x || 
       m.end() == y || 
       m.end() == z  ) { return StatusCode::FAILURE ; } // RETURN 
  
  result.SetXYZ ( x->second , 
                  y->second ,
                  z->second ) ;
  
  return StatusCode::SUCCESS ;
  
}
// ============================================================================
/*  parse 3D-vector 
 *  @param result (output) the parsed vector
 *  @param input  (input)  the input string 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( Gaudi::XYZVector&  result  , 
  const std::string& input   ) 
{
  
  Gaudi::XYZPoint point ;
  
  StatusCode sc = parse ( point , input ) ;
  if ( sc.isFailure() ){ return sc ; }                               // RETURN 
  
  result = point ;
  return StatusCode::SUCCESS ;                                       // RETURN  
}
#ifndef _WIN32
// ============================================================================
/*  parse LorentzVector
 *  @param result (output) the parsed lorentz vector  
 *  @param input  (input)  the input string 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( Gaudi::LorentzVector&  result  , 
  const std::string&     input   ) 
{
  // the grammar 
  Pnt4DGrammar<RealGrammar<double>,Gaudi::LorentzVector> g ;
  
  
  if ( parse ( input.begin () , 
               input.end   () ,
               g[var(result)=arg1],SkipperGrammar()).full ) 
  { return StatusCode::SUCCESS ; } // RETURN 
  
  // try map:
  MAP m ;
  StatusCode sc = parse ( m , input ) ;
  
  if ( sc.isFailure () ) { return sc                  ; } // RETURN 
  if ( 4 != m.size  () ) { return StatusCode::FAILURE ; } // REUTRN
  
  MAP::const_iterator x = find ( m , s_X ) ;
  MAP::const_iterator y = find ( m , s_Y ) ;
  MAP::const_iterator z = find ( m , s_Z ) ;
  MAP::const_iterator e = find ( m , s_E ) ;
  
  if ( m.end() == x || 
       m.end() == y || 
       m.end() == z || 
       m.end() == e ) { return StatusCode::FAILURE ; } // RETURN 
  
  result.SetXYZT ( x->second , 
                   y->second , 
                   z->second ,
                   e->second ) ;
  
  return StatusCode::SUCCESS ;

}
#endif

// ============================================================================
/*  parse the vector of points 
 *  @param resut (OUTPUT) the parser vector 
 *  @param input (INPIUT) the string to be parsed 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( std::vector<Gaudi::XYZPoint>&  result  , 
  const std::string&             input   ) 
{
  result.clear() ;
  
  // the grammar 
  VectorGrammar < Pnt3DGrammar<RealGrammar<double>,Gaudi::XYZPoint> >  g ;
  
  if ( parse ( input.begin () , 
               input.end   () ,
               g[var(result)=arg1],SkipperGrammar()).full ) 
  { return StatusCode::SUCCESS ; }                                  // RETURN 
  //
  return StatusCode::FAILURE ;
}
// ============================================================================
/*  parse the vector of vectors 
 *  @param resut (OUTPUT) the parser vector 
 *  @param input (INPIUT) the string to be parsed 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( std::vector<Gaudi::XYZVector>&  result  , 
  const std::string&              input   ) 
{
  result.clear() ;
  
  // the grammar 
  VectorGrammar < Pnt3DGrammar<RealGrammar<double>,Gaudi::XYZVector> >  g ;
  
  if ( parse ( input.begin () , 
               input.end   () ,
               g[var(result)=arg1],SkipperGrammar()).full ) 
  { return StatusCode::SUCCESS ; }                                  // RETURN 
  //
  return StatusCode::FAILURE ;
}
#ifndef _WIN32
// ============================================================================
/*  parse the vector of vectors 
 *  @param resut (OUTPUT) the parser vector 
 *  @param input (INPIUT) the string to be parsed 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( std::vector<Gaudi::LorentzVector>&  result  , 
  const std::string&              input   ) 
{
  result.clear() ;
  
  // the grammar 
  VectorGrammar < Pnt4DGrammar<RealGrammar<double>,Gaudi::LorentzVector> >  g ;
  
  if ( parse ( input.begin () , 
               input.end   () ,
               g[var(result)=arg1],SkipperGrammar()).full ) 
  { return StatusCode::SUCCESS ; }                                  // RETURN 
  //
  return StatusCode::FAILURE ;
}
#endif
// ============================================================================
// print XYZ-point 
// ============================================================================
std::ostream& Gaudi::Utils::toStream 
( const Gaudi::XYZPoint&  obj, std::ostream& s ) 
{
  s << "( "  ;
  toStream ( obj.X () , s ) ;
  s << " , " ;
  toStream ( obj.Y () , s ) ;
  s << " , " ;
  toStream ( obj.Z () , s ) ;
  s << " )"  ;  
  return s ;
}
// ============================================================================
// print XYZ-vector 
// ============================================================================
std::ostream& Gaudi::Utils::toStream 
( const Gaudi::XYZVector&  obj, std::ostream& s ) 
{ 
  s << "( "  ;
  toStream ( obj.X () , s ) ;
  s << " , " ;
  toStream ( obj.Y () , s ) ;
  s << " , " ;
  toStream ( obj.Z () , s ) ;
  s << " )"  ;
  
  return s ;
}
// ============================================================================
// print LorentzVector 
// ============================================================================
std::ostream& Gaudi::Utils::toStream 
( const Gaudi::LorentzVector&  obj, std::ostream& s ) 
{
  
  s << "( " ;
  toStream ( obj.Px () , s , 12 ) ;
  s << " , " ;
  toStream ( obj.Py () , s , 12 ) ;
  s << " , " ;
  toStream ( obj.Pz () , s , 13 ) ;
  s << " , "  ;
  toStream ( obj.E  () , s , 14 ) ;
  s << " )"  ;
  
  return s ;
}
// ============================================================================
// The END 
// ============================================================================
