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
#ifndef GAUDIKERNEL_IEXCEPTIONSVC_H
#define GAUDIKERNEL_IEXCEPTIONSVC_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IInterface.h>
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
class GAUDI_API IExceptionSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IExceptionSvc, 3, 0 );
  /// Handle caught GaudiExceptions
  virtual StatusCode handle( const INamedInterface& o,
                             const GaudiException&  e ) const = 0; ///< Handle caught exceptions
  /// Handle caught std::exceptions
  virtual StatusCode handle( const INamedInterface& o,
                             const std::exception&  e ) const = 0; ///< Handle caught exceptions
  /// Handle caught (unknown)exceptions
  virtual StatusCode handle( const INamedInterface& o ) const = 0; ///< Handle caught exceptions
  /// Handle errors
  virtual StatusCode handleErr( const INamedInterface& o, const StatusCode& s ) const = 0; ///< Handle errors
};
// ============================================================================

// ============================================================================
#endif // GAUDIKERNEL_IEXCEPTIONSVC_H
