#include "IssueLogger.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Time.h"

#include <algorithm>
namespace {

  std::string getTraceBack() {
    std::string stack;
    constexpr int depth = 30;
    constexpr int offset = 5;
    System::backTrace(stack, depth, offset);
    return stack;
  }

  static const std::map<IssueSeverity::Level, MSG::Level> s_sevMsgMap = 
                { { IssueSeverity::NIL,         MSG::NIL     },
                  { IssueSeverity::VERBOSE,     MSG::VERBOSE },
                  { IssueSeverity::DEBUG,       MSG::DEBUG   },
                  { IssueSeverity::DEBUG1,      MSG::DEBUG   },
                  { IssueSeverity::DEBUG2,      MSG::DEBUG   },
                  { IssueSeverity::DEBUG3,      MSG::DEBUG   },
                  { IssueSeverity::INFO,        MSG::INFO    },
                  { IssueSeverity::WARNING,     MSG::WARNING },
                  { IssueSeverity::RECOVERABLE, MSG::ERROR   },
                  { IssueSeverity::ERROR,       MSG::ERROR   },
                  { IssueSeverity::FATAL,       MSG::FATAL   },
                  { IssueSeverity::ALWAYS,      MSG::ALWAYS  } };

  static const std::map<IssueSeverity::Level, std::string> s_levelTrans =
                 { { IssueSeverity::VERBOSE,      "VERBOSE" },
                   { IssueSeverity::DEBUG,        "DEBUG" },
                   { IssueSeverity::DEBUG1,       "DEBUG1" },
                   { IssueSeverity::DEBUG2,       "DEBUG2" },
                   { IssueSeverity::DEBUG3,       "DEBUG3" },
                   { IssueSeverity::INFO,         "INFO" },
                   { IssueSeverity::WARNING,      "WARNING" },
                   { IssueSeverity::RECOVERABLE,  "RECOVERABLE" },
                   { IssueSeverity::ERROR,        "ERROR" },
                   { IssueSeverity::FATAL,        "FATAL" },
                   { IssueSeverity::ALWAYS,       "ALWAYS" } };

  static const std::map<std::string, IssueSeverity::Level> s_levelSTrans = 
                  { { "VERBOSE",      IssueSeverity::VERBOSE },
                    { "DEBUG",        IssueSeverity::DEBUG },
                    { "DEBUG1",       IssueSeverity::DEBUG1 },
                    { "DEBUG2",       IssueSeverity::DEBUG2 },
                    { "DEBUG3",       IssueSeverity::DEBUG3 },
                    { "INFO",         IssueSeverity::INFO },
                    { "WARNING",      IssueSeverity::WARNING },
                    { "RECOVERABLE",  IssueSeverity::RECOVERABLE },
                    { "ERROR",        IssueSeverity::ERROR },
                    { "FATAL",        IssueSeverity::FATAL },
                    { "ALWAYS",       IssueSeverity::ALWAYS } };
}

DECLARE_COMPONENT(IssueLogger)

//*************************************************************************//
inline void toupper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 (int(*)(int)) toupper);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

