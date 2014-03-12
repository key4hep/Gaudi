// ## Includes.
// * Standard libraries.
#include <algorithm>
#include <vector>
#include <stack>
#include <string>
#include <utility>
#include <memory>
#include <iomanip>
#include <sstream>

// * Boost
#include <boost/foreach.hpp>

// * Gaudi libraries.
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"

// * Intel User API
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "ittnotify.h"

typedef std::map<std::string, __itt_event> TaskTypes;

// Gaudi profiling auditor. The auditor use Intel API for control profiling
// flow. We need to run profiling  throw Intel Amplifier amplxe-cl command
// line tool.
class IntelProfilerAuditor: public Auditor, virtual public IIncidentListener {
public:
  // ## Public functions.
  IntelProfilerAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  // Overridden functions.
  void handle(const Incident& incident);
  using Auditor::before; // avoid hiding base-class methods
  void before(StandardEventType type, INamedInterface* i);
  using Auditor::after; // avoid hiding base-class methods
  void after(StandardEventType type, INamedInterface* i, const StatusCode& sc);
// ## Private attributes.
private:
  // Stack for store current component(algorithm) chain with useful
  // information for the auditor.
  struct stack_entity {
    stack_entity(const std::string& name_, bool status_,
      const __itt_event event_ = 0, const __itt_event parent_event_ = 0):
        name(name_),
        status(status_),
        event(event_),
        parent_event(parent_event_){}
    // Name of the component.
    std::string name;
    // Running status: on/off.
    bool status;
    // Task type holder.
    __itt_event event;
    // Parent task type.
    __itt_event parent_event;
  };
private:
  // From what event to start profiling. Default = 1.
  int m_nStartFromEvent;
  // After what event we stop profiling. If 0 than we also profile finalization
  // stage. Default = 0.
  int m_nStopAtEvent;
   // Names of excluded algorithms.
  std::vector<std::string> m_excluded;
  // Names of included algorithms.
  std::vector<std::string> m_included;
  // Algorithm name, for which intel amplifier event type will be created.
  std::vector<std::string> m_algs_for_tasktypes;
  // The String delimiter between sequences/algorithms names in
  // "Task Type" grouping at Amplifier.
  std::string m_alg_delim;
  // Enable frames (needed for detecting slow events).
  bool m_enable_frames;
  // Frames rate. The recommended maximum rate for calling the Frame API is
  // 1000 frames (events) per second. A higher rate may result in large product
  // memory consumption and slow finalization.
  // You need update "slow-frames-threshold" and "fast-frames-threshold"
  // parameters of amplxe-cl tool to separate slow, medium and fast events.
  int m_frames_rate;
private:
  // Logger.
  MsgStream m_log;
  // Events counter.
  int m_nEvents;
  // Domain for event loop.
  __itt_domain* domain;
  // True if profiler is started.
  bool m_isStarted;
  // Current stack of sequences/algorithms.
  std::vector<stack_entity> m_stack;
  // Mapping of task type name to Amplifier event .
  TaskTypes m_tasktypes;
private:
  // ## Private functions.
  void start_profiling_component(const std::string& name);
  void skip_profiling_component(const std::string& name);

  void start();
  void pause();
  void resume();
  void stop();

  bool hasIncludes() const;
  bool isIncluded(const std::string& name) const;
  bool isExcluded(const std::string& name) const;
  bool isRunning() const;

  int  stackLevel() const;
  std::string stackIndent(bool newLevel = false) const;
  std::string taskTypeName(const std::string& component_name) const;
};
// ## Implementation.
// Constructor
IntelProfilerAuditor::IntelProfilerAuditor(const std::string& name,
  ISvcLocator* pSvcLocator) : Auditor(name, pSvcLocator), m_log(msgSvc(), name)
    ,m_nEvents(0), m_isStarted(false) {
  // ## Properties
  declareProperty("IncludeAlgorithms", m_included,
    "Names of included algorithms."
  );
  declareProperty("ExcludeAlgorithms", m_excluded,
    "Names of excluded algorithms."
  );
  declareProperty("StartFromEventN", m_nStartFromEvent = 1,
    "After what event we stop profiling. "
    "If 0 than we also profile finalization stage."
  );
  declareProperty("StopAtEventN", m_nStopAtEvent = 0,
    "After what event we stop profiling. "
    "If 0 than we also profile finalization stage. Default = 0."
  );
  declareProperty("ComponentsForTaskTypes", m_algs_for_tasktypes,
    "Algorithm name, for which intel amplifier task type will be created."
    "By default all algorithms have a corresponding task type.");
  declareProperty("TaskTypeNameDelimeter", m_alg_delim = " ",
    "The String delimiter between sequences/algorithms names in "
    "\"Task Type\" grouping at Amplifier. Default=\" \"."
  );
  declareProperty("EnableFrames", m_enable_frames = false,
    "Enable frames (needed for detecting slow events). Default=false."
  );
  declareProperty("FramesRate", m_frames_rate = 100,
    "Frames rate. The recommended maximum rate for calling the Frame API is "
    "1000 frames (events) per second. A higher rate may result in large product"
    " memory consumption and slow finalization. "
    "You need update \"slow-frames-threshold\" and \"fast-frames-threshold\" "
    "parameters of amplxe-cl tool to separate slow, medium and fast events. "
    "For use frames you need to switch on \"EnableFrames\". "
    "Default=100"
  );
}

