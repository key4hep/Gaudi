// $Id: Chrono.h,v 1.6 2008/05/13 12:36:54 marcocle Exp $
// ============================================================================
#ifndef     GAUDIKERNEL_CHRONO_H
#define     GAUDIKERNEL_CHRONO_H
// ============================================================================
// Include files
// ============================================================================
// STD& STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IChronoSvc.h"
#include "GaudiKernel/ChronoEntity.h"
// ============================================================================
/** @class Chrono GaudiKernel/Chrono.h
 *
 * A small utility class for chronometry of user codes
 * @see  ChronoEntity
 * @see IChronoSvc
 *
 * @author Vanya Belyaev
 * @date    Nov 26, 1999
 */
class GAUDI_API Chrono
{
public:
  // =========================================================================
  /** Constructor from Chrono Service and the tag
   *
   *  @code
   *
   *  IChronoSvc* svc = ... ;
   *
   *  { // start the scope
   *    Chrono chrono ( svc , "some unique tag here" ) ;
   *
   *    for ( long i = 0 ; i < 10000000 ; ++i )
   *     {
   *        .. put some CPU-intensive computations here
   *     }
   *
   *  } // end of the scope, destroy chrono
   *
   *  @endcode
   *
   *  @param svc pointer to Chrono Service
   *  @param tag the unique tag
   */
  Chrono
  ( IChronoSvc*        svc = 0 ,                  // the service
    const std::string& tag = "CHRONO::UNNAMED" )  // the unique tag/name
    : m_chrono ( 0 )
  { if ( 0 != svc ) { m_chrono = svc -> chronoStart ( tag ) ; } }
  // =========================================================================
  /** Constructor from Chrono Service and the tag
   *
   *  @code
   *
   *  IChronoSvc* svc = ... ;
   *
   *  { // start the scope
   *    Chrono chrono ( "some unique tag here" , svc ) ;
   *
   *    for ( long i = 0 ; i < 10000000 ; ++i )
   *     {
   *        .. put some CPU-intensive computations here
   *     }
   *
   *  } // end of the scope, destroy chrono
   *
   *  @endcode
   *
   *  @param tag the unique tag
   *  @param svc pointer to Chrono Service
   */
  Chrono
  ( const std::string& tag ,   // the unique tag/name
    IChronoSvc*        svc )   // the service
    : m_chrono ( 0 )
  { if ( 0 != svc ) { m_chrono = svc -> chronoStart ( tag ) ; } }
  // =========================================================================
  /** Constructor from Chrono Object/Entity
   *
   *  @code
   *
   *  ChronoEntity* chronometer = ... ;
   *
   *  { // start the scope
   *    Chrono chrono ( chronometer ) ;
   *
   *    for ( long i = 0 ; i < 10000000 ; ++i )
   *     {
   *        .. put some CPU-intensive computations here
   *     }
   *
   *  } // end of the scope, destroy chrono
   *
   *  @endcode
   *
   *  @param c the pointer to Chrono Object/Entity
   */
  Chrono ( ChronoEntity* c ) : m_chrono ( c )
  { if ( 0 != m_chrono ) { m_chrono -> start () ; } }
  // =========================================================================
  /** Constructor from Chrono Object/Entity
   *
   *  @code
   *
   *  ChronoEntity m_chrono = ... ;
   *
   *  { // start the scope
   *    Chrono chrono ( m_chrono ) ;
   *
   *    for ( long i = 0 ; i < 10000000 ; ++i )
   *     {
   *        .. put some CPU-intensive computations here
   *     }
   *
   *  } // end of the scope, destroy chrono
   *
   *  @endcode
   *
   *  @param c the reference to Chrono Object/Entity
   */
  Chrono ( ChronoEntity& c ) : m_chrono ( &c ) { m_chrono -> start () ; }
  // =========================================================================
  /// Destructor , stop the chrono
  ~Chrono () { if ( 0 != m_chrono ) { m_chrono->stop() ; } }
  // =========================================================================
private:
  // =========================================================================
  /// the copy constructor is disabled
  Chrono           ( const Chrono& ) ;  // no copy constructor
  /// the assignment operator is disabled
  Chrono& operator=( const Chrono& ) ;  // no assignment
  // =========================================================================
private:
  // ==========================================================================
  /// The actual chronometer
  ChronoEntity* m_chrono; // The actual chronometer
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif  //  GAUDIKERNEL_CHRONO_H
// ============================================================================


