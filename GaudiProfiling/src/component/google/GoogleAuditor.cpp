
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <algorithm>

#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"

#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/Sequencer.h"

#include "boost/assign/list_of.hpp"

#include "google/heap-profiler.h"
#include "google/heap-checker.h"
#include "google/profiler.h"

namespace Google
{

  /** @class AuditorBase GoogleAuditor.cpp
   *
   *  Base for Google Auditors
   *
   *  @author Chris Jones
   *  @date   18/04/2011
   */
  class AuditorBase : public extends1<Auditor, IIncidentListener>
  {

  public:

    /// Constructor
    AuditorBase( const std::string& name, ISvcLocator* pSvcLocator);

    /// Destructor
    virtual ~AuditorBase() {  }

    /// Initialize the auditor base
    StatusCode initialize()
    {
      m_log << MSG::INFO << "Initialised" << endmsg;

      SmartIF<IIncidentSvc> inSvc(serviceLocator()->service("IncidentSvc"));
      if ( ! inSvc.isValid() ) return StatusCode::FAILURE;

      inSvc->addListener( this, IncidentType::BeginEvent );

      return StatusCode::SUCCESS;
    }

    /// Finalize the auditor base
    StatusCode finalize()
    {
      if ( alreadyRunning() ) stopAudit();
      return StatusCode::SUCCESS;
    }

  private:

    /// Start a full event audit
    void startAudit()
    {
      m_log << MSG::INFO << " -> Starting full audit from event " << m_nEvts << " to "
            << m_nEvts+m_nSampleEvents << endmsg;
      m_inFullAudit = true;
      m_sampleEventCount = 1;
      std::ostringstream t;
      t << "FULL-Events" << m_nEvts << "To" << m_nEvts+m_nSampleEvents ;
      google_before(t.str());
    }

    /// stop a full event audit
    void stopAudit()
    {
      m_log << MSG::INFO << " -> Stopping full audit" << endmsg;
      std::ostringstream t;
      t << "FULL-Events" << m_nEvts << "To" << m_nEvts+m_nSampleEvents ;
      google_after(t.str());
      m_inFullAudit = false;
      m_sampleEventCount = 0;
    }

    /** Check if the component in question is a GaudiSequencer or
     *  a Sequencer */
    bool isSequencer( INamedInterface* i ) const
    {
      return ( dynamic_cast<GaudiSequencer*>(i) != NULL ||
               dynamic_cast<Sequencer*>(i)      != NULL );
    }

  public:

    /** Implement the handle method for the Incident service.
     *  This is used to inform the tool of software incidents.
     *
     *  @param incident The incident identifier
     */
    void handle( const Incident& incident )
    {
      if ( IncidentType::BeginEvent == incident.type() )
      {
        ++m_nEvts;
        m_audit = ( m_nEvts > m_eventsToSkip &&
                    ( m_freq < 0            ||
                      m_nEvts == 1          ||
                      m_nEvts % m_freq == 0  ) );
        m_log << MSG::DEBUG << "Event " << m_nEvts
              << " Audit=" << m_audit << endmsg;
        if ( m_fullEventAudit )
        {
          if ( m_inFullAudit )
          {
            if ( m_sampleEventCount >= m_nSampleEvents &&
                 alreadyRunning() )
            {
              stopAudit();
            }
            else
            {
              ++m_sampleEventCount;
            }
          }
          if ( m_audit && !m_inFullAudit && !alreadyRunning() )
          {
            startAudit();
          }
        }
      }
    }

  public:

    void before(StandardEventType type, INamedInterface* i)
    {
      if ( !m_skipSequencers || !isSequencer(i) )
      {
        before(type,i->name());
      }
    }

    void before(CustomEventTypeRef type, INamedInterface* i)
    {
      if ( !m_skipSequencers || !isSequencer(i) )
      {
        before(type,i->name());
      }
    }

    void before(StandardEventType type, const std::string& s)
    {
      std::ostringstream t;
      t << type;
      before(t.str(),s);
    }

    void before(CustomEventTypeRef, const std::string& s)
    {
      if ( !m_fullEventAudit && m_audit &&
           std::find(m_veto.begin(),m_veto.end(),s) == m_veto.end() &&
           ( m_list.empty() || std::find(m_list.begin(),m_list.end(),s) != m_list.end() ) )
      {
        if ( !alreadyRunning() )
        {
          m_log << MSG::INFO << "Starting Auditor for " << s << endmsg;
          m_startedBy = s;
          std::ostringstream t;
          t << s << "-Event" << m_nEvts;
          google_before(t.str());
        }
        else
        {
          m_log << MSG::WARNING
                << "Auditor already running. Cannot be started for " << s
                << endmsg;
        }
      }
    }

