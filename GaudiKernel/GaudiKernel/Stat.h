// $Id: Stat.h,v 1.6 2008/04/04 18:32:39 marcocle Exp $
// ============================================================================
#ifndef      __GAUDI_CHRONOSTATSVC_STAT_H__
#define      __GAUDI_CHRONOSTATSVC_STAT_H__
// ============================================================================
// Iinclude files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatEntity.h"
// ============================================================================
// forward declarations
// ============================================================================
class IStatSvc    ;
class ICounterSvc ;
// ============================================================================
/** @class Stat  Stat.h GaudiKernel/Stat.h
 *
 *  Small wrapper class for easy manipulation with generic counters
 *   and IStatSvc&ICounterSvc interface
 *
 *  It acts as "smart pointer" fro StatEntity objects, and allows
 *  manipulation with StatEntity objects, owned by
 *  GaudiCommon<TYPE> base class and/or IStatSvc/ICounterSvc
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
class GAUDI_API Stat
{
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
  Stat ( StatEntity*        entity = 0   ,
         const std::string& name   = ""  ,
         const std::string& group  = ""  )
    : m_entity  ( entity )
    , m_tag     ( name   )
    , m_group   ( group  )
    , m_stat    ( 0      )
    , m_counter ( 0      )
  {}
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
  Stat ( StatEntity&        entity       ,
         const std::string& name   = ""  ,
         const std::string& group  = ""  )
    : m_entity  ( &entity )
    , m_tag     ( name    )
    , m_group   ( group   )
    , m_stat    ( 0       )
    , m_counter ( 0       )
  {}
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
  Stat ( IStatSvc*          svc  ,
         const std::string& tag  ) ;
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
  Stat ( IStatSvc*          svc  ,
         const std::string& tag  ,
         const double       flag ) ;
  /** constructor from ICounterSvc, group and name
   *
   *  @code
   *
   *   ICounterSvc* svc = ... ;
   *
   *   // get/create the counter from Counter Service
   *   Stat stat( svc , "ECAL" , "TotalEnergy" ) ;
   *
   *  @endcode
   *
   *  @see ICounterSvc::get
   *  @see ICounterSvc::create
   *  @param svc pointer to  Counter Service
   *  @param group group name
   *  @param name  counter name
   */
  Stat ( ICounterSvc*       svc   ,
         const std::string& group ,
         const std::string& name  ) ;
  /// copy constructor
  Stat           ( const Stat& right ) ;
  /// Assignement operator
  Stat& operator=( const Stat& right) ;
  /// destructor
  ~Stat() ;
  // ==========================================================================
  /// get the entity
  const StatEntity*  entity    () const { return  m_entity     ; }
  /// dereference operaqtor
  const StatEntity*  operator->() const { return    entity()   ; }
  /// cast to StatEntity
  operator const StatEntity&   () const { return   *entity()   ; }
  /// check validity
  bool                operator!() const { return 0 == m_entity ; }
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
  Stat& operator+= ( const double f )
  {
    if ( 0 != m_entity ) { (*m_entity) += f ; }
    return *this ;
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
  Stat& operator++ ()
  {
    if ( 0 != m_entity ) { ++(*m_entity) ; }
    return *this ;
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
  Stat& operator++ (int)
  {
    if ( 0 != m_entity ) { (*m_entity)++ ; }
    return *this ;
  }
  /** General decrement operator for the counter
   *  @see StatEntity
   *  @return self-reference
   *  @param f counter decrement
   */
  Stat& operator-= ( const double   f )
  {
    if ( 0 != m_entity ) { (*m_entity) -= f ; }
    return *this ;
  }
  /// Pre-decrement operator for the flag
  Stat& operator-- ()
  {
    if ( 0 != m_entity ) { --(*m_entity)   ; }
    return *this ;
  }
  /// Post-decrement operator for the flag
  Stat& operator-- (int)
  {
    if ( 0 != m_entity ) {   (*m_entity)-- ; }
    return *this ;
  }
  /// increment with StatEntity object
  Stat& operator+=( const StatEntity& right )
  {
    if ( 0 != m_entity ) { (*m_entity) += right ; }
    return *this ;
  }
  /// increment with  other stat objects
  Stat& operator+=( const Stat& right )
  {
    if ( 0 != right.entity() ) { (*this) += *right.entity() ; }
    return *this ;
  }
  // ==========================================================================
  /// representation as string
  std::string toString() const ;
  /** printout to std::ostream
   *  @param s the reference to the output stream
   *  @return the reference to the output stream
   */
  std::ostream& print( std::ostream& o = std::cout ) const ;
  /** printout  to std::ostream
   *  @param s the reference to the output stream
   *  @return the reference to the output stream
   */
  std::ostream& fillStream ( std::ostream& o ) const { return print ( o ) ; }
  // ==========================================================================
  /// alternative access to underlying counter (for ICounterSvc::CounterObj)
  StatEntity*        counter   () const { return  m_entity   ; }
  /// counter name
  const std::string& name      () const { return  m_tag      ; }
  /// counter group (for ICounterSvc)
  const std::string& group     () const { return  m_group    ; }
  // ==========================================================================
private:
  // underlying counter
  StatEntity*  m_entity  ;   ///< underlying counter
  // unique stat tag(name)
  std::string  m_tag     ;   ///< unique stat tag(name)
  // group (for ICounterSvc)
  std::string  m_group   ;
  // Stat  service
  IStatSvc*    m_stat    ;   ///< Stat  service
  // Counter Svc
  ICounterSvc* m_counter ;   ///< Counter Service
};
// ============================================================================
/// external operator for addition of Stat and a number
GAUDI_API Stat operator+( const Stat&  stat  , const double value  ) ;
// ============================================================================
/// external operator for subtraction of Stat and a number
GAUDI_API Stat operator-( const Stat&  stat  , const double value  ) ;
// ============================================================================
/// external operator for addition of Stat and a number
GAUDI_API Stat operator+( const double value , const Stat&  stat   ) ;
// ============================================================================
/// external operator for addition of Stat and Stat
GAUDI_API Stat operator+( const Stat&  stat  , const Stat&  value ) ;
// ============================================================================
/// external printout operator to std::ostream
GAUDI_API std::ostream& operator<<( std::ostream& stream , const Stat& stat ) ;
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif    // __GAUDI_CHRONOSTATSVC_STAT_H__
// ============================================================================