IssueLogger::IssueLogger( const std::string& name, ISvcLocator* svc )
  : base_class(name, svc) {

  declareProperty ("Output", m_outputfile );
  declareProperty ("ReportLevel", m_reportLevelS="WARNING");
  declareProperty ("TracebackLevel", m_traceLevelS="ERROR");
  declareProperty ("ShowTime", m_showTime=false);

  m_reportLevelS.declareUpdateHandler(&IssueLogger::setupLevels,this);
  m_traceLevelS.declareUpdateHandler(&IssueLogger::setupLevels,this);
  m_outputfile.declareUpdateHandler(&IssueLogger::setupStreams,this);

  m_reportLevel = IssueSeverity::WARNING;
  m_traceLevel  = IssueSeverity::ERROR;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
IssueLogger::initialize() {

  StatusCode st = Service::initialize();
  if (st.isSuccess()) { setupDefaultLogger(); }
  return st;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
IssueLogger::reinitialize() {

  MsgStream log ( msgSvc(), name() );
  log << MSG::WARNING << "reinitialize not implemented" << endmsg;
  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
IssueLogger::finalize() {

  MsgStream log ( msgSvc(), name() );
  log << MSG::DEBUG << "IssueLogger::finalize" << endmsg;
  std::for_each( std::begin(m_log), std::end(m_log), 
                 [](logger_t& i) 
                 { i.reset(); } );
  return Service::finalize();
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
IssueLogger::connect(const std::string& ident) {

  MsgStream log ( msgSvc(), name() );

  auto loc = ident.find(" ");
  using Parser = Gaudi::Utils::AttribStringParser;
  // note: if loc == string::npos then loc + 1 == 0
  for (auto attrib: Parser(ident.substr(loc + 1))) {
    toupper(attrib.tag);
    IssueSeverity::Level level;
    if (attrib.tag == "DEBUG") {               level = IssueSeverity::DEBUG;
    } else if ( attrib.tag == "INFO") {        level = IssueSeverity::INFO;
    } else if ( attrib.tag == "WARNING") {     level = IssueSeverity::WARNING;
    } else if ( attrib.tag == "RECOVERABLE") { level = IssueSeverity::RECOVERABLE;
    } else if ( attrib.tag == "ERROR") {       level = IssueSeverity::ERROR;
    } else if ( attrib.tag == "FATAL") {       level = IssueSeverity::FATAL;
    } else {
      log << MSG::ERROR << "Unknown output level \"" << attrib.tag << "\""
	  << endmsg;
      continue;
    }

    if (m_log[level]) {
      log << MSG::INFO << "closing stream " << m_log[level].name() << endmsg;
      m_log[level].reset();
    }

    if (attrib.value == "MsgSvc") {
      m_log[level] = { new StreamLogger(msgSvc(), s_sevMsgMap.at(level)) , &StreamLogger::WriteToMsgSvc };
    } else if (attrib.value == "STDERR") {
      m_log[level] = { new StreamLogger(std::cerr), &StreamLogger::WriteToStream };
    } else if (attrib.value == "STDOUT") {
      m_log[level] = { new StreamLogger(std::cout),  &StreamLogger::WriteToStream };
    } else { // A file
      try {
        m_log[level] = { new StreamLogger(attrib.value), &StreamLogger::WriteToStream };
      }
      catch (std::exception&) {
        m_log[level].reset();
        log << MSG::ERROR << "Unable to open file \"" << attrib.value
            << "\" for writing issues at level " << attrib.tag << endmsg;
        return StatusCode::FAILURE;
      }
    }
    log << MSG::DEBUG << "Writing " << s_levelTrans.at(level)
                      << " issues to " << m_log[level].name() << endmsg;
  }
  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::report(IssueSeverity::Level lev, const std::string& str,
                    const std::string& org) {
  if ( lev < m_reportLevel) return;
  std::string msg = s_levelTrans.at(lev) + "  " + org + "  \"" + str + "\"";
  if (m_showTime) msg += " [" + Gaudi::Time::current().format(true, "%H:%M:%S %Y/%m/%d %Z") +"]";
  if (lev >= m_traceLevel) msg += "\n" + getTraceBack();
  m_log[lev](msg);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::report( const IssueSeverity &err ) {
  report(err.getLevel(), err.getMsg(), err.getOrigin());
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::setupLevels(Property& prop) {

  StringProperty *sap = dynamic_cast<StringProperty*> (&prop);
  if (!sap) {
    MsgStream log ( msgSvc(), name() );
    log << MSG::ERROR << "Could not convert " << prop.name()
	<< "to a StringProperty (which it should be!)" << endmsg;
    return;
  }

  std::string val = sap->value();
  auto set = [&](IssueSeverity::Level& key, IssueSeverity::Level def) {
    if (s_levelSTrans.find(val) == s_levelSTrans.end()) {
      key = def;
      MsgStream log ( this->msgSvc(), this->name() );
      log << MSG::ERROR << "Option " << prop.name() << ": unknown Issue Severity level \""
	                    << val << "\". Setting it " << s_levelTrans.at(def) << endmsg;
    } else {
      key = s_levelSTrans.at(val);
    }
  };

  if (prop.name() == "ReportLevel") {
    set(m_reportLevel, IssueSeverity::WARNING );
  } else if (prop.name() == "TracebackLevel") {
    set(m_traceLevel, IssueSeverity::ERROR );
  } else {
    MsgStream log ( msgSvc(), name() );
    log << MSG::ERROR << "setting up unknown property \"" 
                      << prop.name() << "\"" << endmsg;
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::setupStreams(Property& prop) {

  StringArrayProperty *sap = dynamic_cast<StringArrayProperty*>( &prop );
  if ( !sap ) {
    MsgStream log ( msgSvc(), name() );
    log << MSG::ERROR << "Could not convert " << prop.name()
	<< "to a StringArrayProperty (which it should be!)" << endmsg;
    return;
  }
  for (const auto& s : sap->value() ) {
    if (connect(s).isFailure()) {
      MsgStream log ( msgSvc(), name() );
      log << MSG::ERROR << "Could not setup stream " << s << endmsg;
    }
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::setupDefaultLogger() {
  for (int i=1; i<IssueSeverity::NUM_LEVELS; ++i) {
    if (!m_log[i]) {
      // default: dump to msgSvc
      IssueSeverity::Level j = IssueSeverity::Level (i);
      m_log[j] = {  new StreamLogger(msgSvc(), s_sevMsgMap.at(j)) , &StreamLogger::WriteToMsgSvc };
      MsgStream log ( msgSvc(), name() );
      log << MSG::DEBUG << "Writing " << s_levelTrans.at(j)
	                    << " issues to " << m_log[j].name() << endmsg;
    }
  }
}
