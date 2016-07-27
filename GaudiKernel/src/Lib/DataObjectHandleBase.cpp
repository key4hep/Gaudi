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
        if (!alt.empty()) boost::split(items, alt, boost::is_any_of(std::string{FIELD_SEP}), boost::token_compress_on);
        return items;
    }
}

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase(DataObjectHandleBase&& other) {
  *this = other;
}

//---------------------------------------------------------------------------
DataObjectHandleBase& DataObjectHandleBase::operator=(const DataObjectHandleBase& other) {
  Gaudi::DataHandle::operator=(other);
  // avoid modification of searchDone and altNames in other while we are copying
  std::lock_guard<std::mutex> guard(other.m_searchMutex);
  m_EDS = other.m_EDS;
  m_MS = other.m_MS;
  m_init = other.m_init;
  m_goodType = other.m_goodType;
  m_optional = other.m_optional;
  m_wasRead = other.m_wasRead;
  m_wasWritten = other.m_wasWritten;
  m_searchDone = other.m_searchDone;
  m_altNames = other.m_altNames;
  return *this;
}

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase(const DataObjID& k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner,
                      std::vector<std::string> alternates):
  Gaudi::DataHandle(k,a,owner), m_altNames( std::move(alternates) ) {}

//---------------------------------------------------------------------------

DataObjectHandleBase::DataObjectHandleBase(const std::string& k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner):
  DataObjectHandleBase(DataObjID(head_alternates(k)), a, owner, unpack(tail_alternates(k))){}

//---------------------------------------------------------------------------
DataObject* DataObjectHandleBase::fetch() const {

  DataObject* p = nullptr;
  if (LIKELY(m_searchDone)) { // fast path: searchDone, objKey is in its final state
      m_EDS->retrieveObject(objKey(), p).ignore();
      return p;
  }

  // slow path -- move into seperate function to improve code generation?

  // convenience towards users -- remind them to register
  // but as m_MS is not yet set, we cannot use a MsgStream...
  if (!m_init) {
    std::cerr << ( m_owner? m_owner->name() : "<UNKNOWN>:")
        << "DataObjectHandle: uninitialized data handle --  "
        << "you probably forgot to declare it with either "
        << "declareProperty, declareInput or declareOutput" << std::endl;;
  }

  // as m_searchDone is not true yet, objKey may change... so in this
  // branch we _first_ grab the mutex, to avoid objKey changing while we use it

  // take a lock to be sure we only execute this once at a time
  std::lock_guard<std::mutex> guard(m_searchMutex);

  StatusCode sc = m_EDS->retrieveObject(objKey(), p);
  if (m_searchDone) { // another thread has done the search while we were blocked
                      // on the mutex. As a result, nothing left to do but return...
      sc.ignore();
      return p;
  }

  if (!sc.isSuccess()) {
      // let's try our alternatives (if any)
      auto alt = std::find_if( alternativeDataProductNames().begin(),
                               alternativeDataProductNames().end(),
                               [&](const std::string& n) {
                                 return m_EDS->retrieveObject(n,p).isSuccess();
                               } );
      if (alt!=alternativeDataProductNames().end()) {
        MsgStream log(m_MS,m_owner->name() + ":DataObjectHandle");
        log << MSG::INFO <<  ": could not find \"" << objKey()
            << "\" -- using alternative source: \"" << *alt << "\" instead"
            << endmsg;
        // found something -- set it as default; this is not atomic, but
        // at least in `fetch` there is no use of `objKey` that races with
        // this assignment... (but there may be others!)
        setKey(*alt);
        // and zero out the alternatives
        setAlternativeDataProductNames( { } );
      }
  }
  m_searchDone = true;
  return p;
}

//---------------------------------------------------------------------------

std::string
DataObjectHandleBase::toString() const {
  std::ostringstream ost;
  GaudiUtils::details::ostream_joiner( ost << m_key
                                           << "|" << mode()
                                           << "|" << isOptional()
                                           << "|",
                                       alternativeDataProductNames(),"&" );
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

  assert(!m_init);

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
    if (tool) {
        m_EDS = tool->evtSvc();
        m_MS = tool->msgSvc();
    } else {
      throw GaudiException( "owner is neither AlgTool nor Algorithm" ,
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
  if (d.owner()) str << "  o: " << d.owner()->name();

  str << "  opt: " << d.isOptional();

  GaudiUtils::details::ostream_joiner(
    str << "  alt: [", d.alternativeDataProductNames(), "," ,
    [](std::ostream& os,const std::string& s) { os << "\"" << s << "\"" ;}
  ) << "]";

  return str;
}
