/*
 * SignalMonitorSvc.cpp
 *
 *  Created on: Apr 14, 2010
 *      Author: Marco Clemencic
 */
#include "GaudiKernel/Service.h"
#include "GaudiUtils/ISignalMonitor.h"

#include <csignal>

#include <iostream>

namespace Gaudi {
  namespace Utils {
    /**
     * Implementation of Gaudi::ISignalMonitor.
     *
     * If instantiated, intercepts the system signals and keep track of the
     * recorded ones.
     *
     * The signal to be monitored have to be declared via the method monitorSignal().
     * It can be interrogated to check if a signal has been received.
     *
     */
    class SignalMonitorSvc: public extends1<Service, Gaudi::ISignalMonitor> {
    public:
#ifdef _WIN32
      typedef void (__cdecl *handler_t)(int);
#else
      typedef struct sigaction handler_t;
#endif

      /// Declare a signal to be monitored.
      /// It installs a signal handler for the requested signal.
      void monitorSignal(int signum, bool propagate) {
        if (!m_monitored[signum]) {
          handler_t sa;
          handler_t oldact;
#ifdef _WIN32
          sa = SignalMonitorSvc::dispatcher;
          oldact = signal(signum, sa);
#else
          sa.sa_handler = SignalMonitorSvc::dispatcher;
          sigemptyset(&sa.sa_mask);
          sa.sa_flags = 0;
          sigaction(signum, &sa, &oldact);
#endif
          m_oldActions[signum] = oldact;
          m_monitored[signum] = (propagate) ? SignalMonitorSvc::propagate
                                            : SignalMonitorSvc::trap;
        }
      }

      /// Remove the specific signal handler for the requested signal, restoring
      /// the previous signal handler.
      void ignoreSignal(int signum) {
        if (m_monitored[signum]) {
#ifdef _WIN32
          (void) signal(signum, m_oldActions[signum]);
#else
          sigaction(signum, &m_oldActions[signum], 0);
#endif
          m_oldActions[signum] = m_defaultAction;
          m_monitored[signum] = ignored;
        }
      }

      /// Check if the given signal has been received.
      bool gotSignal(int signum) const {
        return m_caught[signum] != 0;
      }

      /// Set the flag for the given signal, as if the signal was received.
      void setSignal(int signum) {
        m_caught[signum] = 1;
      }

      /// Clear the flag for the given signal, so that a new occurrence can be identified.
      void clearSignal(int signum) {
        m_caught[signum] = 0;
      }

      /// Initialize internal variables of the service and set the instance pointer.
      SignalMonitorSvc(const std::string& name, ISvcLocator* svcLoc): base_class(name, svcLoc) {
#ifdef _WIN32
        m_defaultAction = SIG_DFL;
#else
        m_defaultAction.sa_handler = SIG_DFL;
        sigemptyset(&m_defaultAction.sa_mask);
        m_defaultAction.sa_flags = 0;
#endif
        for(int i = 0; i < NSIG; ++i){
          m_caught[i] = 0;
          m_monitored[i] = ignored;
          m_oldActions[i] = m_defaultAction;
        }

        setInstance(this);
      }

      /// Stop monitoring signals and clear the instance pointer.
      virtual ~SignalMonitorSvc() {
        for (int i = 0; i < NSIG; ++i) {
          ignoreSignal(i);
        }
        setInstance(0);
      }

    private:
      /// Possible monitoring modes
      enum MonitoringMode {
        ignored,  //< the signal is not monitored
        trap,     //< the signal is monitored and not propagated to previously registered handlers
        propagate //< the signal is monitored and propagated to previously registered handlers
      };
      /// Array of flags to keep track of monitored signals.
      MonitoringMode   m_monitored[NSIG];
      /// Array of flags for received signals.
      sig_atomic_t     m_caught[NSIG];
      /// Helper variable for default signal action.
      handler_t        m_defaultAction;
      /// List of replaced signal actions (for the recovery when disable the monitoring).
      handler_t        m_oldActions[NSIG];

      void i_handle(int signum) {
        m_caught[signum] = 1;
        if ( m_monitored[signum] == propagate &&
#ifdef _WIN32
            m_oldActions[signum] != SIG_DFL
#else
            m_oldActions[signum].sa_handler != SIG_DFL
#endif
        ) {
#ifdef _WIN32
          m_oldActions[signum](signum);
#else
          m_oldActions[signum].sa_handler(signum);
#endif
        }
      }

      /// Pointer to the current instance.
      static SignalMonitorSvc *s_instance;

