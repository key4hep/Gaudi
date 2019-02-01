
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/Sequencer.h"

#include "boost/assign/list_of.hpp"

#ifdef TCMALLOC_OLD_GOOGLE_HEADERS
#  include "google/heap-checker.h"
#  include "google/heap-profiler.h"
#  include "google/profiler.h"
#else
#  include "gperftools/heap-checker.h"
#  include "gperftools/heap-profiler.h"
#  include "gperftools/profiler.h"
#endif

namespace Google {

  /** @class AuditorBase GoogleAuditor.cpp
   *
   *  Base for Google Auditors
   *
   *  @author Chris Jones
   *  @date   18/04/2011
   */
  class AuditorBase : public extends<Auditor, IIncidentListener> {

  public:
    /// Constructor
    using extends::extends;

    /// Initialize the auditor base
    StatusCode initialize() override {
      info() << "Initialised" << endmsg;

      // add a listener for begin event
      auto inSvc = serviceLocator()->service<IIncidentSvc>( "IncidentSvc" );
      if ( !inSvc ) return StatusCode::FAILURE;
      inSvc->addListener( this, IncidentType::BeginEvent );

      // sort various lists for speed when searching
      std::sort( m_when.begin(), m_when.end() );
      std::sort( m_veto.begin(), m_veto.end() );
      std::sort( m_list.begin(), m_list.end() );

      return StatusCode::SUCCESS;
    }

    /// Finalize the auditor base
    StatusCode finalize() override {
      if ( alreadyRunning() ) stopAudit();
      return StatusCode::SUCCESS;
    }

  private:
    /// Start a full event audit
    inline void startAudit() {
      info() << " -> Starting full audit from event " << m_nEvts << " to " << m_nEvts + m_nSampleEvents << endmsg;
      m_inFullAudit      = true;
      m_sampleEventCount = 1;
      std::ostringstream t;
      t << "FULL-Events" << m_nEvts << "To" << m_nEvts + m_nSampleEvents;
      google_before( t.str() );
    }

    /// stop a full event audit
    inline void stopAudit() {
      info() << " -> Stopping full audit" << endmsg;
      std::ostringstream t;
      t << "FULL-Events" << m_nEvts << "To" << m_nEvts + m_nSampleEvents;
      google_after( t.str() );
      m_inFullAudit      = false;
      m_sampleEventCount = 0;
    }

    /** Check if the component in question is a GaudiSequencer or
     *  a Sequencer */
    inline bool isSequencer( INamedInterface* i ) const {
      return ( dynamic_cast<GaudiSequencer*>( i ) != NULL || dynamic_cast<Sequencer*>( i ) != NULL );
    }

    /// Check if auditing is enabled for the current processing phase
    inline bool isPhaseEnabled( CustomEventTypeRef type ) const {
      return ( std::find( m_when.begin(), m_when.end(), type ) != m_when.end() );
    }

    /// Check if auditing is enabled for the given component
    inline bool isComponentEnabled( const std::string& name ) const {
      return ( std::find( m_veto.begin(), m_veto.end(), name ) == m_veto.end() &&
               ( m_list.empty() || std::find( m_list.begin(), m_list.end(), name ) != m_list.end() ) );
    }

    // Construct the dump name based on processing phase and component name
    std::string getDumpName( CustomEventTypeRef type, const std::string& name ) const {
      std::ostringstream t;
      t << name << "-" << type;
      if ( type == "Execute" ) t << "-Event" << m_nEvts;
      return t.str();
    }

  public:
    /** Implement the handle method for the Incident service.
     *  This is used to inform the tool of software incidents.
     *
     *  @param incident The incident identifier
     */
    void handle( const Incident& incident ) override {
      if ( IncidentType::BeginEvent == incident.type() ) {
        ++m_nEvts;
        m_audit = ( m_nEvts > m_eventsToSkip && ( m_freq < 0 || m_nEvts == 1 || m_nEvts % m_freq == 0 ) );
        if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) debug() << "Event " << m_nEvts << " Audit=" << m_audit << endmsg;
        if ( m_fullEventAudit ) {
          if ( m_inFullAudit ) {
            if ( m_sampleEventCount >= m_nSampleEvents && alreadyRunning() ) {
              stopAudit();
            } else {
              ++m_sampleEventCount;
            }
          }
          if ( m_audit && !m_inFullAudit && !alreadyRunning() ) { startAudit(); }
        }
      }
    }

  public:
    void before( StandardEventType type, INamedInterface* i ) override {
      if ( !m_skipSequencers || !isSequencer( i ) ) { before( type, i->name() ); }
    }

