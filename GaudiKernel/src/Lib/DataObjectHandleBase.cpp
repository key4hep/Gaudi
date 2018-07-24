#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/ServiceLocatorHelper.h"

#include <boost/tokenizer.hpp>
#include <ostream>
#include <sstream>
#include <string>

const DataObjID INVALID_DATAOBJID = DataObjID();

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase( DataObjectHandleBase&& other )
    : Gaudi::v1::DataHandle( other )
    , m_EDS( std::move( other.m_EDS ) )
    , m_MS( std::move( other.m_MS ) )
    , m_init( other.m_init )
    , m_optional( other.m_optional )
    , m_wasRead( other.m_wasRead )
    , m_wasWritten( other.m_wasWritten )
    , m_searchDone( other.m_searchDone )
{
  m_owner->declare( *this );
}

//---------------------------------------------------------------------------
DataObjectHandleBase& DataObjectHandleBase::operator=( const DataObjectHandleBase& other )
{
  // avoid modification of searchDone in other while we are copying
  std::lock_guard<std::mutex> guard( other.m_searchMutex );
  // FIXME: operator= should not change our owner, only our 'value'
  Gaudi::v1::DataHandle::operator=( other );
  m_EDS                          = other.m_EDS;
  m_MS                           = other.m_MS;
  m_init                         = other.m_init;
  m_optional                     = other.m_optional;
  m_wasRead                      = other.m_wasRead;
  m_wasWritten                   = other.m_wasWritten;
  m_searchDone                   = other.m_searchDone;
  return *this;
}

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase( const DataObjID& k, Gaudi::v1::DataHandle::Mode a, IDataHandleHolder* owner )
    : Gaudi::v1::DataHandle( k, a, owner )
{
  m_owner->declare( *this );
}

//---------------------------------------------------------------------------

DataObjectHandleBase::DataObjectHandleBase( const std::string& k, Gaudi::v1::DataHandle::Mode a, IDataHandleHolder* owner )
    : DataObjectHandleBase( DataObjID( k ), a, owner )
{
}

//---------------------------------------------------------------------------
DataObjectHandleBase::~DataObjectHandleBase() { owner()->renounce( *this ); }

//---------------------------------------------------------------------------
DataObject* DataObjectHandleBase::fetch() const
{
  DataObject* p = nullptr;
  if ( LIKELY( m_searchDone ) ) { // fast path: searchDone, objKey is in its final state
    m_EDS->retrieveObject( objKey(), p ).ignore();
    return p;
  }

  // slow path -- move into seperate function to improve code generation?

  // convenience towards users -- remind them to register
  // but as m_MS is not yet set, we cannot use a MsgStream...
  if ( !m_init ) {
    std::cerr << ( owner() ? owner()->name() : "<UNKNOWN>:" ) << "DataObjectHandle: uninitialized data handle"
              << std::endl;
  }

  // as m_searchDone is not true yet, objKey may change... so in this
  // branch we _first_ grab the mutex, to avoid objKey changing while we use it

  // take a lock to be sure we only execute this once at a time
  std::lock_guard<std::mutex> guard( m_searchMutex );

  StatusCode sc = m_EDS->retrieveObject( objKey(), p );
  if ( m_searchDone ) { // another thread has done the search while we were blocked
                        // on the mutex. As a result, nothing left to do but return...
    sc.ignore();
    return p;
  }

  if ( !sc.isSuccess() ) {
    auto tokens = boost::tokenizer<boost::char_separator<char>>{objKey(), boost::char_separator<char>{":"}};
    // let's try our alternatives (if any)
    auto alt = std::find_if( tokens.begin(), tokens.end(),
                             [&]( const std::string& n ) { return m_EDS->retrieveObject( n, p ).isSuccess(); } );
    if ( alt != tokens.end() ) {
      MsgStream log( m_MS, owner()->name() + ":DataObjectHandle" );
      log << MSG::DEBUG << ": could not find \"" << objKey() << "\" -- using alternative source: \"" << *alt
          << "\" instead" << endmsg;
      // found something -- set it as default; this is not atomic, but
      // at least in `fetch` there is no use of `objKey` that races with
      // this assignment... (but there may be others!)
      setKey( *alt );
    }
  }
  m_searchDone = true;
  return p;
}

//---------------------------------------------------------------------------

std::string DataObjectHandleBase::toString() const { return objKey(); }

//---------------------------------------------------------------------------
std::string DataObjectHandleBase::pythonRepr() const { return "DataObjectHandleBase(\"" + toString() + "\")"; }

//---------------------------------------------------------------------------
void DataObjectHandleBase::fromString( const std::string& s ) { Gaudi::Parsers::parse( *this, s ).ignore(); }

//---------------------------------------------------------------------------

bool DataObjectHandleBase::init()
{

  assert( !m_init );

  if ( !owner() ) return false;

  setRead( false );
  setWritten( false );

  Algorithm* algorithm = dynamic_cast<Algorithm*>( owner() );
  if ( algorithm ) {
    // Fetch the event Data Service from the algorithm
    m_EDS = algorithm->evtSvc();
    m_MS  = algorithm->msgSvc();
  } else {
    AlgTool* tool = dynamic_cast<AlgTool*>( owner() );
    if ( tool ) {
      m_EDS = tool->evtSvc();
      m_MS  = tool->msgSvc();
    } else {
      throw GaudiException( "owner is neither AlgTool nor Algorithm", "Invalid Cast", StatusCode::FAILURE );
    }
  }
  m_init = true;
  return true;
}

//---------------------------------------------------------------------------

bool DataObjectHandleBase::isValid() const { return fullKey() != INVALID_DATAOBJID; }

//---------------------------------------------------------------------------

std::ostream& operator<<( std::ostream& str, const DataObjectHandleBase& d )
{

  str << d.fullKey() << "  m: " << d.mode();
  if ( d.owner() ) str << "  o: " << d.owner()->name();
  str << "  opt: " << d.isOptional();
  return str;
}
