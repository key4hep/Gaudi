/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Experiment specific include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IInspector.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/StreamBuffer.h"
#include <memory>
#include <vector>

static std::string _sDataObjectCppNotRegistered( "NotRegistered" );

/// Standard Constructor
DataObject::DataObject() : m_pLinkMgr{ LinkManager::newInstance() } {}

/// Copy Constructor
DataObject::DataObject( const DataObject& rhs )
    : m_version{ rhs.m_version }, m_pLinkMgr{ LinkManager::newInstance() } {}

/// Assignment Operator
DataObject& DataObject::operator=( const DataObject& rhs ) {
  m_version = rhs.m_version;
  m_pLinkMgr.reset( LinkManager::newInstance() );
  return *this;
}

/// Move Constructor
DataObject::DataObject( DataObject&& rhs )
    : m_version{ std::move( rhs.m_version ) }, m_pLinkMgr{ std::move( rhs.m_pLinkMgr ) } {}

/// Assignment Operator
DataObject& DataObject::operator=( DataObject&& rhs ) {
  m_version  = std::move( rhs.m_version );
  m_pLinkMgr = std::move( rhs.m_pLinkMgr );
  return *this;
}

/// Standard Destructor
DataObject::~DataObject() {
  // Issue a warning if the object is being deleted and the reference
  // count is non-zero.
  if ( m_refCount > 0 ) {
    // Insert warning here
  }
}

/// Decrease reference count
unsigned long DataObject::release() {
  unsigned long cnt = --m_refCount;
  if ( 0 == cnt ) delete this;
  return cnt;
}

/// Add reference to object
unsigned long DataObject::addRef() { return ++m_refCount; }

/// Retrieve Pointer to class definition structure
const CLID& DataObject::clID() const { return CLID_DataObject; }

/// Retrieve Pointer to class definition structure
const CLID& DataObject::classID() { return CLID_DataObject; }

/// Retrieve DataObject name. It is the name when included in the store.
const std::string& DataObject::name() const { return m_pRegistry ? m_pRegistry->name() : _sDataObjectCppNotRegistered; }

/// Provide empty placeholder for internal object reconfiguration callback
StatusCode DataObject::update() { return StatusCode::SUCCESS; }

static DataObject*  s_objPtr  = nullptr;
static DataObject** s_currObj = &s_objPtr;

static std::vector<DataObject**>& objectStack() {
  static std::unique_ptr<std::vector<DataObject**>> s_current{ new std::vector<DataObject**>() };
  return *s_current;
}

DataObject* Gaudi::getCurrentDataObject() { return *s_currObj; }

void Gaudi::pushCurrentDataObject( DataObject** pobjAddr ) {
  static std::vector<DataObject**>& c = objectStack();
  c.push_back( pobjAddr );
  s_currObj = pobjAddr ? pobjAddr : &s_objPtr;
}

void Gaudi::popCurrentDataObject() {
  static std::vector<DataObject**>& c = objectStack();
  if ( !c.empty() ) {
    s_currObj = c.back();
    c.pop_back();
  } else {
    s_currObj = &s_objPtr;
  }
}
