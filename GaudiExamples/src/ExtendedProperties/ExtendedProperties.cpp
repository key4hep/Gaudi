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
  StatusCode execute();

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
  Gaudi::Property<std::array<double, 3>> m_23{this, "StdArrayDouble3"};

  Gaudi::Property<GaudiUtils::Map<std::string, std::string>> m_24{this, "GaudiMapSS"};
};
// ============================================================================
/// factory
// ============================================================================
DECLARE_COMPONENT( ExtendedProperties )
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
  SimpleProperty<short> m1( "a", 0 );
  SimpleProperty<double> m2( "b", m1 );

  // some properties could be assigned from other (convertible) types
  SimpleProperty<int> m3( "c", 0 );
  m3 = m1;

  float i = 10;
  SimplePropertyRef<float> m4( "d", i );

  m4 = 12;

  return StatusCode::SUCCESS;
}
// ============================================================================
// The END
// ============================================================================
