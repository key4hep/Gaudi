// $Id: IChronoSvc.h,v 1.3 2008/05/13 12:36:54 marcocle Exp $
// ============================================================================
#ifndef GAUDIKERNEL_ICHRONOSVC_H
#define GAUDIKERNEL_ICHRONOSVC_H 1
// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IInterface.h"
// ============================================================================
// forward declaration
// ============================================================================
class ChronoEntity ;
// ============================================================================
/** @class IChronoSvc IChronoSvc.h GaudiKernel/IChronoSvc.h
 *  "Chrono"-related part of interface IChronoStatSvc
 *
 *  The IChronoSvc is the interface implemented by the ChronoStatService.
 *  This interface is used by any algorithm or services wanting to study
 *  its own performance and CPU consumption and some statistical computation
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *         (inspired by Mauro VILLA's codes used within ARTE framework )
 *
 *  @date December 1, 1999
 *  @date 2007-07-08
 */
class GAUDI_API IChronoSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(IChronoSvc,3,0);
  // ==========================================================================
  /// the actual type of identifier for Chrono objects
  typedef    std::string     ChronoTag  ;
  /// Type of the delta-time
  //typedef    longlong        ChronoTime ;
  typedef    double          ChronoTime ;
  // ==========================================================================
public:
  // ==========================================================================
  // status of Chrono objects
  enum ChronoStatus
    {
      UNKNOWN = 0 ,
      RUNNING     ,
      STOPPED
    };
  // types of Chrono objects
  enum ChronoType
    {
      USER = 0 ,
      KERNEL   ,
      ELAPSED
    } ;
  // ==========================================================================
public:
  // ==========================================================================
  /**    start   chrono, tagged by its name
   *     @param t chrono tag (name)
   *     @return chrono object
   */
  virtual       ChronoEntity* chronoStart   ( const ChronoTag& t ) = 0;
  // ==========================================================================
  /**    stop    chrono, tagged by its name
   *     @param t chrono tag (name)
   *     @return status code
   */
  virtual const ChronoEntity* chronoStop    ( const ChronoTag& t ) = 0;
  // ==========================================================================
  /**    return chrono delta time of last start/stop pair
   *     @param t chrono tag (name)
   *     @param f chtono type
   *     @return delta time
   */
  virtual ChronoTime  chronoDelta   ( const ChronoTag& t , ChronoType f ) = 0 ;
  // ==========================================================================
  /**    prints (using message service)  info about chrono, tagged by its name
   *     @param t chrono tag (name)
   *     @return status code
   */
  virtual void  chronoPrint   ( const ChronoTag&  t ) = 0;
  // ==========================================================================
  /**    return the status of named chrono
   *     @param t chrono tag (name)
   *     @return chrono status
   */
  virtual ChronoStatus  chronoStatus ( const ChronoTag&  t ) = 0;
  // ==========================================================================
  /** extract the chrono entity for the given tag (name)
   *  @param t chrono tag(name)
   *  @return pointer to chrono entity
   */
  virtual const ChronoEntity* chrono ( const ChronoTag& t ) const = 0 ;
  // ==========================================================================
protected:
  // ==========================================================================
  // protected and virtual destructor
  virtual ~IChronoSvc(); ///< protected and virtual destructor
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_ICHRONOSVC_H
// ============================================================================
