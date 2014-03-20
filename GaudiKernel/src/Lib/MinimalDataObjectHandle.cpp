#include "GaudiKernel/MinimalDataObjectHandle.h"
#include "GaudiKernel/DataObjectDescriptor.h"

unsigned int MinimalDataObjectHandle::m_tmp_dpi=0;

//---------------------------------------------------------------------------

MinimalDataObjectHandle::MinimalDataObjectHandle()
	:    m_descriptor(new DataObjectDescriptor(DataObjectDescriptor::NULL_,
			DataObjectDescriptor::NULL_)),

		 m_dataProductIndex(m_tmp_dpi++),
	     m_wasRead(false),
	     m_wasWritten(false),
	     m_initialized(false){

  }

MinimalDataObjectHandle::MinimalDataObjectHandle(DataObjectDescriptor & descriptor)
	:    m_descriptor(&descriptor),
	     m_dataProductIndex(m_tmp_dpi++),
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

//---------------------------------------------------------------------------

unsigned int MinimalDataObjectHandle::dataProductIndex() const {
  return m_dataProductIndex;
}

//---------------------------------------------------------------------------

const std::string& MinimalDataObjectHandle::dataProductName() const {
  return m_descriptor->address();
}

StatusCode MinimalDataObjectHandle::setDataProductName(const std::string & address){

	//only allowed if not initialized yet
	if(m_initialized)
		return StatusCode::FAILURE;

	m_descriptor->setAddress(address);

	return StatusCode::SUCCESS;
}

StatusCode MinimalDataObjectHandle::setDataProductNames(const std::vector<std::string> & addresses){

	//only allowed if not initialized yet
	if(m_initialized)
		return StatusCode::FAILURE;

	m_descriptor->setAddresses(addresses);

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
