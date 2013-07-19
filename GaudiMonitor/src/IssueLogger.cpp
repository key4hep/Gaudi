#include "IssueLogger.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Time.h"

#include <sstream>
#include <streambuf>
#include <algorithm>

#include "boost/bind.hpp"

using namespace std;

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

  for (int i=0; i<IssueSeverity::NUM_LEVELS; ++i) {
    m_logger[i] = 0;
  }

  m_msgSevMap[MSG::NIL]     = IssueSeverity::NIL;
  m_msgSevMap[MSG::VERBOSE] = IssueSeverity::VERBOSE;
  m_msgSevMap[MSG::DEBUG]   = IssueSeverity::DEBUG;
  m_msgSevMap[MSG::INFO]    = IssueSeverity::INFO;
  m_msgSevMap[MSG::WARNING] = IssueSeverity::WARNING;
  m_msgSevMap[MSG::ERROR]   = IssueSeverity::ERROR;
  m_msgSevMap[MSG::FATAL]   = IssueSeverity::FATAL;
  m_msgSevMap[MSG::ALWAYS]  = IssueSeverity::ALWAYS;

  m_sevMsgMap[IssueSeverity::NIL]           = MSG::NIL;
  m_sevMsgMap[IssueSeverity::VERBOSE]       = MSG::VERBOSE;
  m_sevMsgMap[IssueSeverity::DEBUG]         = MSG::DEBUG;
  m_sevMsgMap[IssueSeverity::DEBUG1]        = MSG::DEBUG;
  m_sevMsgMap[IssueSeverity::DEBUG2]        = MSG::DEBUG;
  m_sevMsgMap[IssueSeverity::DEBUG3]        = MSG::DEBUG;
  m_sevMsgMap[IssueSeverity::INFO]          = MSG::INFO;
  m_sevMsgMap[IssueSeverity::WARNING]       = MSG::WARNING;
  m_sevMsgMap[IssueSeverity::RECOVERABLE]   = MSG::ERROR;
  m_sevMsgMap[IssueSeverity::ERROR]         = MSG::ERROR;
  m_sevMsgMap[IssueSeverity::FATAL]         = MSG::FATAL;
  m_sevMsgMap[IssueSeverity::ALWAYS]        = MSG::ALWAYS;

  m_levelTrans[IssueSeverity::VERBOSE]     = "VERBOSE";
  m_levelTrans[IssueSeverity::DEBUG]       = "DEBUG";
  m_levelTrans[IssueSeverity::DEBUG1]      = "DEBUG1";
  m_levelTrans[IssueSeverity::DEBUG2]      = "DEBUG2";
  m_levelTrans[IssueSeverity::DEBUG3]      = "DEBUG3";
  m_levelTrans[IssueSeverity::INFO]        = "INFO";
  m_levelTrans[IssueSeverity::WARNING]     = "WARNING";
  m_levelTrans[IssueSeverity::RECOVERABLE] = "RECOVERABLE";
  m_levelTrans[IssueSeverity::ERROR]       = "ERROR";
  m_levelTrans[IssueSeverity::FATAL]       = "FATAL";
  m_levelTrans[IssueSeverity::ALWAYS]      = "ALWAYS";

  m_levelSTrans["VERBOSE"]     = IssueSeverity::VERBOSE;
  m_levelSTrans["DEBUG"]       = IssueSeverity::DEBUG;
  m_levelSTrans["DEBUG1"]      = IssueSeverity::DEBUG1;
  m_levelSTrans["DEBUG2"]      = IssueSeverity::DEBUG2;
  m_levelSTrans["DEBUG3"]      = IssueSeverity::DEBUG3;
  m_levelSTrans["INFO"]        = IssueSeverity::INFO;
  m_levelSTrans["WARNING"]     = IssueSeverity::WARNING;
  m_levelSTrans["RECOVERABLE"] = IssueSeverity::RECOVERABLE;
  m_levelSTrans["ERROR"]       = IssueSeverity::ERROR;
  m_levelSTrans["FATAL"]       = IssueSeverity::FATAL;
  m_levelSTrans["ALWAYS"]      = IssueSeverity::ALWAYS;


}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

