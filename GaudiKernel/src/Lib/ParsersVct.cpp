// $Id:$
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/VectorsAsProperty.h"
// ============================================================================
#include "GaudiKernel/ParsersFactory.h"
#include "GaudiKernel/ToStream.h"
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
namespace Gaudi { namespace Parsers {
  template<typename T1, typename T2>
  inline StatusCode
  parse_(ROOT::Math::PositionVector3D<T1,T2>& result, const std::string& input){
    Skipper skipper;
    typename Grammar_<IteratorT, ROOT::Math::PositionVector3D<T1,T2>, Skipper >::Grammar g;
    IteratorT iter = input.begin(), end = input.end();
    if (qi::phrase_parse( iter, end, g, skipper, result)){
      return StatusCode::SUCCESS;
    }
    //@attention always
    return true;
  }
  // ==========================================================================

  StatusCode parse(Gaudi::XYZPoint& result,
                   const std::string& input  ) {
    return parse_(result, input);
  }

  // ==========================================================================
  /*  parse 3D-vector
   *  @param result (output) the parsed vector
   *  @param input  (input)  the input string
   *  @return status code
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-09-05
   */
  StatusCode parse
  ( Gaudi::XYZVector&  result  ,
    const std::string& input   )
  {

    Gaudi::XYZPoint point ;
    StatusCode sc = parse ( point , input ) ;
    if ( sc.isFailure() ){ return sc ; }                               // RETURN
    result = point ;
    return StatusCode::SUCCESS ;                                       // RETURN
  }

  StatusCode parse(Gaudi::LorentzVector& result, const std::string& input) {
    return parse_(result, input);
  }

  // ==========================================================================
  /*  parse the vector of points
   *  @param resut (OUTPUT) the parser vector
   *  @param input (INPIUT) the string to be parsed
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-09-05
   */
  // ==========================================================================
  StatusCode parse
  ( std::vector<Gaudi::XYZPoint>&  result  ,
    const std::string&             input   )
  {
    result.clear() ;
    return parse_(result, input);
  }
  // ==========================================================================
  /*  parse the vector of vectors
   *  @param resut (OUTPUT) the parser vector
   *  @param input (INPIUT) the string to be parsed
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-09-05
   */
  // ==========================================================================
  StatusCode parse
  ( std::vector<Gaudi::XYZVector>&  result  ,
    const std::string&              input   )
  {
    result.clear() ;
    return parse_(result, input);
  }

#ifndef _WIN32
  // ==========================================================================
  /*  parse the vector of vectors
   *  @param resut (OUTPUT) the parser vector
   *  @param input (INPIUT) the string to be parsed
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-09-05
   */
  // ==========================================================================
  StatusCode parse
  ( std::vector<Gaudi::LorentzVector>&  result  ,
    const std::string&              input   )
  {
    return parse_(result, input);
  }
#endif
} } // namespace Gaudi::Parsers
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
