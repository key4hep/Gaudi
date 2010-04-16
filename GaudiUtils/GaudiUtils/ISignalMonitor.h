#ifndef GAUDIUTILS_ISIGNALMONITOR_H
#define GAUDIUTILS_ISIGNALMONITOR_H

#include "GaudiKernel/IInterface.h"

namespace Gaudi {

  /** Interface for the service that monitors the occurrences of system signals.
   *
   * The signal to be monitored have to be declared via the method monitorSignal().
   * The method gotSignal() can used to check if a signal has been received.
   *
   * Note that the service is passive, in the sense that it simply keeps track of
   * received signals without performing any action. So it is responsibility of the
   * users to add a check if the signal has been received or not.
   *
   *  @author  Marco Clemencic
   */
  class GAUDI_API ISignalMonitor: virtual public IInterface {
  public:
    /// InterfaceID
    DeclareInterfaceID(ISignalMonitor, 1, 0);

    /// Add a signal (number) to the list of signals to be monitored.
    /// It possible to choose if a previously installed signal handler should be
    /// called or not.
    ///
    /// @param signum: signal number
    /// @param propagate: if true (default) an already present signal handler is called,
    ///  otherwise the signal is stopped in the service.
    virtual void monitorSignal(int signum, bool propagate = true) = 0;

    /// Ignore future occurrences of the given signal number.
    ///
    /// @param signum: signal number
    virtual void ignoreSignal(int signum) = 0;

    /// Tell if the given signal has been received or not.
    ///
    /// @param signum: signal number
    virtual bool gotSignal(int signum) const = 0;

    /// Set the flag for the given signal, as if the signal was received.
    ///
    /// @param signum: signal number
    virtual void setSignal(int signum) = 0;

    /// Clear the flag for the given signal, so that a new occurrence can be identified.
    ///
    /// @param signum: signal number
    virtual void clearSignal(int signum) = 0;

  };

} // namespace Gaudi

#endif /* GAUDIUTILS_ISIGNALMONITOR_H */
