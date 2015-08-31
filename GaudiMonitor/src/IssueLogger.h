#ifndef GAUDISVC_ISSUELOGGER_H
#define GAUDISVC_ISSUELOGGER_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIssueLogger.h"
#include "GaudiKernel/IssueSeverity.h"
#include "GaudiKernel/Property.h"
#include "StreamLogger.h"
#include <string>
#include <vector>
#include <map>


class IssueLogger: public extends1<Service, IIssueLogger> {
public:
  IssueLogger(const std::string& name, ISvcLocator *svc );
  ~IssueLogger() override = default;

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  void report(IssueSeverity::Level level, const std::string& msg,
              const std::string& origin);
  void report(const IssueSeverity& err);

private:

  StringArrayProperty m_outputfile;
  StringProperty m_reportLevelS, m_traceLevelS;
  BooleanProperty m_showTime;
  IssueSeverity::Level m_reportLevel, m_traceLevel;

  class logger_t final {
      std::unique_ptr<StreamLogger> m_logger ;
      void (StreamLogger::*m_fun)(const std::string&) = nullptr;
  public:
      logger_t() = default;
      logger_t( StreamLogger* logger, void (StreamLogger::*fun)(const std::string&)  ) 
          : m_logger{ logger }, m_fun{ fun } {} 
      void operator()(const std::string& s) const { (m_logger.get()->*m_fun)(s); }
      const std::string& name() const { return m_logger->name(); }
      explicit operator bool () const { return bool(m_logger); }
      void reset() { m_logger.reset(); }
  };
  std::array<logger_t,IssueSeverity::NUM_LEVELS> m_log;

  StatusCode connect( const std::string& );
  void setupDefaultLogger();
  void setupLevels(Property& prop);
  void setupStreams(Property& prop);

};

#endif