void IntelProfilerAuditor::start() {
  m_isStarted = true;
  __itt_resume();
}

void IntelProfilerAuditor::start_profiling_component(const std::string& name) {
  if (!m_isStarted) return;
  std::string typeName = taskTypeName(name);
  __itt_event taskId = 0;
  TaskTypes::const_iterator iter = m_tasktypes.find(typeName);
  if( iter != m_tasktypes.end()) {
    taskId = iter->second;
  }

  if(!taskId && m_algs_for_tasktypes.empty()) {
    // Create event
    taskId = __itt_event_create(typeName.c_str(), typeName.size());
    m_tasktypes.insert(TaskTypes::value_type(typeName, taskId));
  }

  stack_entity state = stack_entity(name, true, taskId);
  stack_entity* parent = !m_stack.empty()?&m_stack.back():NULL;

  if (parent != NULL) {
    if (parent->event) {
       state.parent_event = parent->event;
    } else {
      state.parent_event = parent->parent_event;
    }
  }

  if (taskId && state.parent_event) {
    m_log << MSG::DEBUG << stackIndent() << "Pause event " <<
      state.parent_event << endmsg;
    __itt_event_end(state.parent_event);
  }
  m_stack.push_back(state);

  m_log << MSG::DEBUG << stackIndent() << "Start profiling component "
    << typeName << endmsg;

  if (taskId) {
     // Start event
    m_log << MSG::DEBUG << stackIndent() << "Start event type "
      << state.event << " for "  << typeName << endmsg;
    __itt_event_start(state.event);
  }

  __itt_resume();
}

void IntelProfilerAuditor::resume() {
  if (!m_isStarted) return;
  m_log << MSG::DEBUG << stackIndent() << "Resume" << endmsg;
  __itt_resume();
}

void IntelProfilerAuditor::pause() {
  if (!m_isStarted) return;
  m_log << MSG::DEBUG << stackIndent() << "Pause" << endmsg;
  __itt_pause();
}

void IntelProfilerAuditor::skip_profiling_component(const std::string& name) {
  if (!m_isStarted) return;
  m_stack.push_back(stack_entity(name, false));
  m_log << MSG::DEBUG << stackIndent() << "Skip component "
    << name << endmsg;
}

void IntelProfilerAuditor::stop() {
  if (!m_isStarted) return;
  m_isStarted = false;
  __itt_pause();
}

bool IntelProfilerAuditor::hasIncludes() const {
  return !m_included.empty();
}

bool IntelProfilerAuditor::isIncluded(const std::string& name) const {
  return std::find(m_included.begin(), m_included.end(), name) !=
    m_included.end();
}

bool IntelProfilerAuditor::isExcluded(const std::string& name) const {
  return std::find(m_excluded.begin(), m_excluded.end(), name) !=
    m_excluded.end();
}

bool IntelProfilerAuditor::isRunning() const {
  return !m_stack.empty() && m_stack.back().status;
}

int IntelProfilerAuditor::stackLevel() const {
  return m_stack.size();
}

std::string IntelProfilerAuditor::stackIndent(bool newLevel) const{
  std::stringstream indent(std::stringstream::out);
  indent << std::setw(stackLevel()*2+(newLevel?2:0)) << " ";
  return indent.str();
}

std::string
IntelProfilerAuditor::taskTypeName(const std::string& component_name) const {
  std::string result;
  std::string delim = "";
  BOOST_FOREACH(const stack_entity& value,
     m_stack) {
    result += delim+value.name;
    delim = m_alg_delim;
  }
  return result+m_alg_delim+component_name;
}

