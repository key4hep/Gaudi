#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/SerializeSTL.h"

#include <sstream>
#include <string>
#include <ostream>
#include <boost/algorithm/string.hpp>


const DataObjID INVALID_DATAOBJID = DataObjID();

namespace {

    const char FIELD_SEP = '&';

    std::string head_alternates(const std::string& alternates) {
        return alternates.substr(0,alternates.find(FIELD_SEP));
    }
    std::string tail_alternates(const std::string& alternates) {
        auto i = alternates.find(FIELD_SEP);
        return i == std::string::npos ? std::string{} : alternates.substr(i+1);
    }
    std::vector<std::string> unpack(const std::string& alt) {
        std::vector<std::string> items;
        boost::split(items, alt, boost::is_any_of(std::string{FIELD_SEP}), boost::token_compress_on);
        return items;
    }
}

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase(const DataObjID & k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner,
                      std::vector<std::string> alternates):
  Gaudi::DataHandle(k,a,owner), m_altNames( std::move(alternates) ) {}

//---------------------------------------------------------------------------

DataObjectHandleBase::DataObjectHandleBase(const std::string & k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner):
  DataObjectHandleBase(DataObjID(head_alternates(k)), a, owner, unpack(tail_alternates(k))){}

//---------------------------------------------------------------------------
DataObject* DataObjectHandleBase::fetch() {

  DataObject* p = nullptr;
  if (UNLIKELY(!m_init)) init();

  StatusCode sc = m_EDS->retrieveObject(objKey(), p);
  if ( UNLIKELY( sc.isFailure() ) &&
       UNLIKELY( !alternativeDataProductNames().empty()) ) {
      // let's try our alternatives
      for (const auto& alt : alternativeDataProductNames() ) {
          sc = m_EDS->retrieveObject(alt , p);
          if (sc.isSuccess()) {
	          MsgStream log(m_MS,"DataObjectHandle");
              log << MSG::WARNING << "could not find " << objKey()
                  << " -- but found alternative: " << alt
                  << " making this the new default, and removing searchpath "
                  << endmsg;
              // found something -- set it as default,
              // and zero out the alternatives
              setKey(alt);
              setAlternativeDataProductNames( { } );
              break;
          }
      }
  }
  return p;
}

//---------------------------------------------------------------------------

std::string
DataObjectHandleBase::toString() const {
  std::ostringstream ost;
  ost << m_key << "|" << mode() << "|" << isOptional() << "|";
  GaudiUtils::details::ostream_joiner( ost, alternativeDataProductNames(),"&" );
  return ost.str();
}

//---------------------------------------------------------------------------
std::string
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

  if (!m_owner) return;

  setRead(false);
  setWritten(false);

  Algorithm* algorithm = dynamic_cast<Algorithm*>( m_owner );
  if (algorithm) {
    // Fetch the event Data Service from the algorithm
    m_EDS = algorithm->evtSvc();
    m_MS = algorithm->msgSvc();
  } else {
    AlgTool* tool = dynamic_cast<AlgTool*>( owner() );
    if (!tool) {
      throw GaudiException( "owner is not an AlgTool or Algorithm" ,
			    "Invalid Cast", StatusCode::FAILURE) ;
    }
  }

  m_init = true;

}

//---------------------------------------------------------------------------

bool
DataObjectHandleBase::isValid() const {

  return fullKey() != INVALID_DATAOBJID;

}

//---------------------------------------------------------------------------

std::ostream&
operator<< (std::ostream& str, const DataObjectHandleBase& d) {

  str << d.fullKey() << "  m: " << d.mode();
  if (d.owner() != 0) str << "  o: " << d.owner()->name();

  str << "  opt: " << d.isOptional();

  if (!d.alternativeDataProductNames().empty()) {
    GaudiUtils::details::ostream_joiner(
     str << "  alt: [", d.alternativeDataProductNames(), "," ) << "]";
  }

  return str;
}
