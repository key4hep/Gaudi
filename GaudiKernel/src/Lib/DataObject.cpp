// $Id: DataObject.cpp,v 1.11 2008/11/13 15:30:27 marcocle Exp $

// Experiment specific include files
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IInspector.h"
#include "GaudiKernel/IRegistry.h"
#include <vector>
#include <memory>

static std::string _sDataObjectCppNotRegistered("NotRegistered");

/// Standard Constructor
DataObject::DataObject()
 : m_refCount(0),
   m_version(0),
   m_pRegistry(0)
{
  m_pLinkMgr = LinkManager::newInstance();
}

/// Standard Constructor
DataObject::DataObject(const DataObject&)
 : m_refCount(0),
   m_version(0),
   m_pRegistry(0)
{
  m_pLinkMgr = LinkManager::newInstance();
}

/// Standard Destructor
DataObject::~DataObject()   {
  // Issue a warning if the object is being deleted and the reference
  // count is non-zero.
  if ( m_refCount > 0 ) {
    // Insert warning here
  }
  if ( m_pLinkMgr ) delete m_pLinkMgr;
  m_pLinkMgr = 0;
}

/// Decrease reference count
unsigned long DataObject::release()  {
  unsigned long cnt = --m_refCount;
  if ( 0 == m_refCount )   {
    delete this;
  }
  return cnt;
}

/// Add reference to object
unsigned long DataObject::addRef()  {
  return ++m_refCount;
}

/// Retrieve Pointer to class definition structure
const CLID& DataObject::clID() const   {
  return CLID_DataObject;
}

/// Retrieve Pointer to class definition structure
const CLID& DataObject::classID()    {
  return CLID_DataObject;
}

/// Retrieve DataObject name. It is the name when included in the store.
const std::string& DataObject::name() const {
  if( m_pRegistry != 0) {
    return m_pRegistry->name();
  }
  else {
    return _sDataObjectCppNotRegistered;
  }
}

/// ISerialize implementation: Serialize the object for reading
StreamBuffer& DataObject::serialize(StreamBuffer& s)   {
  return s >> m_version;
}

/// ISerialize implementation: Serialize the object for writing
StreamBuffer& DataObject::serialize(StreamBuffer& s)  const    {
  return s << m_version;
}


static DataObject*       s_objPtr = 0;
static DataObject**      s_currObj = &s_objPtr;

static std::vector<DataObject**>& objectStack() {
  static std::auto_ptr<std::vector<DataObject**> > s_current;
  if ( 0 == s_current.get() )  {
    s_current = std::auto_ptr<std::vector<DataObject**> >(new std::vector<DataObject**>());
  }
  return *(s_current.get());
}

DataObject* Gaudi::getCurrentDataObject() {
  return *s_currObj;
}

void Gaudi::pushCurrentDataObject(DataObject** pobjAddr) {
  static std::vector<DataObject**>& c = objectStack();
  c.push_back(pobjAddr);
  s_currObj = pobjAddr ? pobjAddr : &s_objPtr;
}


void Gaudi::popCurrentDataObject() {
  static std::vector<DataObject**>& c = objectStack();
  switch(c.size())  {
  case 0:
    s_currObj = c.back();
    c.pop_back();
    break;
  default:
    s_currObj = &s_objPtr;
    break;
  }
}