    void before( CustomEventTypeRef type, INamedInterface* i ) override {
      if ( !m_skipSequencers || !isSequencer( i ) ) { before( type, i->name() ); }
    }

    void before( StandardEventType type, const std::string& s ) override {
      std::ostringstream t;
      t << type;
      before( t.str(), s );
    }

    void before( CustomEventTypeRef type, const std::string& s ) override {
      if ( !m_fullEventAudit && m_audit && isPhaseEnabled( type ) && isComponentEnabled( s ) ) {
        if ( !alreadyRunning() ) {
          info() << "Starting Auditor for " << s << ":" << type << endmsg;
          m_startedBy = s;
          google_before( getDumpName( type, s ) );
        } else {
          warning() << "Auditor already running. Cannot be started for " << s << endmsg;
        }
      }
    }

    void after( StandardEventType type, INamedInterface* i, const StatusCode& sc ) override {
      if ( !m_skipSequencers || !isSequencer( i ) ) {
        std::ostringstream t;
        t << type;
        after( t.str(), i, sc );
      }
    }

    void after( CustomEventTypeRef type, INamedInterface* i, const StatusCode& sc ) override {
      if ( !m_skipSequencers || !isSequencer( i ) ) { after( type, i->name(), sc ); }
    }

    void after( StandardEventType type, const std::string& s, const StatusCode& sc ) override {
      std::ostringstream t;
      t << type;
      after( t.str(), s, sc );
    }

    void after( CustomEventTypeRef type, const std::string& s, const StatusCode& ) override {
      if ( !m_fullEventAudit && m_audit && isPhaseEnabled( type ) && isComponentEnabled( s ) ) {
        if ( s == m_startedBy ) { google_after( getDumpName( type, s ) ); }
      }
    }

    // Obsolete methods
    void beforeInitialize( INamedInterface* i ) override { return before( IAuditor::Initialize, i ); }
    void beforeReinitialize( INamedInterface* i ) override { return before( IAuditor::ReInitialize, i ); }
    void beforeExecute( INamedInterface* i ) override { return before( IAuditor::Execute, i ); }
    void beforeBeginRun( INamedInterface* i ) override { return before( IAuditor::BeginRun, i ); }
    void beforeEndRun( INamedInterface* i ) override { return before( IAuditor::EndRun, i ); }
    void beforeFinalize( INamedInterface* i ) override { return before( IAuditor::Finalize, i ); }

    void afterInitialize( INamedInterface* i ) override {
      return after( IAuditor::Initialize, i, StatusCode::SUCCESS );
    }
    void afterReinitialize( INamedInterface* i ) override {
      return after( IAuditor::ReInitialize, i, StatusCode::SUCCESS );
    }
    void afterExecute( INamedInterface* i, const StatusCode& s ) override { return after( IAuditor::Execute, i, s ); }
    void afterBeginRun( INamedInterface* i ) override { return after( IAuditor::BeginRun, i, StatusCode::SUCCESS ); }
    void afterEndRun( INamedInterface* i ) override { return after( IAuditor::EndRun, i, StatusCode::SUCCESS ); }
    void afterFinalize( INamedInterface* i ) override { return after( IAuditor::Finalize, i, StatusCode::SUCCESS ); }

  protected:
    /// Start the google tool
    virtual void google_before( const std::string& s ) = 0;

    /// stop the google tool
    virtual void google_after( const std::string& s ) = 0;

    /// check if we are already running the tool
    virtual bool alreadyRunning() = 0;

  private:
    Gaudi::Property<std::vector<std::string>> m_when{
        this,
        "ActivateAt",
        {"Initialize", "ReInitialize", "Execute", "BeginRun", "EndRun", "Finalize"},
        "List of phases to activate the Auditoring during"};
    Gaudi::Property<std::vector<std::string>> m_veto{
        this, "DisableFor", {}, "List of component names to disable the auditing for"};
    Gaudi::Property<std::vector<std::string>> m_list{
        this, "EnableFor", {}, "Any component in this list will be audited. If empty,  all will be done."};
    Gaudi::Property<int>  m_freq{this, "ProfileFreq", -1, "The frequence to audit events. -1 means all events"};
    Gaudi::Property<bool> m_fullEventAudit{
        this, "DoFullEventProfile", false,
        "If true, instead of individually auditing components,  the full event (or events) will be audited in one go"};
    Gaudi::Property<unsigned long long> m_nSampleEvents{
        this, "FullEventNSampleEvents", 1, "The number of events to include in a full event audit,  if enabled"};
    Gaudi::Property<unsigned long long> m_eventsToSkip{this, "SkipEvents", 0,
                                                       "Number of events to skip before activating the auditing"};
    Gaudi::Property<bool>               m_skipSequencers{this, "SkipSequencers", true,
                                           "If true,  auditing will be skipped for Sequencer objects."};

