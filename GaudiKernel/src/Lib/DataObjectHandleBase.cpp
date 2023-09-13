/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include <Gaudi/Algorithm.h>

#include <boost/tokenizer.hpp>
#include <ostream>
#include <sstream>
#include <string>

const DataObjID INVALID_DATAOBJID = DataObjID();

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase( DataObjectHandleBase&& other )
    : Gaudi::DataHandle( other )
    , m_EDS( std::move( other.m_EDS ) )
    , m_MS( std::move( other.m_MS ) )
    , m_init( other.m_init )
    , m_optional( other.m_optional )
    , m_searchDone( other.m_searchDone.load() ) {
  m_owner->declare( *this );
}

//---------------------------------------------------------------------------
DataObjectHandleBase& DataObjectHandleBase::operator=( const DataObjectHandleBase& other ) {
  // avoid modification of searchDone in other while we are copying
  auto guard = std::scoped_lock{ other.m_searchMutex };
  // FIXME: operator= should not change our owner, only our 'value'
  Gaudi::DataHandle::operator=( other );
  m_EDS        = other.m_EDS;
  m_MS         = other.m_MS;
  m_init       = other.m_init;
  m_optional   = other.m_optional;
  m_searchDone = other.m_searchDone.load();
  return *this;
}

//---------------------------------------------------------------------------
DataObjectHandleBase::DataObjectHandleBase( DataObjID k, Gaudi::DataHandle::Mode a, IDataHandleHolder* owner )
    : Gaudi::DataHandle( std::move( k ), a, owner ) {
  m_owner->declare( *this );
}

//---------------------------------------------------------------------------

DataObjectHandleBase::DataObjectHandleBase( std::string k, Gaudi::DataHandle::Mode a, IDataHandleHolder* owner )
    : DataObjectHandleBase( DataObjID{ std::move( k ) }, a, owner ) {}

//---------------------------------------------------------------------------
DataObjectHandleBase::~DataObjectHandleBase() { owner()->renounce( *this ); }

//---------------------------------------------------------------------------
DataObject* DataObjectHandleBase::fetch() const {
  DataObject* p = nullptr;
  if ( m_searchDone ) { // fast path: searchDone, objKey is in its final state
    m_EDS->retrieveObject( objKey(), p ).ignore();
    return p;
  }

  // slow path -- move into seperate function to improve code generation?

  // as m_searchDone is not true yet, objKey may change... so in this
  // branch we _first_ grab the mutex, to avoid objKey changing while we use it

  // take a lock to be sure we only execute this one at a time
  auto guard = std::scoped_lock{ m_searchMutex };

  // convenience towards users -- remind them to register
  // but as m_MS is not yet set, we cannot use a MsgStream...
  if ( !m_init ) {
    std::cerr << ( owner() ? owner()->name() : "<UNKNOWN>:" ) << "DataObjectHandle: uninitialized data handle"
              << std::endl;
  }

  // note: if no seperator is found, this returns a range of one with the  input as the single token
  // so if the search was done while we were waiting on the mutex, this (also) does the right thing
  auto tokens = boost::tokenizer<boost::char_separator<char>>{ objKey(), boost::char_separator<char>{ ":" } };
  // let's try our alternatives (if any)
  auto alt = std::find_if( tokens.begin(), tokens.end(),
                           [&]( const std::string& n ) { return m_EDS->retrieveObject( n, p ).isSuccess(); } );
  if ( alt != tokens.end() && *alt != objKey() ) {
    MsgStream log( m_MS, owner()->name() + ":DataObjectHandle" );
    log << MSG::DEBUG << ": could not find \"" << objKey() << "\" -- using alternative source: \"" << *alt
        << "\" instead" << endmsg;
    // found something -- set it as default; this is not atomic, but
    // at least in `fetch` there is no use of `objKey` that races with
    // this assignment... (but there may be others!)
    setKey( *alt ); // if there was one token, this is an unnecessary write... but who cares...
  }

  bool expected = false;                                  // but could be true (already)...
  m_searchDone.compare_exchange_strong( expected, true ); // if not yet true, set it to true...
  return p;
}

//---------------------------------------------------------------------------

bool DataObjectHandleBase::init() {
  if ( m_init ) return true; // initialization already done

  if ( !owner() ) return false;

  Gaudi::Algorithm* algorithm = dynamic_cast<Gaudi::Algorithm*>( owner() );
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
      throw GaudiException( "owner is neither AlgTool nor Gaudi::Algorithm", "Invalid Cast", StatusCode::FAILURE );
    }
  }
  m_init = true;
  return true;
}

//---------------------------------------------------------------------------

bool DataObjectHandleBase::isValid() const { return fullKey() != INVALID_DATAOBJID; }

//---------------------------------------------------------------------------

std::ostream& operator<<( std::ostream& str, const DataObjectHandleBase& d ) {

  str << d.fullKey() << "  m: " << d.mode();
  if ( d.owner() ) str << "  o: " << d.owner()->name();
  str << "  opt: " << d.isOptional();
  return str;
}
