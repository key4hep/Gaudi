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

  std::string name() const { return m_name; }

  void WriteToStream(const std::string& str) { *m_ost << str << std::endl; }
  void WriteToMsgSvc(const std::string& str) { *m_msgStr << m_level << str
							 << endmsg; }


private:
  struct maybeDelete {
      bool m_isMine;
      maybeDelete(bool isMine = false) : m_isMine(isMine) {}
      maybeDelete& operator=(maybeDelete&& rhs) { m_isMine = rhs.m_isMine; rhs.m_isMine = false; return *this; }
      maybeDelete& operator=(maybeDelete& rhs) = delete;
      maybeDelete(maybeDelete&& rhs) { *this = std::move(rhs); }
      maybeDelete(const maybeDelete& rhs) = delete;
      template <typename P>
      void operator()(P* p) { if (m_isMine) delete p; }
  };
  std::unique_ptr<std::ostream,maybeDelete> m_ost;
  std::unique_ptr<MsgStream> m_msgStr;
  MSG::Level m_level;
  std::string m_name;
};


#endif