IssueLogger::~IssueLogger() {

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
IssueLogger::initialize() {

  StatusCode st = Service::initialize();
  if (st.isFailure()) { return st; }

  setupDefaultLogger();

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

  for (int i=0; i<IssueSeverity::NUM_LEVELS; ++i) {
    IssueSeverity::Level j = IssueSeverity::Level (i);
    delete m_logger[j];
  }

  return Service::finalize();
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::getTraceBack(std::string& stack) {
  const int depth = 30;
  const int offset = 5;
  System::backTrace(stack, depth, offset);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
IssueLogger::connect(const std::string& ident) {

  MsgStream log ( msgSvc(), name() );
  Tokenizer tok(true);

  string::size_type loc = ident.find(" ");
//  string stream = ident.substr(0,loc); // icc remark #177: variable "stream" was declared but never referenced
//   typedef std::pair<std::string,std::string>      Prop;
//   std::vector<Prop> props;
  string val,VAL,TAG,filename;

  tok.analyse(ident.substr(loc+1,ident.length()), " ", "", "", "=", "'", "'");

  for ( Tokenizer::Items::iterator i = tok.items().begin();
	i != tok.items().end(); i++)    {
    const std::string& tag = (*i).tag();
    TAG = tag;
    toupper(TAG);

    val = (*i).value();
    VAL = val;
    toupper(VAL);

    IssueSeverity::Level level;

    if (TAG == "DEBUG") {
      level = IssueSeverity::DEBUG;
    } else if ( TAG == "INFO") {
      level = IssueSeverity::INFO;
    } else if ( TAG == "WARNING") {
      level = IssueSeverity::WARNING;
    } else if ( TAG == "RECOVERABLE") {
      level = IssueSeverity::RECOVERABLE;
    } else if ( TAG == "ERROR") {
      level = IssueSeverity::ERROR;
    } else if ( TAG == "FATAL") {
      level = IssueSeverity::FATAL;
    } else {
      log << MSG::ERROR << "Unknown output level \"" << TAG << "\""
	  << endmsg;
      continue;
    }

    if (m_logger[level] != 0) {
      log << MSG::INFO << "closing stream " << m_logger[level]->name()
	  << endmsg;
      delete m_logger[level];
      m_logger[level] = 0;
    }

    if (val == "MsgSvc") {
      m_logger[level] = new StreamLogger(msgSvc(), m_sevMsgMap[level]);
      m_log[level] =
	boost::bind(&StreamLogger::WriteToMsgSvc, m_logger[level],
		    _1);
    } else if (val == "STDERR") {
      m_logger[level] = new StreamLogger(std::cerr);
      m_log[level] =
	boost::bind(&StreamLogger::WriteToStream, m_logger[level],
		    _1);
    } else if (val == "STDOUT") {
      m_logger[level] = new StreamLogger(std::cout);
      m_log[level] =
	boost::bind(&StreamLogger::WriteToStream, m_logger[level],
		    _1);
    } else { // A file
      try {
        m_logger[level] = new StreamLogger(val.c_str());
      }
      catch (std::exception&) {
        m_logger[level] = 0;
        log << MSG::ERROR << "Unable to open file \"" << VAL
	    << "\" for writing issues at level " << TAG << endmsg;
        return StatusCode::FAILURE;
      }
      m_log[level] =
        boost::bind(&StreamLogger::WriteToStream, m_logger[level], _1);
    }
    log << MSG::DEBUG << "Writing " << m_levelTrans[level]
	<< " issues to " << m_logger[level]->name() << endmsg;

  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::report(IssueSeverity::Level lev, const std::string& str,
		    const std::string& org) {

  if ( lev < m_reportLevel) return;

  std::string msg = m_levelTrans[lev] + "  " + org + "  \"" + str + "\"";

  if (m_showTime) {
    msg += " [" + Gaudi::Time::current().format(true, "%H:%M:%S %Y/%m/%d %Z") +"]";
  }

  if (lev >= m_traceLevel) {
    std::string stack;
    getTraceBack(stack);
    msg += "\n" + stack;
  }


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
  if (sap == 0) {
    MsgStream log ( msgSvc(), name() );
    log << MSG::ERROR << "Could not convert " << prop.name()
	<< "to a StringProperty (which it should be!)" << endmsg;
    return;
  }

  std::string val = sap->value();

  if (prop.name() == "ReportLevel") {
    if (m_levelSTrans.find(val) == m_levelSTrans.end()) {
      MsgStream log ( msgSvc(), name() );
      log << MSG::ERROR
	  << "Option ReportLevel: unknown Issue Severity level \""
	  << val << "\". Setting it WARNING" << endmsg;
      m_reportLevel = IssueSeverity::WARNING;
      return;
    } else {
      m_reportLevel = m_levelSTrans[m_reportLevelS];
    }
  } else if (prop.name() == "TracebackLevel") {
    if (m_levelSTrans.find(val) == m_levelSTrans.end()) {
      MsgStream log ( msgSvc(), name() );
      log << MSG::ERROR
	  << "Option TracebackLevel: unknown Issue Severity level \""
	  << val << "\". Setting it to ERROR" << endmsg;
      m_traceLevel = IssueSeverity::ERROR;
      return;
    } else {
      m_traceLevel = m_levelSTrans[m_traceLevelS];
    }
  } else {
      MsgStream log ( msgSvc(), name() );
      log << MSG::ERROR << "setting up unknown property \"" << prop.name()
	  << "\"" << endmsg;
      return;
  }

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::setupStreams(Property& prop) {

  StringArrayProperty *sap = dynamic_cast<StringArrayProperty*>( &prop );
  if (sap == 0) {
    MsgStream log ( msgSvc(), name() );
    log << MSG::ERROR << "Could not convert " << prop.name()
	<< "to a StringArrayProperty (which it should be!)" << endmsg;
    return;
  }

  vector<string>::const_iterator itr;
  for (itr = sap->value().begin(); itr != sap->value().end(); ++itr) {
    if (connect(*itr).isFailure()) {
      MsgStream log ( msgSvc(), name() );
      log << MSG::ERROR << "Could not setup stream " << *itr << endmsg;
    }
  }

  return;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
IssueLogger::setupDefaultLogger() {
  for (int i=1; i<IssueSeverity::NUM_LEVELS; ++i) {
    if (m_logger[i] == 0) {
      // default: dump to msgSvc
      IssueSeverity::Level j = IssueSeverity::Level (i);

      m_logger[j] = new StreamLogger(msgSvc(), m_sevMsgMap[j]);
      m_log[j] = boost::bind(&StreamLogger::WriteToMsgSvc, m_logger[j],
			     _1);

      MsgStream log ( msgSvc(), name() );
      log << MSG::DEBUG << "Writing " << m_levelTrans[j]
	  << " issues to " << m_logger[j]->name() << endmsg;

    }
  }
}
