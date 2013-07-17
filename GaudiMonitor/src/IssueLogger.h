#ifndef GAUDISVC_ISSUELOGGER_H
#define GAUDISVC_ISSUELOGGER_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIssueLogger.h"
#include "GaudiKernel/IssueSeverity.h"
#include "GaudiKernel/Property.h"
#include "StreamLogger.h"
#include "boost/function.hpp"
#include <string>
#include <vector>
#include <map>


class IssueLogger: public extends1<Service, IIssueLogger> {
public:

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();

  IssueLogger(const std::string& name, ISvcLocator *svc );

  void report(IssueSeverity::Level level, const std::string& msg,
	      const std::string& origin);
  void report(const IssueSeverity& err);


protected:

  virtual ~IssueLogger();

private:

  StringArrayProperty m_outputfile;
  StringProperty m_reportLevelS, m_traceLevelS;
  BooleanProperty m_showTime;

  IssueSeverity::Level m_reportLevel, m_traceLevel;

  StreamLogger *m_logger[IssueSeverity::NUM_LEVELS];
  boost::function<void (const std::string&)> m_log[IssueSeverity::NUM_LEVELS];

  std::map<MSG::Level, IssueSeverity::Level> m_msgSevMap;
  std::map<IssueSeverity::Level, MSG::Level> m_sevMsgMap;
  std::map<IssueSeverity::Level, std::string> m_levelTrans;
  std::map<std::string, IssueSeverity::Level> m_levelSTrans;


  StatusCode connect( const std::string& );
  void getTraceBack(std::string& stack);

  MSG::Level sevToMsg(IssueSeverity::Level &lev);
  IssueSeverity::Level msgToSev(MSG::Level &lev);

  void setupDefaultLogger();
  void setupLevels(Property& prop);
  void setupStreams(Property& prop);

};

#endif
