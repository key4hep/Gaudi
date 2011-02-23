#ifndef GAUDISVC_STREAMLOGGER_H
#define GAUDISVC_STREAMLOGGER_H

#include "GaudiKernel/MsgStream.h"

#include <ostream>
#include <string>

class IMessageSvc;

class StreamLogger {
public:
  StreamLogger(const std::string& file);
  StreamLogger(std::ostream &ost);
  StreamLogger(IMessageSvc*, MSG::Level);
  //  StreamLogger();
  ~StreamLogger();

  std::string name() const;

  void WriteToStream(const std::string& str) { *p_ost << str << std::endl; }
  void WriteToMsgSvc(const std::string& str) { *p_msgStr << m_level << str
							 << endmsg; }


private:
  bool m_isMine;
  std::ostream *p_ost;
  MsgStream *p_msgStr;
  MSG::Level m_level;
  std::string m_name;
};


#endif
