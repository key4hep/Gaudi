#define GAUDIKERNEL_ISSUESEVERITY_CPP 1

#include "GaudiKernel/IssueSeverity.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IIssueLogger.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/System.h"

#include <sstream>

bool IssueSeverity::m_init( false );
IIssueLogger* IssueSeverity::m_ers( nullptr );

IssueSeverity::~IssueSeverity()
{
  if ( !m_reported ) {
    report();
  }
}

void IssueSeverity::report()
{
  if ( !m_init ) {
    m_init = true;
    m_ers  = Gaudi::svcLocator()->service<IIssueLogger>( "IssueLogger" ).get();
  }
  if ( m_ers ) {
    m_ers->report( *this );
  } else {
    std::cerr << *this << std::endl;
  }
  m_reported = true;
}

std::string IssueSeverity::getOrigin() const { return m_file + ":" + std::to_string( m_line ); }
