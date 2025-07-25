/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
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
// from GaudiKernel
// ============================================================================
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/IRndmGenSvc.h>
#include <GaudiKernel/Map.h>
#include <GaudiKernel/RndmGenerators.h>
#include <GaudiKernel/System.h>
#include <GaudiKernel/ToStream.h>
#include <GaudiKernel/VectorMap.h>
// ============================================================================
#include <GaudiKernel/Algorithm.h>
// ============================================================================
/** @file
 *
 *  Simple example which shows various maps available in Gaudi
 *
 *  @see GaudiuUtils::VectorMap
 *  @see GaudiuUtils::Map
 *  @see GaudiuUtils::HshMap
 *
 *  @author Vanya BELYAEV ibelyav@physics.syr.edu
 *  @date 2006-11-26
 */
// ============================================================================
/** @class MapAlg
 *
 *  Simple algorithm which shows various "map-like" containers available in Gaudi
 *
 *  @see GaudiUtils::VectorMap
 *  @see GaudiUtils::Map
 *  @see GaudiUtils::HshMap
 *
 *  @author Vanya BELYAEV ibelyav@physics.syr.edu
 *  @date 2006-11-26
 */
class MapAlg : public Algorithm {
public:
  typedef int    Key;
  typedef double Value;

public:
  /// finalization
  StatusCode finalize() override {
    always() << endmsg << " FINALIZE " << endmsg << " \tTested maps : " << endmsg << " \t1) "
             << System::typeinfoName( typeid( m_map1 ) ) << endmsg << " \t2) "
             << System::typeinfoName( typeid( m_map2 ) ) << endmsg << " \t3) "
             << System::typeinfoName( typeid( m_map3 ) ) << endmsg << " \t4) "
             << System::typeinfoName( typeid( m_map4 ) ) << endmsg;
    // finalize the base class
    return Algorithm::finalize(); ///< finalize the base class
  };
  /// the main execution method
  StatusCode execute() override;

public:
  /** standard constructor
   *  @param name algoritmm instance name
   *  @param pSvc pointer to Servcie Locator
   */
  using Algorithm::Algorithm;

  /// The copy constructor is disabled
  MapAlg( const MapAlg& ) = delete;
  /// The assignement is disabled
  MapAlg& operator=( const MapAlg& ) = delete;

private:
  /// const-printout
  void print1( Key key ) const;
  /// non-const printout
  void print2( Key key );

private:
  typedef std::map<Key, Value>              Map1;
  typedef GaudiUtils::VectorMap<Key, Value> Map2;
  typedef GaudiUtils::Map<Key, Value>       Map3;
  typedef GaudiUtils::HashMap<Key, Value>   Map4;

  Map1 m_map1;
  Map2 m_map2;
  Map3 m_map3;
  Map4 m_map4;
};
// ==========================================================================
/// Declaration of the Algorithm Factory
// ==========================================================================
DECLARE_COMPONENT( MapAlg )