      static inline void setInstance(SignalMonitorSvc *i) {
        s_instance = i;
      }

      /// Method to get the singleton instance.
      /// Bypass the serviceLocator for efficiency.
      static inline SignalMonitorSvc *instance() {
        return s_instance;
      }

      /// Signal handler function.
      static void dispatcher(int signum);
    };

    // Implementation of the signal handler function.
    void SignalMonitorSvc::dispatcher(int signum){
      if (instance()) instance()->i_handle(signum);
    }

  } // namespace Utils
} // namespace Gaudi

#include <cctype>
#include <sstream>

#include <map>

#include "GaudiKernel/HashMap.h"

#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/AppReturnCode.h"

namespace {
  // hack because windows doesn't provide sys_siglist
  const char *sig_desc(int signum) {
    if (signum >= NSIG || signum < 0)
      return 0;
#ifdef _WIN32
    switch (signum) {
    case SIGINT:   return "Interrupt";
    case SIGILL:   return "Illegal instruction";
    case SIGFPE:   return "Floating point exception";
    case SIGSEGV:  return "Segmentation fault";
    case SIGTERM:  return "Terminated";
    case SIGBREAK: return "Trace/breakpoint trap";
    case SIGABRT:  return "Aborted";
    default: return 0;
    }
#else
    return sys_siglist[signum];
#endif
  }

  /// Helper class to map signal names and numbers.
  class SigMap {
  public:
    /// Accessor to the singleton
    static const SigMap& instance() {
      static SigMap _instance;
      return _instance;
    }
    /// Return the signal name/id corresponding to a signal number (empty string if not known).
    inline const std::string &name(int signum) const {
      return m_num2id[signum];
    }
    /// Return the signal description corresponding to a signal number (empty string if not known).
    inline const std::string &desc(int signum) const {
      return m_num2desc[signum];
    }
    /// Return the signal number corresponding to a signal name or description (-1 if not known).
    inline int signum(const std::string &str) const {
      GaudiUtils::HashMap<std::string, int>::const_iterator it;
      it = m_name2num.find(str);
      if (it == m_name2num.end()) {
        return -1;
      }
      return it->second;
    }
  private:
    /// Constructor.
    /// Initializes internal maps.
    SigMap(){
#define addSignal(id) i_addSignal(id, #id);
      // List of signals from http://en.wikipedia.org/wiki/POSIX_signal
#ifdef SIGABRT
      addSignal(SIGABRT); // Process aborted
#endif
#ifdef SIGALRM
      addSignal(SIGALRM); // Signal raised by alarm
#endif
#ifdef SIGBUS
      addSignal(SIGBUS); // Bus error: "access to undefined portion of memory object"
#endif
#ifdef SIGCHLD
      addSignal(SIGCHLD); // Child process terminated, stopped (or continued*)
#endif
#ifdef SIGCONT
      addSignal(SIGCONT); // Continue if stopped
#endif
#ifdef SIGFPE
      addSignal(SIGFPE); // Floating point exception: "erroneous arithmetic operation"
#endif
#ifdef SIGHUP
      addSignal(SIGHUP); // Hangup
#endif
#ifdef SIGILL
      addSignal(SIGILL); // Illegal instruction
#endif
#ifdef SIGINT
      addSignal(SIGINT); // Interrupt
#endif
#ifdef SIGKILL
      addSignal(SIGKILL); // Kill (terminate immediately)
#endif
#ifdef SIGPIPE
      addSignal(SIGPIPE); // Write to pipe with no one reading
#endif
#ifdef SIGQUIT
      addSignal(SIGQUIT); // Quit and dump core
#endif
#ifdef SIGSEGV
      addSignal(SIGSEGV); // Segmentation violation
#endif
#ifdef SIGSTOP
      addSignal(SIGSTOP); // Stop executing temporarily
#endif
#ifdef SIGTERM
      addSignal(SIGTERM); // Termination (request to terminate)
#endif
#ifdef SIGTSTP
      addSignal(SIGTSTP); // Terminal stop signal
#endif
#ifdef SIGTTIN
      addSignal(SIGTTIN); // Background process attempting to read from tty ("in")
#endif
#ifdef SIGTTOU
      addSignal(SIGTTOU); // Background process attempting to write to tty ("out")
#endif
#ifdef SIGUSR1
      addSignal(SIGUSR1); // User-defined 1
#endif
#ifdef SIGUSR2
      addSignal(SIGUSR2); // User-defined 2
#endif
#ifdef SIGPOLL
      addSignal(SIGPOLL); // Pollable event
#endif
#ifdef SIGPROF
      addSignal(SIGPROF); // Profiling timer expired
#endif
#ifdef SIGSYS
      addSignal(SIGSYS); // Bad syscall
#endif
#ifdef SIGTRAP
      addSignal(SIGTRAP); // Trace/breakpoint trap
#endif
#ifdef SIGURG
      addSignal(SIGURG); // Urgent data available on socket
#endif
#ifdef SIGVTALRM
      addSignal(SIGVTALRM); // Signal raised by timer counting virtual time: "virtual timer expired"
#endif
#ifdef SIGXCPU
      addSignal(SIGXCPU); // CPU time limit exceeded
#endif
#ifdef SIGXFSZ
      addSignal(SIGXFSZ); // File size limit exceeded
#endif
#undef addSignal
    }
    /// Internal helper function used to fill the maps
    inline void i_addSignal(int signum, const char *signame) {
      m_num2id[signum] = signame;
      m_name2num[signame] = signum;
      const char* desc = sig_desc(signum);
      if (desc) {
        m_num2desc[signum] = desc;
        m_name2num[desc] = signum;
      }
    }
    GaudiUtils::HashMap<std::string, int> m_name2num; //< Map signal string id or description to number
    GaudiUtils::HashMap<int, std::string> m_num2id; //< Map signal number to string id
    GaudiUtils::HashMap<int, std::string> m_num2desc; //< Map signal number to description
  };
}

