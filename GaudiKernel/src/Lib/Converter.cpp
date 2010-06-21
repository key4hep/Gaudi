// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/Converter.cpp,v 1.17 2007/12/12 16:02:32 marcocle Exp $

// Include Files
#include "GaudiKernel/Converter.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiKernel/ConversionSvc.h"

/// Retrieve the class type of objects the converter produces.
const CLID& Converter::objType() const    {
  return m_classType;
}

/// Retrieve the class type of the data store the converter uses.
long Converter::i_repSvcType() const   {
  return m_storageType;
}

/// Create the transient representation of an object.
StatusCode Converter::createObj(IOpaqueAddress*, DataObject*&)   {
  return StatusCode::SUCCESS;
}


/// Resolve the references of the created transient object.
StatusCode Converter::fillObjRefs(IOpaqueAddress*, DataObject*)    {
  return StatusCode::SUCCESS;
}

/// Update the transient object from the other representation.
StatusCode Converter::updateObj(IOpaqueAddress*, DataObject*)   {
  return StatusCode::SUCCESS;
}

/// Update the references of an updated transient object.
StatusCode Converter::updateObjRefs(IOpaqueAddress*, DataObject*)  {
  return StatusCode::SUCCESS;
}

/// Convert the transient object to the requested representation.
StatusCode Converter::createRep(DataObject*, IOpaqueAddress*&)  {
  return StatusCode::SUCCESS;
}

/// Resolve the references of the converted object.
StatusCode Converter::fillRepRefs(IOpaqueAddress*, DataObject*)  {
  return StatusCode::SUCCESS;
}

/// Update the converted representation of a transient object.
StatusCode Converter::updateRep(IOpaqueAddress*, DataObject*)  {
  return StatusCode::SUCCESS;
}

/// Update the references of an already converted object.
StatusCode Converter::updateRepRefs(IOpaqueAddress*, DataObject*)    {
  return StatusCode::SUCCESS;
}

/// Initialize the converter
StatusCode Converter::initialize()    {
  // Get a reference to the Message Service
  if ( !msgSvc().isValid() )   {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// Finalize the converter
StatusCode Converter::finalize() {
  // release services
  m_messageSvc = 0;
  m_dataManager = 0;
  m_dataProvider = 0;
  m_conversionSvc = 0;
  m_addressCreator = 0;
  return StatusCode::SUCCESS;
}

/// Set data provider service
StatusCode Converter::setDataProvider(IDataProviderSvc* svc) {
  m_dataProvider = svc;
  m_dataManager = svc;
  return StatusCode::SUCCESS;
}

/// Get data provider service
SmartIF<IDataProviderSvc>& Converter::dataProvider()  const    {
  return m_dataProvider;
}

/// Get data manager service
SmartIF<IDataManagerSvc>& Converter::dataManager()  const    {
  return m_dataManager;
}

/// Set conversion service the converter is connected to
StatusCode Converter::setConversionSvc(IConversionSvc* svc)   {
  m_conversionSvc = svc;
  return StatusCode::SUCCESS;
}

/// Get data conversion service the converter is connected to
SmartIF<IConversionSvc>& Converter::conversionSvc()  const    {
  return m_conversionSvc;
}

/// Set address creator facility
StatusCode Converter::setAddressCreator(IAddressCreator* creator)   {
  m_addressCreator = creator;
  return StatusCode::SUCCESS;
}

/// Access the transient store
SmartIF<IAddressCreator>& Converter::addressCreator()  const   {
  return m_addressCreator;
}

///--- Retrieve pointer to service locator
SmartIF<ISvcLocator>& Converter::serviceLocator()  const     {
  return m_svcLocator;
}

///--- Retrieve pointer to message service
SmartIF<IMessageSvc>& Converter::msgSvc()  const   {
  if ( !m_messageSvc.isValid() ) {
    m_messageSvc = serviceLocator();
    if( !m_messageSvc.isValid() ) {
      throw GaudiException("Service [MessageSvc] not found", "Converter", StatusCode::FAILURE);
    }
  }
  return m_messageSvc;
}

// Obsoleted name, kept due to the backwards compatibility
SmartIF<IMessageSvc>& Converter::messageService()  const   {
  return msgSvc();
}

/// Standard Constructor
Converter::Converter(long storage_type, const CLID& class_type, ISvcLocator* svc) :
  m_storageType(storage_type),
  m_classType(class_type),
  m_svcLocator(svc)
{
}

/// Standard Destructor
Converter::~Converter() {
}

StatusCode
Converter::service_i(const std::string& svcName, bool createIf,
		     const InterfaceID& iid, void** ppSvc) const {
  // Check for name of conversion service
  SmartIF<INamedInterface> cnvsvc(conversionSvc());
  if (cnvsvc.isValid()) {
    const ServiceLocatorHelper helper(*serviceLocator(), "Converter", cnvsvc->name());
    return helper.getService(svcName, createIf, iid, ppSvc);
  }
  return StatusCode::FAILURE;
}

StatusCode
Converter::service_i(const std::string& svcType, const std::string& svcName,
		     const InterfaceID& iid, void** ppSvc) const {
  // Check for name of conversion service
  SmartIF<INamedInterface> cnvsvc(conversionSvc());
  if (cnvsvc.isValid()) {
    const ServiceLocatorHelper helper(*serviceLocator(), "Converter", cnvsvc->name());
    return helper.createService(svcType, svcName, iid, ppSvc);
  }
  return StatusCode::FAILURE;
}

SmartIF<IService> Converter::service(const std::string& name, const bool createIf) const {
  SmartIF<INamedInterface> cnvsvc(conversionSvc());
  SmartIF<IService> svc;
  if (cnvsvc.isValid()) {
    const ServiceLocatorHelper helper(*serviceLocator(), "Converter", cnvsvc->name());
    svc = helper.service(name, false, createIf);
  }
  return svc;
}
