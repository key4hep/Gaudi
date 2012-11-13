/**
 * Helper functions to set/get the application return code.
 *
 * @author Marco Clemencic
 */
#ifndef APPRETURNCODE_H_
#define APPRETURNCODE_H_

#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"

namespace Gaudi {
  /// ApplicationMgr return code definitions.
  namespace ReturnCode {

    const int Success                  = 0x00;

    const int GenericFailure           = 0x01;

    /// @defgroup loop_stop Loop termination
    /// Error codes for abnormal loop termination.
    /// @{
    const int FailInput                = 0x02; //< Error opening file
    const int AlgorithmFailure         = 0x03; //<
    const int ScheduledStop            = 0x04; //< Loop terminated because of user request
    const int IncidentFailure          = 0x05; //< Fatal error in Incident handling
    const int UnhandledException       = 0x06; //<
    const int CorruptedInput           = 0x10; //< Input file showed a corruption
    /// @}
    /// @{
    /// Error codes for operation failures.
    const int FinalizationFailure      = 0x0b;
    /// @}
    const int SignalOffset             = 0x80; //< Offset for signal-related return codes

  }

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
  inline StatusCode setAppReturnCode(SmartIF<IProperty> &appmgr, int value, bool force = false) {
    if (appmgr) {
      IntegerProperty returnCode("ReturnCode", 0);
      if (appmgr->getProperty(&returnCode).isSuccess()) {
        if (returnCode.value() == 0 || force) {
          returnCode.setValue(value);
          return appmgr->setProperty(returnCode);
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
  inline int getAppReturnCode(const SmartIF<IProperty> &appmgr) {
    if (appmgr) {
      IntegerProperty returnCode("ReturnCode", 0);
      if (appmgr->getProperty(&returnCode).isSuccess())
        return returnCode.value();
    }
    return 0;
  }
}
#endif /* APPRETURNCODE_H_ */
