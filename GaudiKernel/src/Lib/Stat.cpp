// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <sstream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/IStatSvc.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StatEntity.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
// ============================================================================
/** @file
 *  Implementation file for class Stat
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2005-08-02
 *  @date 2007-07-08
 */
// ============================================================================
/*   Constructor from IStatSvc,tag and value
 *
 *   @code
 *
 *   IStatSvc* statSvc = ... ;
 *   double eTotal = .... ;
 *
 *   Stat eTot ( statSvc , "total energy" ) ;
 *   eTot += eTotal ;
 *
 *   @endcode
 *
 *   @see IStatSvc
 *   @param svc pointer to Chrono&Stat Service
 *   @paran tag unique tag for the entry
 */
// ============================================================================
Stat::Stat( IStatSvc* svc, const std::string& tag ) : m_tag( tag ), m_group(), m_stat( svc )
{
  if ( m_stat ) {
    // get from the service
    const StatEntity* tmp = m_stat->stat( tag );
    if ( !tmp ) {
      // create if needed
      m_stat->stat( tag, 0 );
      tmp             = m_stat->stat( tag );
      StatEntity* aux = const_cast<StatEntity*>( tmp );
      aux->reset();
    }
    m_entity = const_cast<StatEntity*>( tmp );
  }
}
// ============================================================================
/*   Constructor from IStatSvc,tag and value
 *
 *   @code
 *
 *   IStatSvc* statSvc = ... ;
 *   double eTotal = .... ;
 *
 *   Stat stat( statSvc , "total energy" , eTotal ) ;
 *
 *   @endcode
 *
 *   @see IStatSvc
 *   @param svc pointer to Chrono&Stat Service
 *   @paran tag unique tag for the entry
 *   @param flag    "flag"(additive quantity) to be used
 */
// ============================================================================
Stat::Stat( IStatSvc* svc, const std::string& tag, const double flag ) : m_tag( tag ), m_group(), m_stat( svc )
{
  if ( m_stat ) {
    m_stat->stat( tag, flag );
    // get from the service
    m_entity = const_cast<StatEntity*>( m_stat->stat( tag ) );
  }
}
// ============================================================================
/*  constructor from ICounterSvc, group and name
 *  @see ICounterSvc::get
 *  @see ICounterSvc::create
 *  @param svc pointer to  Counter Service
 *  @param group group name
 *  @param name  counter name
 */
// ============================================================================
Stat::Stat( ICounterSvc* svc, const std::string& group, const std::string& name )
    : m_tag( name ), m_group( group ), m_counter( svc )
{
  if ( m_counter ) {
    // get from the service
    m_entity = m_counter->get( group, name );
    // create if needed:
    if ( !m_entity ) {
      m_counter->create( group, name, 0, m_entity );
    }
  }
}

// ============================================================================
// representation as string
// ============================================================================
std::string Stat::toString() const
{
  std::ostringstream ost;
  print( ost );
  return ost.str();
}
// ============================================================================
/*  printout to std::ostream
 *  @param s the reference to the output stream
 */
// ============================================================================
std::ostream& Stat::print( std::ostream& o ) const
{
  if ( m_group.empty() && m_tag.empty() ) {
    return !m_entity ? ( o << "NULL" ) : ( o << m_entity );
  }
  if ( !m_group.empty() ) {
    if ( m_entity ) {
      return o << boost::format( " %|1$15s|::%|2$-15s| %|32t|%3%" ) % ( "\"" + m_group ) % ( m_tag + "\"" ) %
                      ( *m_entity );
    } else {
      return o << boost::format( " %|1$15s|::%|2$-15s| %|32t|%NULL%" ) % ( "\"" + m_group ) % ( m_tag + "\"" );
    }
  }
  if ( m_entity ) {
    return o << boost::format( " %|1$=30s| %|32t|%2%" ) % ( "\"" + m_tag + "\"" ) % ( *m_entity );
  }
  return o << boost::format( " %|1$=30s| %|32t|%NULL%" ) % ( "\"" + m_tag + "\"" );
}
// ============================================================================
// external operator for addition of Stat and a number
// ============================================================================
Stat operator+( const Stat& stat, const double value )
{
  Stat s( stat );
  s += value;
  return s;
}
// ============================================================================
// external operator for subtraction of Stat and a number
// ============================================================================
Stat operator-( const Stat& stat, const double value )
{
  Stat s( stat );
  s -= value;
  return s;
}
// ============================================================================
// external operator for addition of Stat and a number
// ============================================================================
Stat operator+( const double value, const Stat& stat )
{
  Stat s( stat );
  s += value;
  return s;
}
// ============================================================================
// external operator for addition of Stat and Stat
Stat operator+( const Stat& stat, const Stat& value )
{
  Stat s( stat );
  s += value;
  return s;
}
// ============================================================================
// external printout operator to std::ostream
std::ostream& operator<<( std::ostream& stream, const Stat& stat ) { return stat.print( stream ); }
// ============================================================================

// ============================================================================
// The END
// ============================================================================
