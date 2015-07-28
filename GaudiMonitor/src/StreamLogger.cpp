#include "StreamLogger.h"

#include "GaudiKernel/IMessageSvc.h"

#include <fstream>

StreamLogger::StreamLogger(const std::string& file)
    : m_oost{ new std::ofstream(file.c_str()) }, m_ost{ m_oost.get() }
{
  m_name = "file:" + file;
  if (!m_ost) {
    throw std::invalid_argument("can't open file");
  }
}

StreamLogger::StreamLogger(IMessageSvc *svc, MSG::Level lev):
  m_msgStr{ new MsgStream( svc, "IssueLogger") }
{
  m_level = lev;
  m_name = "MsgStream";
}

StreamLogger::StreamLogger(std::ostream& ost)
{
  m_ost = &ost;
  if (ost == std::cerr) {
    m_name = "STDERR";
  } else if ( ost == std::cout) {
    m_name = "STDOUT";
  } else {
    m_name = "unknown ostream";
  }
}
