/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/ToStream.h>
#include <GaudiKernel/VectorsAsProperty.h>

namespace {
  typedef std::map<std::string, double> MAP;

  template <unsigned int N>
  inline MAP::const_iterator find( const MAP& m, const std::string ( &keys )[N] ) {
    for ( unsigned int i = 0; i < N; ++i ) {
      auto found = m.find( keys[i] );
      if ( m.end() != found ) { return found; }
    }
    return m.end();
  }
} // namespace
namespace Gaudi {
  namespace Parsers {
    template <typename T1, typename T2>
    inline StatusCode parse_( ROOT::Math::PositionVector3D<T1, T2>& result, std::string_view input ) {
      Skipper                                                                              skipper;
      typename Grammar_<IteratorT, ROOT::Math::PositionVector3D<T1, T2>, Skipper>::Grammar g;
      IteratorT iter = input.begin(), end = input.end();
      if ( qi::phrase_parse( iter, end, g, skipper, result ) ) { return StatusCode::SUCCESS; }
      //@attention always
      return StatusCode::SUCCESS;
    }

    StatusCode parse( Gaudi::XYZPoint& result, std::string_view input ) { return parse_( result, input ); }

    /*  parse 3D-vector
     *  @param result (output) the parsed vector
     *  @param input  (input)  the input string
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    StatusCode parse( Gaudi::XYZVector& result, std::string_view input ) {

      Gaudi::XYZPoint point;
      StatusCode      sc = parse( point, input );
      if ( sc.isFailure() ) { return sc; }
      result = point;
      return StatusCode::SUCCESS;
    }

    StatusCode parse( Gaudi::LorentzVector& result, std::string_view input ) { return parse_( result, input ); }

    /*  parse the vector of points
     *  @param resut (OUTPUT) the parser vector
     *  @param input (INPIUT) the string to be parsed
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    StatusCode parse( std::vector<Gaudi::XYZPoint>& result, std::string_view input ) {
      result.clear();
      return parse_( result, input );
    }
    /*  parse the vector of vectors
     *  @param resut (OUTPUT) the parser vector
     *  @param input (INPIUT) the string to be parsed
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    StatusCode parse( std::vector<Gaudi::XYZVector>& result, std::string_view input ) {
      result.clear();
      return parse_( result, input );
    }

    /*  parse the vector of vectors
     *  @param resut (OUTPUT) the parser vector
     *  @param input (INPIUT) the string to be parsed
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    StatusCode parse( std::vector<Gaudi::LorentzVector>& result, std::string_view input ) {
      return parse_( result, input );
    }
  } // namespace Parsers
} // namespace Gaudi
std::ostream& Gaudi::Utils::toStream( const Gaudi::XYZPoint& obj, std::ostream& s ) {
  s << "( ";
  toStream( obj.X(), s );
  s << " , ";
  toStream( obj.Y(), s );
  s << " , ";
  toStream( obj.Z(), s );
  s << " )";
  return s;
}
std::ostream& Gaudi::Utils::toStream( const Gaudi::XYZVector& obj, std::ostream& s ) {
  s << "( ";
  toStream( obj.X(), s );
  s << " , ";
  toStream( obj.Y(), s );
  s << " , ";
  toStream( obj.Z(), s );
  s << " )";

  return s;
}
std::ostream& Gaudi::Utils::toStream( const Gaudi::LorentzVector& obj, std::ostream& s ) {

  s << "( ";
  toStream( obj.Px(), s, 12 );
  s << " , ";
  toStream( obj.Py(), s, 12 );
  s << " , ";
  toStream( obj.Pz(), s, 13 );
  s << " , ";
  toStream( obj.E(), s, 14 );
  s << " )";

  return s;
}
