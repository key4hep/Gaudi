/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/**
 * Helper functions to set/get the application return code.
 *
 * @author Marco Clemencic
 */
#ifndef APPRETURNCODE_H_
#define APPRETURNCODE_H_

#include "GaudiKernel/IProperty.h"
#include <Gaudi/Property.h>

namespace Gaudi {
  /// ApplicationMgr return code definitions.
  namespace ReturnCode {

    constexpr int Success = 0x00;

    constexpr int GenericFailure = 0x01;

    /// @defgroup loop_stop Loop termination
    /// Error codes for abnormal loop termination.
    /// @{
    constexpr int FailInput          = 0x02; //< Error opening file
    constexpr int AlgorithmFailure   = 0x03; //<
    constexpr int ScheduledStop      = 0x04; //< Loop terminated because of user request
    constexpr int IncidentFailure    = 0x05; //< Fatal error in Incident handling
    constexpr int UnhandledException = 0x06; //<
    constexpr int CorruptedInput     = 0x10; //< Input file showed a corruption
    /// @}
    /// @{
    /// Error codes for operation failures.
    constexpr int FinalizationFailure = 0x0b;
    /// @}
    constexpr int SignalOffset = 0x80; //< Offset for signal-related return codes
  }                                    // namespace ReturnCode

  /**
   * Set the application return code.
   * By default the return code of the application is modified only if the
   * original value was 0 (i.e. no error). In this way we keep the first cause
   * of error.
   *
   * @param appmgr  IProperty interface of the ApplicationMgr
   * @param value   value to assign to the return code
   * @param force   if set to true, the return code is set even if it was already set
   *
   * @return SUCCESS if it was possible to set the return code or the return code was already set
   */
  inline StatusCode setAppReturnCode( SmartIF<IProperty>& appmgr, int value, bool force = false ) {
    if ( appmgr ) {
      Gaudi::Property<int> returnCode( "ReturnCode", 0 );
      if ( appmgr->getProperty( &returnCode ).isSuccess() ) {
        if ( returnCode.value() == 0 || force ) {
          returnCode.setValue( value );
          return appmgr->setProperty( returnCode );
        }
        // Consider is a success if we do already have an error code.
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }

  /**
   * Get the application (current) return code.
   *
   * @return the return code or 0 if it was not possible to get it
   */
  inline int getAppReturnCode( const SmartIF<IProperty>& appmgr ) {
    if ( appmgr ) {
      Gaudi::Property<int> returnCode( "ReturnCode", 0 );
      if ( appmgr->getProperty( &returnCode ).isSuccess() ) return returnCode.value();
    }
    return 0;
  }
} // namespace Gaudi
#endif /* APPRETURNCODE_H_ */
