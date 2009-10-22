#define GAUDIKERNEL_ISSUESEVERITY_CPP 1

#include "GaudiKernel/IssueSeverity.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IIssueLogger.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"

#include <sstream>

bool IssueSeverity::m_init(false);
IIssueLogger* IssueSeverity::m_ers(0);

void IssueSeverity::init() {
  if (!m_init) {
    m_init = true;
    m_ers = Gaudi::svcLocator()->service<IIssueLogger>("IssueLogger").get();
  }
}

IssueSeverity::~IssueSeverity() {

  if (!m_reported) {
    report();
    // do stuff
  }

}

void
IssueSeverity::report() {

  if (m_ers != 0) {
    m_ers->report(this);
  } else {
    std::cerr << *this << std::endl;
  }
  m_reported = true;

}


std::string
IssueSeverity::getOrigin() const {
  std::ostringstream ost;
  ost << m_file << ":" << m_line;
  return ost.str();
}

IssueSeverity::operator StatusCode() const {
  if (m_level == IssueSeverity::RECOVERABLE) {
    return StatusCode(StatusCode::RECOVERABLE, *this);
  } else if (m_level < IssueSeverity::ERROR) {
    return StatusCode(StatusCode::SUCCESS, *this);
  } else {
    return StatusCode(StatusCode::FAILURE, *this);
  }
}
