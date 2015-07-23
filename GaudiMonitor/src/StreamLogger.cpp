#include "StreamLogger.h"

#include "GaudiKernel/IMessageSvc.h"

#include <stdexcept>
#include <fstream>

StreamLogger::StreamLogger(const std::string& file)
    : m_ost{ new std::ofstream(file.c_str()), true }
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
  m_ost = std::unique_ptr<std::ostream,maybeDelete>( &ost, false );
  if (ost == std::cerr) {
    m_name = "STDERR";
  } else if ( ost == std::cout) {
    m_name = "STDOUT";
  } else {
    m_name = "unknown ostream";
  }
}