    void after(StandardEventType type, INamedInterface* i, const StatusCode& sc)
    {
      if ( !m_skipSequencers || !isSequencer(i) )
      {
        std::ostringstream t;
        t << type;
        after(t.str(),i,sc);
      }
    }

    void after(CustomEventTypeRef type, INamedInterface* i, const StatusCode& sc)
    {
      if ( !m_skipSequencers || !isSequencer(i) )
      {
        after(type,i->name(),sc);
      }
    }

    void after(StandardEventType type, const std::string& s, const StatusCode& sc)
    {
      std::ostringstream t;
      t << type;
      after(t.str(),s,sc);
    }

    void after(CustomEventTypeRef, const std::string& s, const StatusCode&)
    {
      if ( !m_fullEventAudit && m_audit &&
           std::find(m_veto.begin(),m_veto.end(),s) == m_veto.end() &&
           ( m_list.empty() || std::find(m_list.begin(),m_list.end(),s) != m_list.end() ) )
      {
        if ( s == m_startedBy )
        {
          std::ostringstream t;
          t << s << "-Event" << m_nEvts;
          google_after(t.str());
        }
      }
    }

    // Obsolete methods
    void beforeInitialize  (INamedInterface *i) { return before(IAuditor::Initialize,i);   }
    void beforeReinitialize(INamedInterface *i) { return before(IAuditor::ReInitialize,i); }
    void beforeExecute     (INamedInterface *i) { return before(IAuditor::Execute,i);      }
    void beforeBeginRun    (INamedInterface *i) { return before(IAuditor::BeginRun,i);     }
    void beforeEndRun      (INamedInterface *i) { return before(IAuditor::EndRun,i);       }
    void beforeFinalize    (INamedInterface *i) { return before(IAuditor::Finalize,i);     }

    void afterInitialize   (INamedInterface *i) { return after(IAuditor::Initialize,i,StatusCode::SUCCESS); }
    void afterReinitialize (INamedInterface *i) { return after(IAuditor::ReInitialize,i,StatusCode::SUCCESS); }
    void afterExecute      (INamedInterface *i, const StatusCode& s) { return after(IAuditor::Execute,i,s); }
    void afterBeginRun     (INamedInterface *i) { return after(IAuditor::BeginRun,i,StatusCode::SUCCESS); }
    void afterEndRun       (INamedInterface *i) { return after(IAuditor::EndRun,i,StatusCode::SUCCESS); }
    void afterFinalize     (INamedInterface *i) { return after(IAuditor::Finalize,i,StatusCode::SUCCESS); }

  protected:

    /// Start the google tool
    virtual void google_before(const std::string& s) = 0;

    /// stop the google tool
    virtual void google_after(const std::string& s) = 0;

    /// check if we are already running the tool
    virtual bool alreadyRunning() = 0;

  protected:

    mutable MsgStream         m_log;   ///< Messaging object

  private:

    std::vector<std::string>  m_when;  ///< When to audit the algorithms
    std::vector<std::string>  m_veto;  ///< Veto list. Any component in this list will not be audited
    std::vector<std::string>  m_list;  ///< Any component in this list will be audited. If empty, all will be done.

    unsigned long long m_eventsToSkip; ///< Number of events to skip before auditing

    bool m_skipSequencers; ///< Boolean indicating if sequencers should be skipped or not

    int m_freq;   ///< The frequency to audit events. -1 means all events.

    bool m_audit; ///< Internal flag to say if auditing is enabled or not for the current event

    unsigned long long m_nEvts; ///< Number of events processed.

    bool m_fullEventAudit; ///< Flag to indicate if full event auditing is enabled or not.

    unsigned long long m_nSampleEvents; ///< Number of events to include in a full event audit

    unsigned long long m_sampleEventCount; ///< Internal count of the number of events currently processed during an audit

    bool m_inFullAudit; ///< Internal flag to indicate if we are current in a full event audit

    std::string m_startedBy; ///< Name of the component we are currently auditing

  };

