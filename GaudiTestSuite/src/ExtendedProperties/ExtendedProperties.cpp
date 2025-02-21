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
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <array>
#include <map>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include <GaudiKernel/Map.h>

// ============================================================================
// Include parsers for creating parser that handles tuple
// ============================================================================
#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/StdArrayAsProperty.h>

#include <GaudiKernel/Algorithm.h>
// ============================================================================

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#  pragma warning( disable : 177 )
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

class ExtendedProperties : public Algorithm {
public:
  StatusCode execute() override;

  ExtendedProperties( const std::string& name, ISvcLocator* pSvc ) : Algorithm( name, pSvc ) {
    setProperty( "PropertiesPrint", true ).ignore();

    m_20["key"] = "value";
    m_21.value().push_back( 123 );
  }

private:
  Gaudi::Property<std::pair<double, double>> m_1{ this, "PairDD" };

  Gaudi::Property<std::vector<std::pair<double, double>>> m_2{ this, "VectorOfPairsDD" };
  Gaudi::Property<std::vector<std::vector<std::string>>>  m_3{ this, "VectorOfVectorsString" };
  Gaudi::Property<std::vector<std::vector<double>>>       m_4{ this, "VectorOfVectorsDouble" };

  Gaudi::Property<std::map<int, double>>                           m_5{ this, "MapIntDouble" };
  Gaudi::Property<std::map<std::string, std::string>>              m_6{ this, "MapStringString" };
  Gaudi::Property<std::map<std::string, int>>                      m_7{ this, "MapStringInt" };
  Gaudi::Property<std::map<std::string, double>>                   m_8{ this, "MapStringDouble" };
  Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_9{ this, "MapStringVectorOfStrings" };

  Gaudi::Property<std::pair<int, int>>                        m_10{ this, "PairII" };
  Gaudi::Property<std::map<std::string, std::vector<double>>> m_11{ this, "MapStringVectorOfDoubles" };
  Gaudi::Property<std::map<std::string, std::vector<int>>>    m_12{ this, "MapStringVectorOfInts" };

  Gaudi::Property<std::map<int, int>>               m_13{ this, "MapIntInt" };
  Gaudi::Property<std::vector<std::pair<int, int>>> m_14{ this, "VectorOfPairsII" };

  Gaudi::Property<std::map<int, std::string>>          m_15{ this, "MapIntString" };
  Gaudi::Property<std::map<unsigned int, std::string>> m_16{ this, "MapUIntString" };

  Gaudi::Property<std::map<std::string, std::string>> m_20{ this, "EmptyMap" };
  Gaudi::Property<std::vector<int>>                   m_21{ this, "EmptyVector" };

  Gaudi::Property<std::tuple<std::string, int, double>> m_22{ this, "TupleStringIntDouble" };
  Gaudi::Property<std::tuple<std::string>>              m_25{ this, "TupleString" };

  Gaudi::Property<std::unordered_set<int>>         m_30{ this, "SetOfInt" };
  Gaudi::Property<std::unordered_set<std::string>> m_31{ this, "SetOfString" };

  // std::array must be explicitly initialized
  Gaudi::Property<std::array<double, 3>> m_23{ this, "StdArrayDouble3", { 0 } };
  Gaudi::Property<std::array<int, 1>>    m_26{ this, "StdArrayInt1", { 0 } };

  Gaudi::Property<GaudiUtils::Map<std::string, std::string>> m_24{ this, "GaudiMapSS" };

  using Triplet = std::tuple<std::string, std::string, std::string>;
  static std::hash<std::string> hasher;
  struct HashFunction {
    std::size_t operator()( Triplet const& entry ) const {
      return hasher( std::get<0>( entry ) ) ^ hasher( std::get<1>( entry ) ) ^ hasher( std::get<2>( entry ) );
    }
  };

  Gaudi::Property<std::vector<Triplet>>                      m_tuplevector{ this, "PTupleVector" };
  Gaudi::Property<std::set<Triplet>>                         m_tupleset{ this, "PTupleSet" };
  Gaudi::Property<std::unordered_set<Triplet, HashFunction>> m_tupleunset{ this, "PTupleUnSet" };
  using VS = std::vector<std::string>;
  Gaudi::Property<std::tuple<int, VS>> m_intvectortuple{ this, "PIntVectorTuple" };
  using SS = std::set<std::string>;
  Gaudi::Property<std::tuple<int, SS>> m_intsettuple{ this, "PIntSetTuple" };
  using USS = std::unordered_set<std::string>;
  Gaudi::Property<std::tuple<int, USS>> m_intunsettuple{ this, "PIntUnSetTuple" };
};
// ============================================================================
/// factory
// ============================================================================
DECLARE_COMPONENT( ExtendedProperties )
// ============================================================================
StatusCode ExtendedProperties::execute() {
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
  always() << " \t" << m_25 << endmsg;
  always() << " \t" << m_23 << endmsg;
  always() << " \t" << m_26 << endmsg;

  always() << " \t" << m_24 << endmsg;

  always() << " \t" << m_30 << endmsg;
  always() << " \t" << m_31 << endmsg;

  always() << " \t" << m_tuplevector << endmsg;
  always() << " \t" << m_intvectortuple << endmsg;
  always() << " \t" << m_tupleset << endmsg;
  always() << " \t" << m_intsettuple << endmsg;
  always() << " \t" << m_tupleunset << endmsg;
  always() << " \t" << m_intunsettuple << endmsg;

  // some properties could be created from other (convertible) types:
  Gaudi::Property<short>  m1( "a", 0 );
  Gaudi::Property<double> m2( "b", m1 );

  // some properties could be assigned from other (convertible) types
  Gaudi::Property<int> m3( "c", 0 );
  m3 = m1;

  float                   i = 10;
  Gaudi::Property<float&> m4( "d", i );

  m4 = 12;

  return StatusCode::SUCCESS;
}
// ============================================================================
// The END
// ============================================================================
