///////////////////////////////////////////////////////////////////////////
//
// $Id: ServiceHistory.cpp,v 1.2 2008/06/04 12:35:15 marcocle Exp $
//
// GaudiHistory/ServiceHistory.cpp
//
// Contains history information for a Service
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/ServiceHistory.h"
#include "GaudiKernel/JobHistory.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/Service.h"
#include <iostream>

using namespace std;


//
///////////////////////////////////////////////////////////////////////////
//

ServiceHistory::ServiceHistory():
  //  HistoryObj(),
  m_pService(0),
  m_name("none"),
  m_type("none")
{

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ServiceHistory::ServiceHistory(const IService* isv, const JobHistory* job):
  //  HistoryObj(),
  m_pService(isv),
  m_jobHistory(job),
  m_name(isv->name())
{

  const Service *svc = dynamic_cast<const Service*>(isv);
  m_type = System::typeinfoName(typeid(*svc));
  m_properties = svc->getProperties();

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ServiceHistory::ServiceHistory(const IService& isv, const JobHistory* job):
  //  HistoryObj(),
  m_pService(&isv),
  m_jobHistory(job),
  m_name(isv.name())
{
  
  const Service *svc = dynamic_cast<const Service*>(&isv);
  m_type = System::typeinfoName(typeid(*svc));
  m_properties = svc->getProperties();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ServiceHistory::~ServiceHistory() {

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& ServiceHistory::classID() {

  static CLID CLID_ServiceHistory = 187225489;   // from `clid ServiceHistory`
  return CLID_ServiceHistory;

}

//**********************************************************************
// Free functions.
//**********************************************************************

// Output stream.

ostream& operator<<(ostream& lhs, const ServiceHistory& rhs) {
  lhs << "Name: " << rhs.name() << endl;
  lhs << "Type: " << rhs.type() << endl;

  //Properties
  lhs << "Properties: [" << endl;

  for ( ServiceHistory::PropertyList::const_iterator
        ipprop=rhs.properties().begin();
        ipprop!=rhs.properties().end(); ++ipprop ) {
    const Property& prop = **ipprop;
    prop.fillStream(lhs);
    lhs << endl;
  }
  lhs << "]" << endl;

  return lhs;
}