  AuditorBase::AuditorBase( const std::string& name,
                            ISvcLocator* pSvcLocator )
    : base_class ( name , pSvcLocator )
    , m_log      ( msgSvc() , name )
    , m_audit    ( false )
    , m_nEvts    ( 0 )
    , m_sampleEventCount( 0 )
    , m_inFullAudit ( false )
  {
    {
      // Note: 'tmp' is needed to avoid an issue with list_of and C++11.
      const std::vector<std::string> tmp =
        boost::assign::list_of
          ("Initialize")
          ("ReInitialize")
          ("Execute")
          ("BeginRun")
          ("EndRun")
          ("Finalize");
      m_when = tmp;
    }

    declareProperty("ActivateAt", m_when,
                    "List of phases to activate the Auditoring during" );
    declareProperty("DisableFor", m_veto,
                    "List of component names to disable the auditing for" );
    declareProperty("EnableFor", m_list );
    declareProperty("ProfileFreq", m_freq = -1,
                    "The frequence to audit events. -1 means all events" );
    declareProperty("DoFullEventProfile", m_fullEventAudit = false,
                    "If true, instead of individually auditing components, the full event (or events) will be audited in one go" );
    declareProperty("FullEventNSampleEvents", m_nSampleEvents = 1,
                    "The number of events to include in a full event audit, if enabled" );
    declareProperty("SkipEvents", m_eventsToSkip = 0,
                    "Number of events to skip before activating the auditing" );
    declareProperty("SkipSequencers", m_skipSequencers = true,
                    "If true, auditing will be skipped for Sequencer objects." );
  }

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
  class HeapProfiler : public AuditorBase
  {

  public:

    /// Constructor
    HeapProfiler( const std::string& name, ISvcLocator* pSvcLocator)
      : AuditorBase( name, pSvcLocator )
    {
      declareProperty( "DumpHeapProfiles",    m_dumpProfileHeaps   = true  );
      declareProperty( "PrintProfilesToLog",  m_printProfilesToLog = false );
    }

  protected:

    void google_before(const std::string& s)
    {
      HeapProfilerStart(s.c_str());
    }

    void google_after(const std::string& s)
    {
      if ( m_dumpProfileHeaps )
      {
        HeapProfilerDump(s.c_str());
      }
      if ( m_printProfilesToLog )
      {
        const char * profile = GetHeapProfile();
        m_log << MSG::INFO << profile << endmsg;
        delete profile;
      }
      HeapProfilerStop();
    }

    bool alreadyRunning() { return IsHeapProfilerRunning(); }

  private:

    bool m_dumpProfileHeaps;
    bool m_printProfilesToLog;

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
  class HeapChecker : public AuditorBase
  {

  public:

    /// Constructor
    HeapChecker( const std::string& name, ISvcLocator* pSvcLocator)
      : AuditorBase ( name, pSvcLocator ),
        m_enabled   ( true ),
        m_checker   ( NULL )
    { }

    virtual ~HeapChecker() { delete m_checker; }

  public:

    StatusCode initialize()
    {
      const StatusCode sc = AuditorBase::initialize();
      if ( sc.isFailure() ) return sc;

      const char * HEAPCHECK = getenv("HEAPCHECK");
      if ( !HEAPCHECK )
      {
        m_log << MSG::FATAL
              << "Environment variable HEAPCHECK must be set to 'local'"
              << endmsg;
        return StatusCode::FAILURE;
      }
      if ( std::string(HEAPCHECK) != "local" )
      {
        m_log << MSG::WARNING
              << "Environment variable HEAPCHECK is set to " << HEAPCHECK
              << " Partial Program Heap Checking is disabled"
              << endmsg;
        m_enabled = false;
      }

      return sc;
    }

  protected:

    void google_before(const std::string& s)
    {
      if ( m_enabled && !m_checker )
      {
        m_checker = new HeapLeakChecker(s.c_str());
      }
    }

    void google_after(const std::string& s)
    {
      if ( m_enabled && m_checker )
      {
        if ( ! m_checker->NoLeaks() )
        {
          m_log << MSG::WARNING << "Leak detected for " << s << endmsg;
        }
        delete m_checker;
        m_checker = NULL;
      }
    }

    bool alreadyRunning() { return m_enabled && m_checker != NULL ; }

  private:

    bool m_enabled;
    HeapLeakChecker * m_checker;

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
  class CPUProfiler : public AuditorBase
  {

  public:

    CPUProfiler( const std::string& name, ISvcLocator* pSvcLocator )
      : AuditorBase ( name, pSvcLocator ),
        m_running   ( false )
    { }

  protected:

    void google_before(const std::string& s)
    {
      if ( !m_running )
      {
        m_running = true;
        ProfilerStart((s+".prof").c_str());
      }
    }

    void google_after(const std::string&)
    {
      if ( m_running )
      {
        ProfilerStop();
        m_running = false;
      }
    }

    bool alreadyRunning() { return m_running; }

  private:

    bool m_running;

  };

  DECLARE_COMPONENT( HeapProfiler )
  DECLARE_COMPONENT( HeapChecker  )
  DECLARE_COMPONENT( CPUProfiler  )

}
