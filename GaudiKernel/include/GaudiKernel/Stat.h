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
#pragma once
// ============================================================================
// Iinclude files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/IStatSvc.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/StatEntity.h>
// ============================================================================
/** @class Stat  Stat.h GaudiKernel/Stat.h
 *
 *  Small wrapper class for easy manipulation with generic counters
 *   and IStatSvc interface
 *
 *  It acts as "smart pointer" fro StatEntity objects, and allows
 *  manipulation with StatEntity objects, owned by
 *  GaudiCommon<TYPE> base class and/or IStatSvc
 *
 *   @code
 *
 *   long nTracks = ... ;
 *   Stat stat( chronoSvc() , "#tracks" , nTracks ) ;
 *
 *   @endcode
 *
 *  Alternatively one can use operator methods:
 *
 *   @code
 *
 *   long nTracks = ... ;
 *   Stat stat( chronoSvc() , "#tracks" ) ;
 *   stat += nTracks ;
 *
 *   @endcode
 *
 *   @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *   @date 2007-08-02
 */
class GAUDI_API Stat {
public:
  /**  constructor from StatEntity, name and group :
   *
   *   @code
   *
   *    StatEntity* entity = ... ;
   *
   *    // make helper object:
   *    Stat stat ( entity ) ;
   *
   *   @endcode
   *
   *   @see StatEntity
   *   @param entity pointer to entity object
   *   @param name  (optional) name of the object, for printout
   *   @param group (optional) group of the object, for printout
   */
  Stat( StatEntity* entity = 0, const std::string& name = "", const std::string& group = "" )
      : m_entity( entity ), m_tag( name ), m_group( group ) {}
  /**  constructor from StatEntity, name and group :
   *
   *   @code
   *
   *    // make helper object:
   *    Stat stat = Stat( countter("Name") , "Name" ) ;
   *
   *   @endcode
   *   @see StatEntity
   *   @see GaudiCommon::counter
   *   @param entity reference to entity object
   *   @param name  (optional) name of the object, for printout
   *   @param group (optional) group of the object, for printout
   */
  Stat( StatEntity& entity, const std::string& name = "", const std::string& group = "" )
      : m_entity( &entity ), m_tag( name ), m_group( group ) {}
  /**  constructor from IStatSvc, tag and value
   *
   *   @code
   *
   *   IStatSvc* svc = ... ;
   *   double eTotal = .... ;
   *
   *   // get/create the counter from Stat Service
   *   Stat eTot ( svc , "total energy" ) ;
   *
   *   eTot += eTotal ;
   *
   *   @endcode
   *
   *   @see IStatSvc
   *   @param svc pointer to Chrono&Stat Service
   *   @paran tag unique tag for the entry
   */
  Stat( IStatSvc* svc, const std::string& tag );
  /**  constructor from IStatSvc, tag and value
   *
   *   @code
   *
   *   IStatSvc* svc = ... ;
   *   double eTotal = .... ;
   *
   *   // get/create the counter from Stat Service
   *   Stat stat( svc , "total energy" , eTotal ) ;
   *
   *   @endcode
   *
   *   @see IStatSvc
   *   @param svc pointer to Chrono&Stat Service
   *   @paran tag unique tag for the entry
   *   @param flag    "flag"(additive quantity) to be used
   */
  Stat( IStatSvc* svc, const std::string& tag, const double flag );
  /// copy constructor
  Stat( const Stat& ) = default;
  /// Assignement operator
  Stat& operator=( const Stat& ) = default;
  /// destructor
  ~Stat() = default;
  // ==========================================================================
  /// get the entity
  const StatEntity* entity() const { return m_entity; }
  /// dereference operaqtor
  const StatEntity* operator->() const { return entity(); }
  /// cast to StatEntity
  operator const StatEntity&() const { return *entity(); }
  /// check validity
  bool operator!() const { return 0 == m_entity; }
  // ==========================================================================
  /** General increment for the counter
   *
   *  @code
   *
   *    Stat stat = ... ;
   *
   *    const long nTracks = ... ;
   *
   *    stat += nTracks ;
   *
   *  @endcode
   *
   *  @see StatEntity
   *  @param f value to be added to the counter
   *  @return selfreference
   */
  Stat& operator+=( const double f ) {
    if ( m_entity ) { ( *m_entity ) += f; }
    return *this;
  }
  /** Pre-increment operator for the counter
   *
   *  @code
   *
   *    Stat stat = ... ;
   *
   *    ++stat ;
   *
   *  @endcode
   *
   *  @see StatEntity
   *  @return selfreference
   */
  Stat& operator++() {
    if ( m_entity ) { ++( *m_entity ); }
    return *this;
  }
  /** Post-increment operator for the counter
   *
   *  @code
   *
   *    Stat stat = ... ;
   *
   *    stat++ ;
   *
   *  @endcode
   *  @see StatEntity
   *  @return self-reference
   */
  Stat& operator++( int ) {
    if ( m_entity ) { ( *m_entity )++; }
    return *this;
  }
  /** General decrement operator for the counter
   *  @see StatEntity
   *  @return self-reference
   *  @param f counter decrement
   */
  Stat& operator-=( const double f ) {
    if ( m_entity ) { ( *m_entity ) -= f; }
    return *this;
  }
  /// Pre-decrement operator for the flag
  Stat& operator--() {
    if ( m_entity ) { --( *m_entity ); }
    return *this;
  }
  /// Post-decrement operator for the flag
  Stat& operator--( int ) {
    if ( m_entity ) { ( *m_entity )--; }
    return *this;
  }
  /// increment with StatEntity object
  Stat& operator+=( const StatEntity& right ) {
    if ( m_entity ) { ( *m_entity ) += right; }
    return *this;
  }
  /// increment with  other stat objects
  Stat& operator+=( const Stat& right ) {
    if ( 0 != right.entity() ) { ( *this ) += *right.entity(); }
    return *this;
  }
  // ==========================================================================
  /// representation as string
  std::string toString() const;
  /** printout to std::ostream
   *  @param s the reference to the output stream
   *  @return the reference to the output stream
   */
  std::ostream& print( std::ostream& o = std::cout ) const;
  /** printout  to std::ostream
   *  @param s the reference to the output stream
   *  @return the reference to the output stream
   */
  std::ostream& fillStream( std::ostream& o ) const { return print( o ); }
  // ==========================================================================
  /// alternative access to underlying counter
  StatEntity* counter() const { return m_entity; }
  /// counter name
  const std::string& name() const { return m_tag; }
  /// counter group
  const std::string& group() const { return m_group; }
  // ==========================================================================
private:
  // underlying counter
  StatEntity* m_entity = nullptr; ///< underlying counter
  // unique stat tag(name)
  std::string m_tag; ///< unique stat tag(name)
  // group
  std::string m_group;
  // Stat  service
  SmartIF<IStatSvc> m_stat; ///< Stat  service
};
// ============================================================================
/// external operator for addition of Stat and a number
GAUDI_API Stat operator+( const Stat& stat, const double value );
// ============================================================================
/// external operator for subtraction of Stat and a number
GAUDI_API Stat operator-( const Stat& stat, const double value );
// ============================================================================
/// external operator for addition of Stat and a number
GAUDI_API Stat operator+( const double value, const Stat& stat );
// ============================================================================
/// external operator for addition of Stat and Stat
GAUDI_API Stat operator+( const Stat& stat, const Stat& value );
// ============================================================================
/// external printout operator to std::ostream
GAUDI_API std::ostream& operator<<( std::ostream& stream, const Stat& stat );
// ============================================================================

// ============================================================================
// The END
// ============================================================================