namespace Gaudi {
  namespace Utils {
    /**
     * Service that stop the processing if a signal is received.
     *
     * The signals to be intercepted have to be declared in the property Signals
     * as a list of strings (signal names or numbers). If '+' is appended to the
     * signal name, then the signal is propagated to the signal handlers already
     * registered when this service is initialized.
     *
     */
    class StopSignalHandler: public extends1<Service, IIncidentListener> {
    public:
      StopSignalHandler(const std::string& name, ISvcLocator* svcLoc): base_class(name, svcLoc) {
        m_usedSignals.reserve(2);
        m_usedSignals.push_back("SIGINT");
        m_usedSignals.push_back("SIGXCPU");
        m_stopRequested = false;
        declareProperty("Signals", m_usedSignals,
            "List of signal names or numbers to use to schedule a stop. "
            "If the signal is followed by a '+' the signal is propagated the previously "
            "registered handler (if any).");
      }
      StatusCode initialize() {
        StatusCode sc = Service::initialize();
        if (sc.isFailure()) {
          return sc;
        }
        std::string serviceName("Gaudi::Utils::SignalMonitorSvc");
        m_signalMonitor = serviceLocator()->service(serviceName);
        if ( ! m_signalMonitor ) {
          error() << "Cannot retrieve " << serviceName << endmsg;
          return StatusCode::FAILURE;
        }
        serviceName = "IncidentSvc";
        m_incidentSvc = serviceLocator()->service(serviceName);
        if ( ! m_incidentSvc ) {
          error() << "Cannot retrieve " << serviceName << endmsg;
          return StatusCode::FAILURE;
        }
        // Get the IMainAppStatus interface of the ApplicationMgr
        m_appProperty = serviceLocator();
        if ( ! m_appProperty ) {
          warning() << "Cannot retrieve IProperty interface of ApplicationMgr, "
                       "the return code will not be changed" << endmsg;
        }
        // Decode the signal names
        std::pair<int, bool> sigid;
        for (std::vector<std::string>::const_iterator signame = m_usedSignals.begin();
            signame != m_usedSignals.end(); ++signame) {
          sigid = i_decodeSignal(*signame);
          if (sigid.first >= 0) {
            m_signals[sigid.first] = sigid.second;
          }
        }
        debug() << "Stopping on the signals:" << endmsg;
        const SigMap& sigmap(SigMap::instance());
        for (std::map<int, bool>::const_iterator s = m_signals.begin();
            s != m_signals.end(); ++s) {
          debug() << "\t" << sigmap.name(s->first) << ": "
                  << sigmap.desc(s->first) << " (" << s->first << ")";
          if (s->second) debug() << " propagated";
          debug() << endmsg;
          // tell the signal monitor that we are interested in these signals
          m_signalMonitor->monitorSignal(s->first, s->second);
        }
        m_stopRequested = false;
        debug() << "Register to the IncidentSvc" << endmsg;
        m_incidentSvc->addListener(this, IncidentType::BeginEvent);
        return StatusCode::SUCCESS;
      }
      StatusCode finalize() {
        m_incidentSvc->removeListener(this, IncidentType::BeginEvent);
        m_incidentSvc.reset();
        // disable the monitoring of the signals
        for (std::map<int, bool>::const_iterator s = m_signals.begin();
            s != m_signals.end(); ++s) {
          // tell the signal monitor that we are interested in these signals
          m_signalMonitor->ignoreSignal(s->first);
        }
        m_signalMonitor.reset();
        return Service::finalize();
      }