StatusCode IntelProfilerAuditor::initialize() {
  m_log.setLevel(outputLevel());
  m_log << MSG::INFO << "Initialised" << endmsg;

  IIncidentSvc * inSvc = NULL;
  const StatusCode sc = serviceLocator()->service("IncidentSvc", inSvc);
  if (sc.isFailure())
    return sc;
  // Useful to start profiling only after some event, we don't need profile
  // initialization stage. For that we need to count events with BeginEvent
  // listener.
  inSvc->addListener(this, IncidentType::BeginEvent);
  // If the end event number don't setup we finish profiling at the end
  // of loop. We don't need profiling finalization stage.
  inSvc->addListener(this, IncidentType::EndProcessing);

  std::string str_excluded, str_included, str_eventtypes;
  BOOST_FOREACH(const std::string& name, m_excluded)
  {
    str_excluded += " " + name;
  }
  BOOST_FOREACH(const std::string& name, m_included)
  {
    str_included += " " + name;
  }

  BOOST_FOREACH(const std::string& name, m_algs_for_tasktypes)
  {
    str_eventtypes += " " + name;
  }

  if (!m_included.empty()) {
    m_log << MSG::INFO << "Included algorithms (" << m_included.size()
      << "): " << str_included << endmsg;
  }

  if (!m_excluded.empty()){
    m_log << MSG::INFO << "Excluded algorithms (" << m_excluded.size()
      << "): " << str_excluded << endmsg;
  }

  if (!m_algs_for_tasktypes.empty()){
    m_log << MSG::INFO << "Event types (" << m_algs_for_tasktypes.size()
      << "): " << str_eventtypes << endmsg;
  }

  // Create a profiler domain for detection of slow events.
  domain = __itt_domain_create("Event loop");
  domain->flags = m_enable_frames;

  return StatusCode::SUCCESS;
}



void IntelProfilerAuditor::handle(const Incident& incident) {
  if (IncidentType::BeginEvent != incident.type()) return;
  // Increment the event counter
  ++m_nEvents;

  if (m_nStartFromEvent == m_nEvents) {
    m_log << MSG::INFO << "Start profiling (event #" << m_nEvents << ")"
        << endmsg;
    start();
  }

  if (m_nStopAtEvent == m_nEvents) {
    m_log << MSG::INFO << "Stop profiling (event #" << m_nEvents << ")"
        << endmsg;
    stop();
  }
}

void IntelProfilerAuditor::before(StandardEventType type, INamedInterface* i) {
  // Skip unnecessary event types.
  if (!((type == IAuditor::Execute) && m_isStarted)) return;

  // Name of the current component.
  const std::string& name = i->name();
  //m_log << MSG::DEBUG <<  "Before: " << name << " " << type << endmsg;

  if (isRunning()) {
    if (isExcluded(name)) {
      // If profiling is running and component is explicitly excluded
      // then skip component.
      skip_profiling_component(name);
    }else{
      // If profiling is running and component is'not explicitly excluded
      // then start profiling for component (add to stack).
      start_profiling_component(name);
    }
  }else {
    if (hasIncludes()) {
      // If the profiling is not running and  "includes" is explicitly defined ...
      if (isIncluded(name)) {
        // and the current component is in the include's list then start the
        // component profiling.
        start_profiling_component(name);
      } else{
        // and the current component is not in the includes list then skip
        // a profiling of the component.
        skip_profiling_component(name);
      }
    }else {
        // If "Includes" property isn't present and the component is ...
        if (isExcluded(name)) {
          // in the excludes list then skip a profiling
          skip_profiling_component(name);
        }else{
          // not in the exclude list then start a profiling
          start_profiling_component(name);
        }
    }
  }
  if (m_nEvents % m_frames_rate == 0) {
    __itt_frame_begin_v3(domain, NULL);
  }

}

void IntelProfilerAuditor::after(StandardEventType type,
  INamedInterface* i, const StatusCode&/* sc*/) {
  // Skip unnecessary event types
  if (!((type == IAuditor::Execute) && m_isStarted)) return;

  if ((m_nEvents+1) % m_frames_rate == 0) {
    __itt_frame_end_v3(domain, NULL);
  }

  // Name of the current component
  const std::string& name = i->name();
  stack_entity state = m_stack.back();
  // Remove component from stack.
  m_stack.pop_back();


  if (state.event != 0) {
     m_log << MSG::DEBUG << stackIndent(true) << "End event for "
      << name << endmsg;
    __itt_event_end(state.event);

    if (state.parent_event != 0) {
      m_log << MSG::DEBUG << stackIndent() << "Resume event for "
        << state.parent_event << endmsg;
      __itt_event_start(state.parent_event);
    }
  }

  if (m_stack.empty()) {
    // Pause if there are no parent components (top algorithm).
    pause();
  } else if (state.status) {
    // If the profiling is running and we have parent component that is
    // paused then pause the profiling.
    if (!m_stack.back().status) {
      pause();
    }
  }else {
    // If the profiling was stopped, but the parent component should be profiled
    // then resume profiling.
    if (m_stack.back().status) {
      resume();
    }
  }
}

// Register the auditor
DECLARE_COMPONENT(IntelProfilerAuditor)
