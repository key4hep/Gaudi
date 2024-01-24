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
    , m_optional( other.m_optional ) {
  m_owner->declare( *this );
}

//---------------------------------------------------------------------------
DataObjectHandleBase& DataObjectHandleBase::operator=( const DataObjectHandleBase& other ) {
  // FIXME: operator= should not change our owner, only our 'value'
  Gaudi::DataHandle::operator=( other );
  m_EDS      = other.m_EDS;
  m_MS       = other.m_MS;
  m_init     = other.m_init;
  m_optional = other.m_optional;
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
  // convenience towards users -- remind them to register
  // but as m_MS is not yet set, we cannot use a MsgStream...
  if ( !m_init ) {
    std::cerr << ( owner() ? owner()->name() : "<UNKNOWN>:" ) << "DataObjectHandle: uninitialized data handle"
              << std::endl;
  }
  DataObject* p = nullptr;
  m_EDS->retrieveObject( objKey(), p ).ignore();
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
