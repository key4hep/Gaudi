// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <array>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "GaudiKernel/Map.h"

// ============================================================================
// Include parsers for creating parser that handles tuple
// ============================================================================
#include "GaudiKernel/ParsersFactory.h"
#include "GaudiKernel/StdArrayAsProperty.h"

// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning( disable : 177 )
#endif

/** @file
 *  simple DEMO-file for "extended properties",
 *  implementation file for class ExtendedProperties
 *  @author Alexander MAZUROV alexander.mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date  2006-03-21
 */

/** @class ExtendedProperties
 *  simple DEMO-file for "extended properties",
 *  implementation file for class ExtendedProperties
 *  @author Alexander MAZUROV alexander.mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date  2006-03-21
 */

class ExtendedProperties : public GaudiAlgorithm
{
public:
  StatusCode execute() override;

  ExtendedProperties( const std::string& name, ISvcLocator* pSvc ) : GaudiAlgorithm( name, pSvc )
  {
    setProperty( "PropertiesPrint", "true" ).ignore();

    m_20["key"] = "value";
    m_21.value().push_back( 123 );
  }
  /// destructor
  ~ExtendedProperties() override = default;

private:
  Gaudi::Property<std::pair<double, double>> m_1{this, "PairDD"};

  Gaudi::Property<std::vector<std::pair<double, double>>> m_2{this, "VectorOfPairsDD"};
  Gaudi::Property<std::vector<std::vector<std::string>>> m_3{this, "VectorOfVectorsString"};
  Gaudi::Property<std::vector<std::vector<double>>> m_4{this, "VectorOfVectorsDouble"};

  Gaudi::Property<std::map<int, double>> m_5{this, "MapIntDouble"};
  Gaudi::Property<std::map<std::string, std::string>> m_6{this, "MapStringString"};
  Gaudi::Property<std::map<std::string, int>> m_7{this, "MapStringInt"};
  Gaudi::Property<std::map<std::string, double>> m_8{this, "MapStringDouble"};
  Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_9{this, "MapStringVectorOfStrings"};

  Gaudi::Property<std::pair<int, int>> m_10{this, "PairII"};
  Gaudi::Property<std::map<std::string, std::vector<double>>> m_11{this, "MapStringVectorOfDoubles"};
  Gaudi::Property<std::map<std::string, std::vector<int>>> m_12{this, "MapStringVectorOfInts"};

  Gaudi::Property<std::map<int, int>> m_13{this, "MapIntInt"};
  Gaudi::Property<std::vector<std::pair<int, int>>> m_14{this, "VectorOfPairsII"};

  Gaudi::Property<std::map<int, std::string>> m_15{this, "MapIntString"};
  Gaudi::Property<std::map<unsigned int, std::string>> m_16{this, "MapUIntString"};

  Gaudi::Property<std::map<std::string, std::string>> m_20{this, "EmptyMap"};
  Gaudi::Property<std::vector<int>> m_21{this, "EmptyVector"};

  Gaudi::Property<std::tuple<std::string, int, double>> m_22{this, "TupleStringIntDouble"};
  // std::array must be explicitly initialized
  Gaudi::Property<std::array<double, 3>> m_23{this, "StdArrayDouble3", {0}};

  Gaudi::Property<GaudiUtils::Map<std::string, std::string>> m_24{this, "GaudiMapSS"};
};
// ============================================================================
/// factory
// ============================================================================
DECLARE_COMPONENT( ExtendedProperties )
// ============================================================================
namespace
{
  template <class TYPE>
  inline SimplePropertyRef<TYPE> _prop( TYPE& value )
  {
    // construct a readable name
    std::string name            = System::typeinfoName( typeid( value ) );
    std::string::size_type ipos = name.find( "std::" );
    while ( std::string::npos != ipos ) {
      name.erase( ipos, 5 );
      ipos = name.find( "std::" );
    }
    ipos = name.find( "__cxx11::" );
    while ( std::string::npos != ipos ) {
      name.erase( ipos, 9 );
      ipos = name.find( "__cxx11::" );
    }
    ipos = name.find( " " );
    while ( std::string::npos != ipos ) {
      name.erase( ipos, 1 );
      ipos = name.find( " " );
    }
    ipos = name.find( "const" );
    while ( std::string::npos != ipos ) {
      name.erase( ipos, 5 );
      ipos = name.find( "const" );
    }
    ipos = name.find( ",allocator<" );
    while ( std::string::npos != ipos ) {
      std::string::size_type ip2 = ipos + 11;
      int ip3                    = 1;
      for ( ; ip2 < name.size(); ++ip2 ) {
        if ( '<' == name[ip2] ) {
          ip3 += 1;
        }
        if ( '>' == name[ip2] ) {
          ip3 -= 1;
        }
        if ( 0 == ip3 ) {
          break;
        }
      }
      name.erase( ipos, ip2 + 1 - ipos );
      ipos = name.find( ",allocator<" );
    }
    if ( std::string::npos != name.find( "map<" ) ) {
      ipos = name.find( ",less<" );
      while ( std::string::npos != ipos ) {
        std::string::size_type ip2 = ipos + 6;
        int ip3                    = 1;
        for ( ; ip2 < name.size(); ++ip2 ) {
          if ( '<' == name[ip2] ) {
            ip3 += 1;
          }
          if ( '>' == name[ip2] ) {
            ip3 -= 1;
          }
          if ( 0 == ip3 ) {
            break;
          }
        }
        name.erase( ipos, ip2 + 1 - ipos );
        ipos = name.find( ",less<" );
      }
    }
    ipos = name.find( ">>" );
    while ( std::string::npos != ipos ) {
      name.replace( ipos, 2, "> >" );
      ipos = name.find( ">>" );
    }
    return SimplePropertyRef<TYPE>( name, value );
  }
}
// ============================================================================
StatusCode ExtendedProperties::execute()
{
  always() << " My Properties : " << endmsg;

  always() << " \t" << m_1 << endmsg;
  always() << " \t" << m_2 << endmsg;
  always() << " \t" << m_3 << endmsg;
  always() << " \t" << m_4 << endmsg;
  always() << " \t" << m_5 << endmsg;
  always() << " \t" << m_6 << endmsg;
  always() << " \t" << m_7 << endmsg;
  always() << " \t" << m_8 << endmsg;
  always() << " \t" << m_9 << endmsg;
  always() << " \t" << m_10 << endmsg;
  always() << " \t" << m_11 << endmsg;
  always() << " \t" << m_12 << endmsg;
  always() << " \t" << m_13 << endmsg;
  always() << " \t" << m_14 << endmsg;
  always() << " \t" << m_14 << endmsg;
  always() << " \t" << m_15 << endmsg;
  always() << " \t" << m_16 << endmsg;

  always() << " \t" << m_20 << endmsg;
  always() << " \t" << m_21 << endmsg;

  always() << " \t" << m_22 << endmsg;
  always() << " \t" << m_23 << endmsg;

  always() << " \t" << m_24 << endmsg;

  // some properties could be created from other (convertible) types:
  Gaudi::Property<short> m1( "a", 0 );
  Gaudi::Property<double> m2( "b", m1 );

  // some properties could be assigned from other (convertible) types
  Gaudi::Property<int> m3( "c", 0 );
  m3 = m1;

  float i = 10;
  Gaudi::Property<float&> m4( "d", i );

  m4 = 12;

  return StatusCode::SUCCESS;
}
// ============================================================================
// The END
// ============================================================================
