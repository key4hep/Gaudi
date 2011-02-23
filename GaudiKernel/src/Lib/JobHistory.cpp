///////////////////////////////////////////////////////////////////////////
//
// $Id: JobHistory.cpp,v 1.1 2006/11/09 10:24:05 mato Exp $
//
// GaudiHistory/JobHistory.cpp
//
// Contains history information for a job. Imports all environment vars
// and info from uname
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/JobHistory.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Property.h"

#include <cstdlib>
#include <iostream>

using std::string;
using std::ostream;
using std::endl;
using std::vector;

//**********************************************************************
// Member functions.
//**********************************************************************

// Constructor.

JobHistory::JobHistory()
: m_start_time(0) {

  time(&m_start_time);

  std::string rel;

  if  ( (rel = System::getEnv("ATLAS_BASE_RELEASE")) != "UNKNOWN" ) {
    m_release_version = rel;
  } else if ( (rel = System::getEnv("GAUDIROOT")) != "UNKNOWN" ) {
    m_release_version = rel;
  } else {
    m_release_version = "UNKNOWN";
  }
  m_dir = System::getEnv("PWD");
  m_cmtconfig = System::getEnv("CMTCONFIG");

  m_osname = System::osName();
  m_hostname = System::hostName();
  m_os_version = System::osVersion();
  m_machine = System::machineType();

  m_environ = System::getEnv();
 
}

JobHistory::JobHistory(const std::string& rel, const std::string& os,
		       const std::string& host, const std::string& dir,
		       const std::string& osver, const std::string& mach,
		       const std::string& cmtconfig,
		       const time_t& time):
  m_release_version(rel), m_dir(dir), m_cmtconfig(cmtconfig), m_osname(os), 
  m_hostname(host),
  m_os_version(osver), m_machine(mach), m_start_time(time) {

}
  

//**********************************************************************

// Destructor.

JobHistory::~JobHistory() {
}

const CLID& JobHistory::classID() {

  static CLID CLID_JobHistory = 247994533;
  return CLID_JobHistory;

}

void
JobHistory::addProperty(const std::string& client, const Property* prop) {
//  if (m_props.find(prop) == m_props.end()) {
    m_props.push_back( std::pair<std::string, const Property*>(client,prop) );
//  }
}

//**********************************************************************
// Free functions.
//**********************************************************************

// Output stream.

ostream& operator<<(ostream& lhs, const JobHistory& rhs) {
  lhs << "Release: " << rhs.release_version() << endl;
  lhs << "OS:      " << rhs.os() << endl;
  lhs << "OS ver:  " << rhs.os_version() << endl;
  lhs << "Host:    " << rhs.hostname() << endl;
  lhs << "Machine: " << rhs.machine() << endl;
  lhs << "Run dir: " << rhs.dir() << endl;
  lhs << "CMTCONFIG: " << rhs.cmtconfig() << endl;
  lhs << "Job start time: " << rhs.start_time() << endl << endl;
  lhs << "Properties: [" << endl;;
  for ( JobHistory::PropertyList::const_iterator
        ipprop=rhs.properties().begin();
        ipprop!=rhs.properties().end(); ++ipprop ) {
     std::string client = ipprop->first;
     const Property* prop = ipprop->second;
     lhs << client << ":  ";
     prop->fillStream(lhs);
     lhs << endl;
  }
  lhs << "]" << endl;
  vector<string> env = rhs.environment();
  for (vector<string>::const_iterator itr=env.begin();  itr != env.end(); 
       ++itr) {
    lhs << *itr << endl;
  }
  return lhs;
}

//**********************************************************************
