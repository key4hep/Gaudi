#include "GaudiKernel/MinimalDataObjectHandle.h"

unsigned int MinimalDataObjectHandle::m_tmp_dpi=0;
//---------------------------------------------------------------------------

MinimalDataObjectHandle::MinimalDataObjectHandle(const std::string& productName,
                                                 IAlgorithm* fatherAlg,
                                                 AccessType accessType,
                                                 bool isOptional):
                                                 m_dataProductName(productName),
                                                 m_fatherAlg(fatherAlg),
                                                 m_isOptional(isOptional),
                                                 m_dataProductIndex(m_tmp_dpi++),
                                                 m_accessType(accessType),
                                                 m_wasRead(false),
                                                 m_wasWritten(false){
  // Register algo on the Sched Svc                                                 
  }
  
//---------------------------------------------------------------------------
  
StatusCode MinimalDataObjectHandle::initialize(){
  setRead(false);
  setWritten(false);
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
  return m_isOptional;
}

//---------------------------------------------------------------------------

unsigned int MinimalDataObjectHandle::dataProductIndex() const {
  return m_dataProductIndex;
}

//---------------------------------------------------------------------------

const std::string& MinimalDataObjectHandle::dataProductName() const {
  return m_dataProductName;
}

//---------------------------------------------------------------------------

auto MinimalDataObjectHandle::accessType() const -> AccessType {
  return m_accessType;
}

//---------------------------------------------------------------------------

bool MinimalDataObjectHandle::wasRead() const {
  return m_wasRead;
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