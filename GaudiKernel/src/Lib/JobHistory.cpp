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

const CLID& 
JobHistory::classID() {

  static CLID CLID_JobHistory = 247994533;
  return CLID_JobHistory;

}

void
JobHistory::addProperty(const std::string& client, const Property* prop) {
//  if (m_props.find(prop) == m_props.end()) {
    m_ppl.push_back( std::pair<std::string, const Property*>(client,prop) );
//  }
}


void 
JobHistory::dump(std::ostream& ost, const bool isXML, int /*ind*/) const {

  if (!isXML) {
    ost << "Release: " << release_version() << endl;
    ost << "OS:      " << os() << endl;
    ost << "OS ver:  " << os_version() << endl;
    ost << "Host:    " << hostname() << endl;
    ost << "Machine: " << machine() << endl;
    ost << "Run dir: " << dir() << endl;
    ost << "CMTCONFIG: " << cmtconfig() << endl;
    ost << "Job start time: " << start_time() << endl << endl;
    ost << "Properties: [" << endl;;
    for ( JobHistory::PropertyPairList::const_iterator
	    ipprop=propertyPairs().begin();
	  ipprop!=propertyPairs().end(); ++ipprop ) {
      std::string client = ipprop->first;
      const Property* prop = ipprop->second;
      ost << client << ":  ";
      prop->fillStream(ost);
      ost << endl;
    }
    ost << "]" << endl;
    vector<string> env = environment();
    for (vector<string>::const_iterator itr=env.begin();  itr != env.end(); 
	 ++itr) {
      ost << *itr << endl;
    }
  } else {

  }

}



//**********************************************************************
// Free functions.
//**********************************************************************

// Output stream.

ostream& operator<<(ostream& lhs, const JobHistory& rhs) {

  rhs.dump(lhs,false);

  return lhs;
}

//**********************************************************************
