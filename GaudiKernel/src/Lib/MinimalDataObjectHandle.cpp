#include "GaudiKernel/MinimalDataObjectHandle.h"
#include "GaudiKernel/DataObjectDescriptor.h"
#include "GaudiKernel/ContextSpecificPtr.h"

//---------------------------------------------------------------------------

MinimalDataObjectHandle::MinimalDataObjectHandle():
  m_descriptor(new DataObjectDescriptor(DataObjectDescriptor::NULL_,
                                        DataObjectDescriptor::NULL_)),

  m_dataProductIndex(-1),
  m_wasRead(false),
  m_wasWritten(false),
  m_initialized(false){
}

MinimalDataObjectHandle::MinimalDataObjectHandle(DataObjectDescriptor & descriptor):
  m_descriptor(&descriptor),
  m_dataProductIndex(updateDataProductIndex()),
  m_wasRead(false),
  m_wasWritten(false),
  m_initialized(false){
}

MinimalDataObjectHandle::~MinimalDataObjectHandle(){
  delete m_descriptor;
}

//---------------------------------------------------------------------------

StatusCode MinimalDataObjectHandle::initialize(){
  setRead(false);
  setWritten(false);

  m_initialized = true;
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode MinimalDataObjectHandle::reinitialize(){
  setRead(false);
  setWritten(false);
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode MinimalDataObjectHandle::finalize(){
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

bool MinimalDataObjectHandle::isOptional() const {
  return m_descriptor->optional();
}

void MinimalDataObjectHandle::setOptional(bool optional){
	m_descriptor->setOptional(optional);
}

//---------------------------------------------------------------------------

size_t MinimalDataObjectHandle::dataProductIndex() const {
  return m_dataProductIndex;
}

//---------------------------------------------------------------------------

const std::string& MinimalDataObjectHandle::dataProductName() const {
  return m_descriptor->address();
}

const std::vector<std::string> & MinimalDataObjectHandle::alternativeDataProductNames() const{
  return m_descriptor->alternativeAddresses();
}

StatusCode MinimalDataObjectHandle::setDataProductName(const std::string & address){
  //only allowed if not initialized yet
  if(m_initialized)
    return StatusCode::FAILURE;

  m_descriptor->setAddress(address);

  updateDataProductIndex();

  return StatusCode::SUCCESS;
}

StatusCode MinimalDataObjectHandle::setAlternativeDataProductNames(const std::vector<std::string> & alternativeAddresses){
  //only allowed if not initialized yet
  if(m_initialized)
    return StatusCode::FAILURE;

  m_descriptor->setAltAddresses(alternativeAddresses);

  updateDataProductIndex();

  return StatusCode::SUCCESS;
}

StatusCode MinimalDataObjectHandle::setDataProductNames(const std::vector<std::string> & addresses){
  //only allowed if not initialized yet
  if(m_initialized)
    return StatusCode::FAILURE;

  m_descriptor->setAddresses(addresses);

  updateDataProductIndex();

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

auto MinimalDataObjectHandle::accessType() const -> AccessType {
  return m_descriptor->accessType();
}

//---------------------------------------------------------------------------

bool MinimalDataObjectHandle::wasRead() const {
  return m_wasRead;
}

//---------------------------------------------------------------------------

bool MinimalDataObjectHandle::isValid() const {
  return m_descriptor->valid();
}

//---------------------------------------------------------------------------

bool MinimalDataObjectHandle::wasWritten() const {
  return m_wasWritten;
}

//---------------------------------------------------------------------------

void MinimalDataObjectHandle::setRead(bool wasRead){m_wasRead=wasRead;}

//---------------------------------------------------------------------------

void MinimalDataObjectHandle::setWritten(bool wasWritten){m_wasWritten=wasWritten;}

//---------------------------------------------------------------------------

DataObjectDescriptor * MinimalDataObjectHandle::descriptor(){return m_descriptor;}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
std::unordered_map<std::string, size_t> MinimalDataObjectHandle::m_dataProductIndexMap;

size_t MinimalDataObjectHandle::updateDataProductIndex (){
  if(dataProductName() == DataObjectDescriptor::NULL_)
    m_dataProductIndex = -1;
  else {
    auto idx = m_dataProductIndexMap.find(dataProductName());
    if(idx != m_dataProductIndexMap.end())
      m_dataProductIndex = idx->second;
    else{
      auto res = m_dataProductIndexMap.emplace(dataProductName(), m_dataProductIndexMap.size());
      m_dataProductIndex = res.first->second;
    }
  }

  return m_dataProductIndex;
}

//---------------------------------------------------------------------------

//methods dealing with locking mechanism

std::map<size_t, std::map<size_t, tbb::spin_mutex> > MinimalDataObjectHandle::m_locks;

void MinimalDataObjectHandle::lock(){
  m_locks[Gaudi::Hive::currentContextId()][dataProductIndex()].lock();
}

void MinimalDataObjectHandle::unlock(){
  m_locks[Gaudi::Hive::currentContextId()][dataProductIndex()].unlock();

  if(m_locks[Gaudi::Hive::currentContextId()].size() > CLEANUP_THRESHOLD){
    for(auto & lock : m_locks[Gaudi::Hive::currentContextId()]){
      //non-blocking call to try_lock
      //if we can get the lock, then it wasn't set before -- delete it
      if(lock.second.try_lock()){
        m_locks[Gaudi::Hive::currentContextId()].erase(lock.first);
      }
    }

  }
}