// ==========================================================================
/// Main execution
// ==========================================================================
StatusCode MapAlg::execute() {
  using namespace Gaudi::Utils;

  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
  Rndm::Numbers gauss2( randSvc(), Rndm::Gauss( 0.0, 10.0 ) );

  const Key   key    = Key( gauss2() );
  const Value value1 = Value( int( 100 * gauss() ) ) / 100.0;

  always() << " Inserting key " << toString( key ) << " 1st: "
           << " " << toString( m_map1.insert( std::make_pair( key, value1 ) ).second ) << " "
           << toString( m_map2.insert( std::make_pair( key, value1 ) ).second ) << " "
           << toString( m_map3.insert( std::make_pair( key, value1 ) ).second ) << " "
           << toString( m_map4.insert( std::make_pair( key, value1 ) ).second ) << endmsg;

  always() << "1 Map1: " << toString( m_map1 ) << endmsg;
  always() << "1 Map2: " << toString( m_map2 ) << endmsg;
  always() << "1 Map3: " << toString( m_map3 ) << endmsg;
  always() << "1 Map4: " << toString( m_map4 ) << endmsg;

  print1( (Key)1 );

  always() << "2 Map1: " << toString( m_map1 ) << endmsg;
  always() << "2 Map2: " << toString( m_map2 ) << endmsg;
  always() << "2 Map3: " << toString( m_map3 ) << endmsg;
  always() << "2 Map4: " << toString( m_map4 ) << endmsg;

  print2( (Key)7 );

  always() << "3 Map1: " << toString( m_map1 ) << endmsg;
  always() << "3 Map2: " << toString( m_map2 ) << endmsg;
  always() << "3 Map3: " << toString( m_map3 ) << endmsg;
  always() << "3 Map4: " << toString( m_map4 ) << endmsg;

  const Value value2 = gauss();

  always() << " Inserting key " << toString( key ) << " 2nd: "
           << " " << toString( m_map1.insert( std::make_pair( key, value2 ) ).second ) << " "
           << toString( m_map2.insert( std::make_pair( key, value2 ) ).second ) << " "
           << toString( m_map3.insert( std::make_pair( key, value2 ) ).second ) << " "
           << toString( m_map4.insert( std::make_pair( key, value2 ) ).second ) << endmsg;

  always() << "4 Map1: " << toString( m_map1 ) << endmsg;
  always() << "4 Map2: " << toString( m_map2 ) << endmsg;
  always() << "4 Map3: " << toString( m_map3 ) << endmsg;
  always() << "4 Map4: " << toString( m_map4 ) << endmsg;

  if ( 0 == ::labs( key ) % 2 ) {
    always() << " Erased : "
             << " " << toString( 0 != m_map1.erase( key ) ) << " " << toString( 0 != m_map2.erase( key ) ) << " "
             << toString( 0 != m_map3.erase( key ) ) << " " << toString( 0 != m_map4.erase( key ) ) << endmsg;
  }

  always() << "5 Map1: " << toString( m_map1 ) << endmsg;
  always() << "5 Map2: " << toString( m_map2 ) << endmsg;
  always() << "5 Map3: " << toString( m_map3 ) << endmsg;
  always() << "5 Map4: " << toString( m_map4 ) << endmsg;

  always() << " Count key    0 : "
           << " " << m_map1.count( 0 ) << " " << m_map2.count( 0 ) << " " << m_map3.count( 0 ) << " "
           << m_map4.count( 0 ) << endmsg;
  always() << " Count key    1 : "
           << " " << m_map1.count( 1 ) << " " << m_map2.count( 1 ) << " " << m_map3.count( 1 ) << " "
           << m_map4.count( 1 ) << endmsg;
  always() << " Count key    7 : "
           << " " << m_map1.count( 7 ) << " " << m_map2.count( 7 ) << " " << m_map3.count( 7 ) << " "
           << m_map4.count( 7 ) << endmsg;
  always() << " Count key -100 : "
           << " " << m_map1.count( -100 ) << " " << m_map2.count( -100 ) << " " << m_map3.count( -100 ) << " "
           << m_map4.count( -100 ) << endmsg;

  return StatusCode::SUCCESS;
}
// ============================================================================
void MapAlg::print1( MapAlg::Key key ) const {
  always() << "     CONST: "
           << "  map2['" << key << "']: " << m_map2[key] << ", map3['" << key << "']: " << m_map3[key] << ", map4['"
           << key << "']: " << m_map4[key] << endmsg;
}
// ============================================================================
void MapAlg::print2( MapAlg::Key key ) {
  always() << " NON-CONST: "
           << "  map1['" << key << "']: " << m_map1[key] << ", map2['" << key << "']: " << m_map2[key] << ", map3['"
           << key << "']: " << m_map3[key] << ", map4['" << key << "']: " << m_map4[key] << endmsg;
}
// ============================================================================

// ==========================================================================
// The END
// ==========================================================================