    bool               m_audit = true; ///< Internal flag to say if auditing is enabled or not for the current event
    unsigned long long m_nEvts = 0;    ///< Number of events processed.
    unsigned long long m_sampleEventCount =
        0;                             ///< Internal count of the number of events currently processed during an audit
    bool        m_inFullAudit = false; ///< Internal flag to indicate if we are current in a full event audit
    std::string m_startedBy;           ///< Name of the component we are currently auditing
  };

  /** @class HeapProfiler GoogleAuditor.cpp
   *
   *  Auditor based on the Google Heap Profiler
   *
   *  See
   *
   *  http://google-perftools.googlecode.com/svn/trunk/doc/heapprofile.html
   *
   *  For more details.
   *
   *  @author Chris Jones
   *  @date   18/04/2011
   */
  class HeapProfiler : public AuditorBase {

  public:
    /// Constructor
    using AuditorBase::AuditorBase;

  protected:
    void google_before( const std::string& s ) override { HeapProfilerStart( s.c_str() ); }

    void google_after( const std::string& s ) override {
      if ( m_dumpProfileHeaps ) { HeapProfilerDump( s.c_str() ); }
      if ( m_printProfilesToLog ) {
        const char* profile = GetHeapProfile();
        info() << profile << endmsg;
        delete profile;
      }
      HeapProfilerStop();
    }

    bool alreadyRunning() override { return IsHeapProfilerRunning(); }

  private:
    Gaudi::Property<bool> m_dumpProfileHeaps{this, "DumpHeapProfiles", true, ""};
    Gaudi::Property<bool> m_printProfilesToLog{this, "PrintProfilesToLog", false, ""};
  };

  /** @class HeapChecker GoogleAuditor.cpp
   *
   *  Auditor using the Google Heap Checker
   *
   *  See
   *
   *  http://google-perftools.googlecode.com/svn/trunk/doc/heap_checker.html
   *
   *  For more details on usage.
   *
   *  @author Chris Jones
   *  @date   18/04/2011
   */
  class HeapChecker : public AuditorBase {

  public:
    /// Constructor
    using AuditorBase::AuditorBase;

    StatusCode initialize() override {
      const StatusCode sc = AuditorBase::initialize();
      if ( sc.isFailure() ) return sc;

      const char* HEAPCHECK = getenv( "HEAPCHECK" );
      if ( !HEAPCHECK ) {
        fatal() << "Environment variable HEAPCHECK must be set to 'local'" << endmsg;
        return StatusCode::FAILURE;
      }
      if ( std::string( HEAPCHECK ) != "local" ) {
        warning() << "Environment variable HEAPCHECK is set to " << HEAPCHECK
                  << " Partial Program Heap Checking is disabled" << endmsg;
        m_enabled = false;
      }

      return sc;
    }

  protected:
    void google_before( const std::string& s ) override {
      if ( m_enabled && !m_checker ) { m_checker.reset( new HeapLeakChecker( s.c_str() ) ); }
    }

    void google_after( const std::string& s ) override {
      if ( m_enabled && m_checker ) {
        if ( !m_checker->NoLeaks() ) { warning() << "Leak detected for " << s << endmsg; }
        m_checker.reset();
      }
    }

    bool alreadyRunning() override { return m_enabled && m_checker; }

  private:
    bool                             m_enabled = true;
    std::unique_ptr<HeapLeakChecker> m_checker;
  };

  /** @class CPUProfiler GoogleAuditor.cpp
   *
   *  Auditor using the Google CPU Profiler
   *
   *  See
   *
   *  http://google-perftools.googlecode.com/svn/trunk/doc/cpuprofile.html
   *
   *  For more details on usage.
   *
   *  @author Chris Jones
   *  @date   18/04/2011
   */
  class CPUProfiler : public AuditorBase {

  public:
    using AuditorBase::AuditorBase;

  protected:
    void google_before( const std::string& s ) override {
      if ( !m_running ) {
        m_running = true;
        ProfilerStart( ( s + ".prof" ).c_str() );
      }
    }

    void google_after( const std::string& ) override {
      if ( m_running ) {
        ProfilerStop();
        m_running = false;
      }
    }

    bool alreadyRunning() override { return m_running; }

  private:
    bool m_running = false;
  };

  DECLARE_COMPONENT( HeapProfiler )
  DECLARE_COMPONENT( HeapChecker )
  DECLARE_COMPONENT( CPUProfiler )
} // namespace Google
