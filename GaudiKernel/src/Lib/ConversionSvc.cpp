// $Id: ConversionSvc.cpp,v 1.22 2007/09/24 08:58:21 hmd Exp $
#define  GAUDIKERNEL_CONVERSIONSVC_CPP

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/CnvFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ConversionSvc.h"

using ROOT::Reflex::PluginService;

enum CnvSvcAction   {
  CREATE_OBJ,
  FILL_OBJ_REFS,
  UPDATE_OBJ,
  UPDATE_OBJ_REFS,
  CREATE_REP,
  FILL_REP_REFS,
  UPDATE_REP,
  UPDATE_REP_REFS
};

StatusCode ConversionSvc::makeCall(int typ,
                                   bool ignore_add,
                                   bool ignore_obj,
                                   bool update,
                                   IOpaqueAddress*& pAddress,
                                   DataObject*& pObject)      {
  if ( 0 != pAddress || ignore_add )    {
    if ( 0 != pObject  || ignore_obj )    {
      const CLID&  obj_class =
        (0 != pObject && !ignore_obj) ? pObject->clID()
        : (0 != pAddress && !ignore_add)
        ? pAddress->clID()
        : CLID_NULL;
      IConverter*  cnv  = converter(obj_class);
      if ( !cnv && pObject ) {
        //Give it a try to autoload the class (dictionary) for which the converter is needed
        loadConverter( pObject);
        cnv  = converter(obj_class);
      }

      StatusCode status(StatusCode::FAILURE,true);
      if ( 0 != cnv )   {
        switch(typ)   {
        case CREATE_OBJ:
          pObject = 0;
          status = cnv->createObj(pAddress, pObject);
          break;
        case FILL_OBJ_REFS:
          status = cnv->fillObjRefs(pAddress, pObject);
          break;
        case UPDATE_OBJ:
          status = cnv->updateObj(pAddress, pObject);
          break;
        case UPDATE_OBJ_REFS:
          status = cnv->updateObjRefs(pAddress, pObject);
          break;
        case CREATE_REP:
          pAddress = 0;
          status = cnv->createRep(pObject, pAddress);
          break;
        case FILL_REP_REFS:
          status = cnv->fillRepRefs(pAddress, pObject);
          break;
        case UPDATE_REP:
          status = cnv->updateRep(pAddress, pObject);
          break;
        case UPDATE_REP_REFS:
          status = cnv->updateRepRefs(pAddress, pObject);
          break;
        default:
          status = StatusCode::FAILURE;
          break;
        }
        if ( status.isSuccess() && update )   {
          status = updateServiceState(pAddress);
        }
        return status;
      }
      status.ignore();
      MsgStream log(msgSvc(), name());
      log << MSG::INFO << "No converter for object ";
      if ( pObject != 0 )   {
        log << System::typeinfoName(typeid(*pObject));
      }
      log << "  CLID= " << obj_class << endmsg;
      return NO_CONVERTER;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ADDRESS;
}

void ConversionSvc::loadConverter(DataObject*) {

}

StatusCode ConversionSvc::updateServiceState(IOpaqueAddress* /*pAddress */)   {
  return StatusCode::SUCCESS;
}

/// Create the transient representation of an object.
StatusCode ConversionSvc::createObj(IOpaqueAddress* pAddress, DataObject*& refpObj)   {
  return makeCall(CREATE_OBJ, false, true, false, pAddress, refpObj);
}

/// Resolve the references of the created transient object.
StatusCode ConversionSvc::fillObjRefs(IOpaqueAddress* pAddress, DataObject* pObj)    {
  return makeCall(FILL_OBJ_REFS, false, true, true, pAddress, pObj);
}

/// Update the transient object from the other representation.
StatusCode ConversionSvc::updateObj(IOpaqueAddress* pAddress, DataObject* pObj)   {
  return makeCall(UPDATE_OBJ, false, true, false, pAddress, pObj);
}

/// Update the references of an updated transient object.
StatusCode ConversionSvc::updateObjRefs(IOpaqueAddress* pAddress, DataObject* pObj)  {
  return makeCall(UPDATE_OBJ_REFS, false, true, true, pAddress, pObj);
}

/// Convert the transient object to the requested representation.
StatusCode ConversionSvc::createRep(DataObject* pObj, IOpaqueAddress*& refpAddress)  {
  return makeCall(CREATE_REP, true, false, false, refpAddress, pObj);
}

/// Resolve the references of the converted object.
StatusCode ConversionSvc::fillRepRefs(IOpaqueAddress* pAddress, DataObject* pObj)  {
  return makeCall(FILL_REP_REFS, true, false, false, pAddress, pObj);
}

/// Update the converted representation of a transient object.
StatusCode ConversionSvc::updateRep(IOpaqueAddress* pAddress, DataObject* pObj)  {
  return makeCall(UPDATE_REP, true, false, false, pAddress, pObj);
}

/// Update the references of an already converted object.
StatusCode ConversionSvc::updateRepRefs(IOpaqueAddress* pAddress, DataObject* pObj)    {
  return makeCall(UPDATE_REP_REFS, true, false, false, pAddress, pObj);
}

/// Retrieve converter from list
IConverter* ConversionSvc::converter(const CLID& clid)     {
  IConverter* cnv = 0;
  Workers::iterator i = std::find_if(m_workers->begin(),m_workers->end(),CnvTest(clid));
  if ( i != m_workers->end() )      {
    cnv = (*i).converter();
  }
  if ( 0 == cnv )     {
    StatusCode status = addConverter(clid);
    if ( status.isSuccess() )   {
      i = std::find_if(m_workers->begin(),m_workers->end(),CnvTest(clid));
      if ( i != m_workers->end() )      {
        cnv = (*i).converter();
      }
    }
  }
  return cnv;
}

/// Define transient data store
StatusCode ConversionSvc::setDataProvider(IDataProviderSvc* pDataSvc)    {
  if ( !pDataSvc ) return StatusCode::SUCCESS; //Atlas does not use DataSvc
  if ( m_dataSvc ) m_dataSvc->release();
  m_dataSvc = pDataSvc;
  m_dataSvc->addRef();
  Workers::iterator stop  = m_workers->end();
  Workers::iterator start = m_workers->begin();
  for(Workers::iterator i=start; i != stop; i++ )    {
    IConverter* cnv = (*i).converter();
    if ( 0 != cnv )   {
      if (cnv->setDataProvider(m_dataSvc).isFailure()) {
        MsgStream log(msgSvc(), name());
        log << MSG::ERROR << "setting Data Provider" << endmsg;
      }
    }
  }
  return StatusCode::SUCCESS;
}

/// Access the transient store
SmartIF<IDataProviderSvc>& ConversionSvc::dataProvider()  const   {
  return m_dataSvc;
}

/// Set address creator facility
StatusCode ConversionSvc::setAddressCreator(IAddressCreator* creator)   {
  m_addressCreator = creator;
  Workers::iterator stop  = m_workers->end();
  Workers::iterator start = m_workers->begin();
  for(Workers::iterator i=start; i != stop; i++ )    {
    IConverter* cnv = (*i).converter();
    if ( 0 != cnv )   {
      if (cnv->setAddressCreator(m_addressCreator).isFailure()) {
	MsgStream log(msgSvc(), name());
	log << MSG::ERROR << "setting Address Creator"  << endmsg;
      }
    }
  }
  return StatusCode::SUCCESS;
}

/// Access the transient store
SmartIF<IAddressCreator>& ConversionSvc::addressCreator()  const   {
  return m_addressCreator;
}

/// Set conversion service the converter is connected to
StatusCode ConversionSvc::setConversionSvc(IConversionSvc* /* svc */)   {
  return StatusCode::FAILURE;
}

/// Get conversion service the converter is connected to
SmartIF<IConversionSvc>& ConversionSvc::conversionSvc()    const   {
  return m_cnvSvc;
}

/// Add converter object to conversion service.
StatusCode ConversionSvc::addConverter(const CLID& clid)  {
  // First look for the more specific converter
  long typ = repSvcType();
  IConverter* pConverter = createConverter(typ, clid, 0);
  if ( 0 != pConverter )    {
    StatusCode status = configureConverter( typ, clid, pConverter );
    if ( status.isSuccess() )   {
      status = initializeConverter( typ, clid, pConverter );
      if ( status.isSuccess() )   {
        status = activateConverter( typ, clid, pConverter );
        if ( status.isSuccess() )   {
          long conv_typ  = pConverter->repSvcType();
          const CLID&   conv_clid = pConverter->objType();
          typ      = (typ<0xFF) ? typ : typ&0xFFFFFF00;
          conv_typ = (conv_typ<0xFF) ? conv_typ : conv_typ&0xFFFFFF00;
          if ( conv_typ == typ && conv_clid == clid )   {
            return addConverter(pConverter);
          }
        }
      }
    }
    pConverter->release();
  }
  return NO_CONVERTER;
}

/// Add converter object to conversion service.
StatusCode ConversionSvc::addConverter(IConverter* pConverter)    {
  if ( 0 != pConverter )    {
    const CLID& clid = pConverter->objType();
    removeConverter(clid).ignore();
    m_workers->push_back(WorkerEntry(clid, pConverter));
    pConverter->addRef();
    return StatusCode::SUCCESS;
  }
  return NO_CONVERTER;
}

/// Remove converter object from conversion service (if present).
StatusCode ConversionSvc::removeConverter(const CLID& clid)  {
  CnvTest test(clid);
  Workers::iterator stop  = m_workers->end();
  Workers::iterator start = m_workers->begin();
  for(Workers::iterator i=start; i != stop; i++ )    {
    if ( test( *i ) )   {
      (*i).converter()->finalize().ignore();
      (*i).converter()->release();
    }
  }
  Workers::iterator j = std::remove_if(start, stop, test);
  if ( j != stop )  {
    m_workers->erase(j, stop);
    return StatusCode::SUCCESS;
  }
  return NO_CONVERTER;
}

/// Initialize the service.
StatusCode ConversionSvc::initialize()     {
  StatusCode status = Service::initialize();
  return status;
}

/// stop the service.
StatusCode ConversionSvc::finalize()      {
  // Release all workers.
  MsgStream log(msgSvc(), name());
  for ( Workers::iterator i = m_workers->begin(); i != m_workers->end(); i++ )    {
    if ( (*i).converter()->finalize().isFailure() ) {
      log << MSG::ERROR << "finalizing worker" << endmsg;
    }
    (*i).converter()->release();
  }
  m_workers->erase(m_workers->begin(), m_workers->end() );
  // release interfaces
  m_addressCreator = 0;
  m_dataSvc = 0;
  m_cnvSvc = 0;
  return Service::finalize();
}


/// Create new Converter using factory
IConverter* ConversionSvc::createConverter(long typ,
                                           const CLID& clid,
                                           const ICnvFactory* /*fac*/)   {
  IConverter* pConverter;
  pConverter = PluginService::CreateWithId<IConverter*>(ConverterID(typ,clid),serviceLocator().get());
  if ( 0 == pConverter )  {
    typ = (typ<0xFF) ? typ : typ&0xFFFFFF00;
    pConverter = PluginService::CreateWithId<IConverter*>(ConverterID(typ,clid),serviceLocator().get());
  }
  return pConverter;
}

/// Configure the freshly created converter
StatusCode ConversionSvc::configureConverter(long /* typ */,
                                              const CLID& /* clid */,
                                              IConverter* pConverter)    {
  if ( 0 != pConverter )    {
    pConverter->setConversionSvc(this).ignore();
    pConverter->setAddressCreator(m_addressCreator).ignore();
    pConverter->setDataProvider(m_dataSvc).ignore();
    return StatusCode::SUCCESS;
  }
  return NO_CONVERTER;
}

/// Initialize new converter
StatusCode ConversionSvc::initializeConverter(long /* typ */,
                                              const CLID& /* clid */,
                                              IConverter* pConverter)    {
  if ( pConverter )    {
    return pConverter->initialize();
  }
  return NO_CONVERTER;
}

/// Activate the freshly created converter
StatusCode ConversionSvc::activateConverter(long /* typ */,
                                            const CLID& /* clid */,
                                            IConverter* pConverter)    {
  if ( 0 != pConverter )    {
    return StatusCode::SUCCESS;
  }
  return NO_CONVERTER;
}

/// Retrieve the class type of objects the converter produces.
const CLID& ConversionSvc::objType() const    {
  return CLID_NULL;
}

/// Retrieve the class type of the data store the converter uses.
long ConversionSvc::repSvcType() const {
  return m_type;
}

/// Connect the output file to the service with open mode.
StatusCode ConversionSvc::connectOutput(const std::string&    outputFile,
                                        const std::string& /* openMode */) {
  return connectOutput(outputFile);
}

/// Connect the output file to the service.
StatusCode ConversionSvc::connectOutput(const std::string& /* outputFile */) {
  return StatusCode::SUCCESS;
}

/// Commit pending output.
StatusCode ConversionSvc::commitOutput(const std::string& , bool ) {
  return StatusCode::SUCCESS;
}

/// Create a Generic address using explicit arguments to identify a single object.
StatusCode ConversionSvc::createAddress(long                 /* svc_type */,
                                        const CLID&          /* clid     */,
                                        const std::string*   /* par      */,
                                        const unsigned long* /* ip       */,
                                        IOpaqueAddress*& refpAddress)    {
  refpAddress = 0;
  return StatusCode::FAILURE;
}

/// Convert an address to string form
StatusCode ConversionSvc::convertAddress( const IOpaqueAddress* /* pAddress */,
                                          std::string& refAddress)
{
  refAddress = "";
  return StatusCode::FAILURE;
}

/// Convert an address in string form to object form
StatusCode ConversionSvc::createAddress( long /* svc_type */,
                                        const CLID& /* clid */,
                                        const std::string& /* refAddress */,
                                        IOpaqueAddress*& refpAddress)
{
  refpAddress = 0;
  return StatusCode::FAILURE;
}

/// Standard Constructor
ConversionSvc::ConversionSvc(const std::string& name, ISvcLocator* svc, long type)
 : base_class(name, svc),
   m_cnvSvc(static_cast<IConversionSvc*>(this))
{
  m_type            = type;
  m_dataSvc         = 0;
  m_workers = new Workers();
  setAddressCreator(this).ignore();
}

/// Standard Destructor
ConversionSvc::~ConversionSvc()   {
  // Release all workers.
  for ( Workers::iterator i = m_workers->begin(); i != m_workers->end(); i++ )    {
    (*i).converter()->release();
  }
  m_workers->erase(m_workers->begin(), m_workers->end() );
  delete m_workers;
}
