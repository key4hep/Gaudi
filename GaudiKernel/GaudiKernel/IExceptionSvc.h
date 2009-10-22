// $Id: IExceptionSvc.h,v 1.3 2007/05/24 14:39:11 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.3 $
// ============================================================================
#ifndef GAUDIKERNEL_IEXCEPTIONSVC_H
#define GAUDIKERNEL_IEXCEPTIONSVC_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// Forward declarations
// ============================================================================
class INamedInterface;
// ============================================================================
/** @class IExceptionSvc
 *  The abstract interface for exception handling service
 *  @see GaudiException
 *  @author (1) ATLAS collaboration
 *  @author (2) modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-08
 */
// ============================================================================
class GAUDI_API IExceptionSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IExceptionSvc,3,0);
  /// Handle caught GaudiExceptions
  virtual StatusCode handle
  ( const INamedInterface& o ,
    const GaudiException&  e ) const = 0 ; ///< Handle caught exceptions
  /// Handle caught std::exceptions
  virtual StatusCode handle
  ( const INamedInterface& o ,
    const std::exception & e ) const = 0 ; ///< Handle caught exceptions
  /// Handle caught (unknown)exceptions
  virtual StatusCode handle
  ( const INamedInterface& o ) const = 0 ; ///< Handle caught exceptions
  /// Handle errors
  virtual StatusCode handleErr
  ( const INamedInterface& o ,
    const StatusCode&      s ) const = 0 ; ///< Handle errors
protected :
  // virtual and protected destructor
  virtual ~IExceptionSvc() ; ///< virtual and protected destructor
} ;
// ============================================================================


// ============================================================================
#endif // GAUDIKERNEL_IEXCEPTIONSVC_H
// ============================================================================
// The END
// ============================================================================
