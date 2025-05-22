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

#include <GaudiKernel/ChronoEntity.h>
#include <GaudiKernel/IChronoSvc.h>
#include <GaudiKernel/Kernel.h>
#include <string>

/** @class Chrono GaudiKernel/Chrono.h
 *
 * A small utility class for chronometry of user codes
 * @see  ChronoEntity
 * @see IChronoSvc
 *
 * @author Vanya Belyaev
 * @date    Nov 26, 1999
 */
class GAUDI_API Chrono {
public:
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
  Chrono( IChronoSvc*        svc = nullptr,            // the service
          const std::string& tag = "CHRONO::UNNAMED" ) // the unique tag/name
  {
    if ( svc ) { m_chrono = svc->chronoStart( tag ); }
  }

  Chrono( Chrono&& rhs ) {
    m_chrono     = rhs.m_chrono;
    rhs.m_chrono = nullptr;
  }

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
  Chrono( const std::string& tag, // the unique tag/name
          IChronoSvc*        svc )       // the service
  {
    if ( svc ) { m_chrono = svc->chronoStart( tag ); }
  }

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
  Chrono( ChronoEntity* c ) : m_chrono( c ) {
    if ( m_chrono ) { m_chrono->start(); }
  }

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
  Chrono( ChronoEntity& c ) : m_chrono( &c ) { m_chrono->start(); }

  /// Destructor , stop the chrono
  ~Chrono() {
    if ( m_chrono ) { m_chrono->stop(); }
  }

private:
  Chrono( const Chrono& )            = delete;
  Chrono& operator=( const Chrono& ) = delete;

  /// The actual chronometer
  ChronoEntity* m_chrono = nullptr; // The actual chronometer
};