      virtual void handle(const Incident&) {
        if (!m_stopRequested) {
          const SigMap& sigmap(SigMap::instance());
          for (std::map<int, bool>::const_iterator s = m_signals.begin();
              s != m_signals.end(); ++s) {
            if (m_signalMonitor->gotSignal(s->first)) {
              warning() << "Received signal '" << sigmap.name(s->first)
                        << "' (" << s->first;
              const std::string &desc = sigmap.desc(s->first);
              if ( ! desc.empty() ) {
                warning() << ", " << desc;
              }
              warning() << ")" << endmsg;
              m_stopRequested = true;
              // Report the termination by signal at the end of the application
              using Gaudi::ReturnCode::SignalOffset;
              if (Gaudi::setAppReturnCode(m_appProperty, SignalOffset + s->first).isFailure()) {
                error() << "Could not set return code of the application ("
                    << SignalOffset + s->first << ")"
                    << endmsg;
              }
            }
          }
          if (m_stopRequested) {
            SmartIF<IEventProcessor> ep(serviceLocator());
            if (ep) {
              warning() << "Scheduling a stop" << endmsg;
              ep->stopRun().ignore();
            }
            else {
              warning() << "Cannot stop the processing because the IEventProcessor interface cannot be retrieved." << endmsg;
            }
          }
        }
      }
      private:
      /// List of signal names or numbers (encoded as strings) to use to schedule a stop.
      std::vector<std::string> m_usedSignals;
      /// Map of monitored signal numbers to the flag telling if they have to be propagated or not.
      std::map<int, bool> m_signals;
      /// Flag to remember if the stop has been requested because of a signal.
      bool m_stopRequested;
      /// Pointer to the signal monitor service.
      SmartIF<Gaudi::ISignalMonitor> m_signalMonitor;
      /// Pointer to the incident service.
      SmartIF<IIncidentSvc> m_incidentSvc;
      /// Pointer to the interface to set the return code of the application.
      SmartIF<IProperty> m_appProperty;
      /// Function to translate the signal name to the signal number.
      std::pair<int, bool> i_decodeSignal(const std::string &sig) {
        debug() << "Decoding signal declaration '" << sig << "'" << endmsg;
        if ( sig.empty() || sig == "+" ) {
          debug() << "Empty signal, ignored" << endmsg;
          return std::make_pair<int, bool>(-1, false); // silently ignore empty strings
        }
        const SigMap& sigmap(SigMap::instance());
        std::string signal = sig;
        bool propagate = false;
        // Check if the signal must be propagated
        if (signal[signal.size() - 1] == '+') {
          debug() << "Must be propagated to previously registered signal handlers" << endmsg;
          propagate = true;
          signal.erase(signal.size() - 1, 1); // remove the '+' at the end of the string
        }
        int signum = -1;
        // check if the signal is a number
        if (std::isdigit(signal[0])){
          std::istringstream ss(signal);
          ss >> signum;
        } else {
          // try to find the signal name in the list of known signals
          signum = sigmap.signum(signal);
        }
        if (signum < 0) {
          warning() << "Cannot understand signal identifier '" << sig << "', ignored" << endmsg;
        } else {
          verbose() << "Matched signal '" << sigmap.name(signum)
                    << "' (" << signum;
          const std::string &desc = sigmap.desc(signum);
          if ( ! desc.empty() ) {
            verbose() << ", " << desc;
          }
          verbose() << ")" << endmsg;
        }
        return std::make_pair(signum, propagate);
      }
    };

  } // namespace Utils
} // namespace Gaudi

// Initialization of static data member
Gaudi::Utils::SignalMonitorSvc* Gaudi::Utils::SignalMonitorSvc::s_instance = 0;

// ========================================================================
// Instantiation of a static factory class used by clients to create instances of this service
typedef Gaudi::Utils::SignalMonitorSvc g_u_sms;
DECLARE_COMPONENT(g_u_sms)

// Instantiation of a static factory class used by clients to create instances of this service
typedef Gaudi::Utils::StopSignalHandler g_u_ssh;
DECLARE_COMPONENT(g_u_ssh)
