#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ServiceLocatorHelper.h"

#include <sstream>
#include <string>
#include <ostream>

const DataObjID INVALID_DATAOBJID = DataObjID();

//---------------------------------------------------------------------------

DataObjectHandleBase::DataObjectHandleBase():
  Gaudi::DataHandle(), m_init(false), 
  m_goodType(false), m_optional(false), m_wasRead(false), m_wasWritten(false) {}

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase(const DataObjID & k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner):
  Gaudi::DataHandle(k,a,owner), m_init(false), m_goodType(false),
  m_optional(false), m_wasRead(false), m_wasWritten(false) {}

//---------------------------------------------------------------------------

DataObjectHandleBase::DataObjectHandleBase(const std::string & k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner):
  DataObjectHandleBase(DataObjID(k), a, owner){}

//---------------------------------------------------------------------------                                           

const std::string
DataObjectHandleBase::toString() const {
  std::ostringstream ost;
  ost << m_key << "|" << mode() << "|" << isOptional() << "|";
  for (auto &a : m_altNames) {
    ost << a << "&";
  }
  //  ost << ")";
  return ost.str();
}

//---------------------------------------------------------------------------                                           
const std::string 
DataObjectHandleBase::pythonRepr() const {
  return "DataObjectHandleBase(\"" + toString() + "\")";
}
 
//---------------------------------------------------------------------------                                           
void 
DataObjectHandleBase::fromString(const std::string& s) {
  Gaudi::Parsers::parse(*this,s).ignore();
}

//---------------------------------------------------------------------------

void 
DataObjectHandleBase::init() {

  if (m_init) return;

  if (m_owner == 0) return;

  setRead(false);
  setWritten(false);

  Algorithm* algorithm = dynamic_cast<Algorithm*>( m_owner );
  if (algorithm != 0) {
    // Fetch the event Data Service from the algorithm
    m_EDS = algorithm->evtSvc();
    m_MS = algorithm->msgSvc();
  } else {
    AlgTool* tool = dynamic_cast<AlgTool*>( owner() );
    if (tool != 0) {
      m_EDS = tool->evtSvc();
      m_MS = tool->msgSvc();
    } else {
      throw GaudiException( "owner is not an AlgTool or Algorithm" ,
			    "Invalid Cast", StatusCode::FAILURE) ;
    }
  }
  
  m_init = true;

}

//---------------------------------------------------------------------------

bool
DataObjectHandleBase::isValid() const {

  return  ( fullKey() != INVALID_DATAOBJID );

}

//---------------------------------------------------------------------------

std::ostream&
operator<< (std::ostream& str, const DataObjectHandleBase& d) {

  str << d.fullKey() << "  m: " << d.mode();
  if (d.owner() != 0) {
    str << "  o: " << d.owner()->name();
  }

  str << "  opt: " << d.isOptional();

  if (d.alternativeDataProductNames().size() != 0) {
    str << "  alt: [";
    for ( auto &n : d.alternativeDataProductNames()) {
      str << n << ",";
    }
    str << "]";
  }

  return str;
}
