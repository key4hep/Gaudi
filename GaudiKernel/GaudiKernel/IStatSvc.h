// $Id: IStatSvc.h,v 1.1 2007/08/06 08:39:40 marcocle Exp $
// ============================================================================
#ifndef GAUDIKERNEL_ISTATSVC_H
#define GAUDIKERNEL_ISTATSVC_H 1
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
class StatEntity ;
// ============================================================================
/** @class IStatSvc IStatSvc.h GaudiKernel/IStatSvc.h
 *  "Stat"-related part of interface IChronoStatSvc
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date December 1, 1999
 *  @date   2007-07-08
 */
class GAUDI_API IStatSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(IStatSvc,2,0);
  /// the actual identificator of Stat
  typedef    std::string     StatTag    ;
  /// the actual value type used for Stat
  typedef    double          StatFlag   ;
public:
  /** add statistical information to the entity , tagged by its name
   *   @param t stat tag(name)
   *   @param f flag (quantity to be accumulated)
   *   @return status of stat
   */
  virtual void  stat         ( const StatTag&    t , const StatFlag&   f ) = 0 ;
  /** prints (using message service)  info about statistical entity, tagged by its name
   *  @param t stat tag(name)
   *  @return status code
   */
  virtual void statPrint      ( const StatTag&    t ) = 0;
  /** extract the stat   entity for the given tag (name)
   *  @param t stat   tag(name)
   *  @return pointer to stat   entity
   */
  virtual const StatEntity*   stat   ( const StatTag&   t ) const = 0 ;
protected:
  // virtual and protected destructor
  virtual ~IStatSvc( ); ///< Destructor
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_ISTATSVC_H
// ============================================================================
