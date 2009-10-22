#include "StreamLogger.h"

#include "GaudiKernel/IMessageSvc.h"

#include <stdexcept>
#include <fstream>

// StreamLogger::StreamLogger(): m_isMine(0), p_ost(0), p_msgStr(0), m_level(0){};

StreamLogger::StreamLogger(const std::string& file): m_isMine(1),
						     p_ost(0), p_msgStr(0) {
  m_name = "file:" + file;
  p_ost = new std::ofstream(file.c_str());
  if (p_ost == 0) {
    throw std::invalid_argument("can't open file");
  }
}

StreamLogger::StreamLogger(IMessageSvc *svc, MSG::Level lev):
  m_isMine(1), p_ost(0), p_msgStr(0) {
  p_msgStr = new MsgStream( svc, "IssueLogger");
  m_level = lev;
  m_name = "MsgStream";
}

StreamLogger::StreamLogger(std::ostream& ost): m_isMine(0) , p_ost(0),
					       p_msgStr(0){
  p_ost = &ost;
  if (ost == std::cerr) {
    m_name = "STDERR";
  } else if ( ost == std::cout) {
    m_name = "STDOUT";
  } else {
    m_name = "unknown ostream";
  }
}

std::string
StreamLogger::name() const {
  return m_name;
}

StreamLogger::~StreamLogger() {
  if (m_isMine && p_ost != 0) {
    delete p_ost;
  }
  if (p_msgStr != 0) {
    delete p_msgStr;
  }
}
