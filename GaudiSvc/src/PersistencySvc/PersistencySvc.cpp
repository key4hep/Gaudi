//====================================================================
//  PersistencySvc.cpp
//--------------------------------------------------------------------
//
//  Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the PersistencySvc
//
//  Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  PERSISTENCYSVC_PERSISTENCYSVC_CPP

// Interface defintions
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/CnvFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/IDataProviderSvc.h"

// Implementation specific definitions
#include "PersistencySvc.h"

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(PersistencySvc)

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

StatusCode PersistencySvc::makeCall(int typ,
                                    IOpaqueAddress*& pAddress,
                                    DataObject*& pObject)      {
  if ( m_enable )    {
    IConversionSvc* svc    = 0;
    switch(typ)   {
    case CREATE_REP:
    case FILL_REP_REFS:
    case UPDATE_REP:
    case UPDATE_REP_REFS:
      svc = m_cnvDefault;
      break;
    default:
      if ( 0 != pAddress )    {
        long svc_type = pAddress->svcType();
        svc = service(svc_type);
        if ( 0 == svc )   {
          return BAD_STORAGE_TYPE;
        }
      }
      else  {
        return INVALID_ADDRESS;
      }
      break;
    }

    StatusCode status(StatusCode::FAILURE,true);
    switch( typ )     {
    case CREATE_OBJ:
      pObject = 0;
      status = svc->createObj(pAddress, pObject);
      break;
    case FILL_OBJ_REFS:
      status = svc->fillObjRefs(pAddress, pObject);
      break;
    case UPDATE_OBJ:
      status = svc->updateObj(pAddress, pObject);
      break;
    case UPDATE_OBJ_REFS:
      status = svc->updateObjRefs(pAddress, pObject);
      break;
    case CREATE_REP:
      status = svc->createRep(pObject, pAddress);
      break;
    case FILL_REP_REFS:
      status = svc->fillRepRefs(pAddress, pObject);
      break;
    case UPDATE_REP:
      status = svc->updateRep(pAddress, pObject);
      break;
    case UPDATE_REP_REFS:
      status = svc->updateRepRefs(pAddress, pObject);
      break;
    default:
      status = StatusCode::FAILURE;
      break;
    }
    status.ignore();
    return status;
  }
  return StatusCode::SUCCESS;
}

/// Create the transient representation of an object.
StatusCode PersistencySvc::createObj(IOpaqueAddress* pAddr, DataObject*& refpObj)   {
  return makeCall(CREATE_OBJ, pAddr, refpObj);
}

/// Resolve the references of the created transient object.
StatusCode PersistencySvc::fillObjRefs(IOpaqueAddress* pAddr, DataObject* pObj)    {
  return makeCall(FILL_OBJ_REFS, pAddr, pObj);
}

/// Update the transient object from the other representation.
StatusCode PersistencySvc::updateObj(IOpaqueAddress* pAddr, DataObject* pObj)   {
  return makeCall(UPDATE_OBJ, pAddr, pObj);
}

/// Update the references of an updated transient object.
StatusCode PersistencySvc::updateObjRefs(IOpaqueAddress* pAddr, DataObject* pObj)  {
  return makeCall(UPDATE_OBJ_REFS, pAddr, pObj);
}

/// Convert the transient object to the requested representation.
StatusCode PersistencySvc::createRep(DataObject* pObj, IOpaqueAddress*& refpAddr)  {
  return makeCall(CREATE_REP, refpAddr, pObj);
}

/// Resolve the references of the converted object.
StatusCode PersistencySvc::fillRepRefs(IOpaqueAddress* pAddr, DataObject* pObj)  {
  return makeCall(FILL_REP_REFS, pAddr, pObj);
}

/// Update the converted representation of a transient object.
StatusCode PersistencySvc::updateRep(IOpaqueAddress* pAddr, DataObject* pObj)  {
  return makeCall(UPDATE_REP, pAddr, pObj);
}

/// Update the references of an already converted object.
StatusCode PersistencySvc::updateRepRefs(IOpaqueAddress* pAddr, DataObject* pObj)    {
  return makeCall(UPDATE_REP_REFS, pAddr, pObj);
}

/// Retrieve address creator service from list
SmartIF<IAddressCreator>& PersistencySvc::addressCreator(long type)     {
  long typ = type;
  Services::iterator it = m_cnvServices.find( typ );
  if( it == m_cnvServices.end() ) {
    IConversionSvc* s = service(type);
    if ( s )   {
      it = m_cnvServices.find( typ );
      if ( it != m_cnvServices.end() ) {
        return (*it).second.addrCreator();
      }
    }
    static SmartIF<IAddressCreator> no_creator;
    return no_creator;
  }
  return (*it).second.addrCreator();
}

/// Define transient data store
StatusCode PersistencySvc::setDataProvider(IDataProviderSvc* pDataSvc)    {
  m_dataSvc = pDataSvc;
  for ( Services::iterator i = m_cnvServices.begin(); i != m_cnvServices.end(); i++ )   {
    (*i).second.conversionSvc()->setDataProvider(m_dataSvc).ignore();
  }
  return StatusCode(StatusCode::SUCCESS,true);
}

/// Access the dataprovider service
SmartIF<IDataProviderSvc>& PersistencySvc::dataProvider()  const  {
  return m_dataSvc;
}

/// Set conversion service the converter is connected to
StatusCode PersistencySvc::setConversionSvc(IConversionSvc* svc)   {
  m_cnvDefault = svc;
  return StatusCode(StatusCode::SUCCESS,true);
}

/// Get conversion service the converter is connected to
SmartIF<IConversionSvc>& PersistencySvc::conversionSvc()    const   {
  return m_cnvDefault;
}

/// Add converter object to conversion service.
StatusCode PersistencySvc::addConverter(const CLID& /* clid */)  {
  return StatusCode::FAILURE;
}

/// Add converter object to conversion service.
StatusCode PersistencySvc::addConverter(IConverter* pConverter)    {
  if ( 0 != pConverter )    {
    long typ  = pConverter->repSvcType();
    IConversionSvc* svc = service(typ);
    if ( 0 != svc )   {
      return svc->addConverter(pConverter);
    }
    return BAD_STORAGE_TYPE;
  }
  return NO_CONVERTER;
}

/// Remove converter object from conversion service (if present).
StatusCode PersistencySvc::removeConverter(const CLID& clid)  {
  // Remove converter type from all services
  StatusCode status = NO_CONVERTER, iret = StatusCode::SUCCESS;
  for ( Services::iterator i = m_cnvServices.begin(); i != m_cnvServices.end(); i++ )    {
    iret = (*i).second.conversionSvc()->removeConverter(clid);
    if ( iret.isSuccess() )    {
      status = iret;
    }
  }
  return status;
}

/// Retrieve converter from list
IConverter* PersistencySvc::converter(const CLID& /*clid*/) {
  return 0;
}

/// Retrieve conversion service by name
SmartIF<IConversionSvc>& PersistencySvc::service(const std::string& nam)     {
  Gaudi::Utils::TypeNameString tn(nam);
  IConversionSvc* svc = 0;
  for ( Services::iterator it = m_cnvServices.begin(); it != m_cnvServices.end(); it++ )    {
    if ( (*it).second.service()->name() == tn.name() )   {
      return (*it).second.conversionSvc();
    }
  }
  StatusCode status = Service::service(nam, svc, true);
  if ( status.isSuccess() )   {
    if ( addCnvService(svc).isSuccess() )   {
      svc->release();       // Do not double-reference count
      return service(nam); // now it is in the list
    }
  }
  info() << "Cannot access Conversion service:" << nam << endmsg;
  static SmartIF<IConversionSvc> no_svc;
  return no_svc;
}

/// Retrieve conversion service from list
SmartIF<IConversionSvc>& PersistencySvc::service(long type)     {
  typedef std::vector<std::string> SvcNames;
  // Check wether this is already an active service
  Services::iterator it = m_cnvServices.find( type );
  if( it != m_cnvServices.end() ) {
    return (*it).second.conversionSvc();
  }
  // if not, check if the service is in the list and may be requested
  const SvcNames& theNames = m_svcNames.value();
  for ( SvcNames::const_iterator i = theNames.begin(); i != theNames.end(); i++ )   {
    SmartIF<IConversionSvc>& svc = service(*i);
    if ( svc != 0 )  {
      long typ = svc->repSvcType();
      if ( typ == type )    {
        return svc;
      }
    }
  }
  static SmartIF<IConversionSvc> no_svc;
  return no_svc;
}

/// Add data service
StatusCode PersistencySvc::addCnvService(IConversionSvc* servc)    {
  if ( 0 != servc )   {
    long type = servc->repSvcType();
    long def_typ = (m_cnvDefault) ? m_cnvDefault->repSvcType() : 0;
    Services::iterator it = m_cnvServices.find( type );
    IConversionSvc* cnv_svc = 0;
    if ( it != m_cnvServices.end() )    {
      cnv_svc = (*it).second.conversionSvc();
    }
    if ( type == def_typ )     {
      m_cnvDefault = servc;
    }
    if ( cnv_svc != servc )   {
      IAddressCreator* icr = 0;
      StatusCode status  = servc->queryInterface(IAddressCreator::interfaceID(), pp_cast<void>(&icr));
      if ( status.isSuccess() )   {
        IService* isvc = 0;
        status = servc->queryInterface(IService::interfaceID(), pp_cast<void>(&isvc));
        if ( status.isSuccess() )    {
          if ( 0 != cnv_svc )   {
            removeCnvService (type).ignore();
          }
          std::pair<Services::iterator, bool> p =
            m_cnvServices.insert( Services::value_type( type, ServiceEntry(type, isvc, servc, icr)));
          if( p.second )    {
            info() << "Added successfully Conversion service:" << isvc->name() << endmsg;
            servc->addRef();
            servc->setAddressCreator(this).ignore();
            servc->setDataProvider(m_dataSvc).ignore();
            return StatusCode::SUCCESS;
          }
          info() << "Cannot add Conversion service of type " << isvc->name() << endmsg;
          isvc->release();
          icr->release();
          return StatusCode::FAILURE;
        }
        icr->release();
      }
      info() << "Cannot add Conversion service of type " << type << endmsg;
      return StatusCode::FAILURE;
    }
    else    {
      return StatusCode::SUCCESS;
    }
  }
  return BAD_STORAGE_TYPE;
}

/// Remove conversion service
StatusCode PersistencySvc::removeCnvService(long svctype)    {
  Services::iterator it = m_cnvServices.find( svctype );
  if( it != m_cnvServices.end() ) {
    (*it).second.service()->release();
    (*it).second.addrCreator()->release();
    m_cnvServices.erase(it);
    return StatusCode::SUCCESS;
  }
  return BAD_STORAGE_TYPE;
}

/// Retrieve the class type of the data store the converter uses.
long PersistencySvc::repSvcType() const {
  long typ = (m_cnvDefault) ? m_cnvDefault->repSvcType() : 0;
  return typ;
}

/// Set default conversion service
StatusCode PersistencySvc::setDefaultCnvService(long type)     {
  m_cnvDefault = service(type);
  return StatusCode::SUCCESS;
}

/// Connect the output file to the service with open mode.
StatusCode PersistencySvc::connectOutput(const std::string&    outputFile,
                                         const std::string& /* openMode */) {
  return connectOutput(outputFile);
}

/// Connect the output file to the service.
StatusCode PersistencySvc::connectOutput(const std::string&) {
  return StatusCode::SUCCESS;
}

/// Commit pending output.
StatusCode PersistencySvc::commitOutput(const std::string& , bool ) {
  return StatusCode::SUCCESS;
}

/// Create a Generic address using explicit arguments to identify a single object.
StatusCode PersistencySvc::createAddress(long svc_type,
                                         const CLID& clid,
                                         const std::string* pars,
                                         const unsigned long* ipars,
                                         IOpaqueAddress*& refpAddress)    {
  IAddressCreator* svc = addressCreator(svc_type);
  StatusCode   status  = BAD_STORAGE_TYPE;        // Preset error
  refpAddress = 0;
  if ( 0 != svc )   {
    status = svc->createAddress(svc_type, clid, pars, ipars, refpAddress);
  }
  return status;
}

/// Convert an address to string form
StatusCode PersistencySvc::convertAddress( const IOpaqueAddress* pAddress,
                                           std::string& refAddress)
{
  // Assumuption is that the Persistency service prepends a header
  // and requests the conversion service refered to by the service
  // type to encode the rest
  long svc_type = 0;
  CLID clid = 0;
  if ( 0 != pAddress ) {
    svc_type = pAddress->svcType();
    clid     = pAddress->clID();
  }
  IAddressCreator* svc = addressCreator(svc_type);
  StatusCode   status  = BAD_STORAGE_TYPE;        // Preset error
  refAddress = "";

  if ( 0 != svc )   {
    // Found service, set header
    encodeAddrHdr(svc_type, clid, refAddress);
    std::string address;
    // Get rest of address from conversion service
    status = svc->convertAddress(pAddress, address);
    refAddress += address;
  }
  return status;
}

/// Convert an address in string form to object form
StatusCode PersistencySvc::createAddress( long /* svc_type */,
                                         const CLID& /* clid */,
                                         const std::string& refAddress,
                                         IOpaqueAddress*& refpAddress)
{
  // Assumption is that the Persistency service decodes that header
  // and requests the conversion service referred to by the service
  // type to decode the rest
  long new_svc_type = 0;
  CLID new_clid = 0;
  std::string address_trailer;
  decodeAddrHdr(refAddress, new_svc_type, new_clid, address_trailer);
  IAddressCreator* svc = addressCreator(new_svc_type);
  StatusCode   status  = BAD_STORAGE_TYPE;        // Preset error
  if ( 0 != svc )   {
    status = svc->createAddress( new_svc_type, new_clid, address_trailer, refpAddress);
  }
  return status;
}

/// Retrieve string from storage type and clid
void PersistencySvc::encodeAddrHdr( long service_type,
                                    const CLID& clid,
                                    std::string& address) const
{
  // For address header, use xml-style format of
  // <addrhdr service_type="xxx" clid="yyy" />
  std::stringstream stream;
  int svctyp = service_type; // must put int into stream, not char
  stream << "<address_header service_type=\"" << svctyp << "\" clid=\"" << clid << "\" /> ";
  address = stream.str();
}

/// Retrieve storage type and clid from address header of string
void PersistencySvc::decodeAddrHdr( const std::string& address,
                                    long& service_type,
                                    CLID& clid,
                                    std::string& address_trailer) const
{
  // For address header, use xml-style format of
  // <address_header service_type="xxx" clid="yyy" />
  service_type = 0;
  clid = 0;
  address_trailer = "";

  // Check for address_header tag
  size_t pos = address.find("<address_header");
  if (std::string::npos != pos) {
    // Get service_type
    pos = address.find("service_type=\"");
    if (std::string::npos != pos) {
      pos += 14;
      size_t end = address.find('"', pos);
      if (std::string::npos != end) {
        std::istringstream str(address.substr(pos, end-pos));
        str >> service_type;
        // Get clid
        pos = address.find("clid=\"");
        if (std::string::npos != pos) {
          pos += 6;
          end = address.find('\"', pos);
          if (std::string::npos != end) {
            str.clear(); // reuse the istringstream (the error flags must be explicitly cleared)
            str.str(address.substr(pos, end-pos));
            str >> clid;
            // Get trailer_address
            pos = address.find('>');
            if (pos < (address.size()-2)) { // this means that '>' was found (pos != npos)
                                            // it is before the last char
              address_trailer = address.substr(pos+1);
            }
          }
        }
      }
    }
  }
}

/// Set address creator facility
StatusCode PersistencySvc::setAddressCreator(IAddressCreator*)    {
  // The persistency service is a address creation dispatcher istelf.
  // The persistency service can NEVER create addresses itself.
  // The entry point must only be provided in order to fulfill the needs of the
  // implementing interfaces.
  return StatusCode::FAILURE;
}

/// Retrieve address creator facility
SmartIF<IAddressCreator>& PersistencySvc::addressCreator()   const   {
  return m_addrCreator;
}

/// Retrieve conversion service identified by technology
StatusCode PersistencySvc::getService(long service_type, IConversionSvc*& refpSvc)     {
  refpSvc = service(service_type);
  return (0==refpSvc) ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

/// Retrieve conversion service identified by technology
StatusCode PersistencySvc::getService(const std::string& service_type, IConversionSvc*& refpSvc)     {
  const char* imp = service_type.c_str();
  long len = service_type.length();
  if ( ::strncasecmp(imp,"SICB", len) == 0 )
    return getService(SICB_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"ZEBRA", len) == 0 )
    return getService(SICB_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"MS Access", len) == 0 )
    return getService(ACCESS_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"Microsoft Access", strlen("Microsoft Access")) == 0 )
    return getService(ACCESS_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"SQL Server", len) == 0 )
    return getService(SQLSERVER_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"Microsoft ODBC for Oracle", len) == 0 )
    return getService(ORACLE_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"Oracle ODBC", strlen("Oracle ODBC")) == 0 )
    return getService(ORACLE_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"Oracle OCI", strlen("Oracle OCI")) == 0 )
    return getService(ORACLE_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"MySQL", len) == 0 )
    return getService(MYSQL_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"ROOT", len) == 0 )
    return getService(ROOT_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"OBJY", len) == 0 )
    return getService(OBJY_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"OBJYECTI", 7) == 0 )
    return getService(OBJY_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL_ROOTKEY", 12) == 0 )
    return getService(POOL_ROOTKEY_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL_ROOTTREE", 12) == 0 )
    return getService(POOL_ROOTTREE_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL_ROOT", 9) == 0 )
    return getService(POOL_ROOT_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL_MySQL", 8) == 0 )
    return getService(POOL_MYSQL_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL_ORACLE", 8) == 0 )
    return getService(POOL_ORACLE_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL_ACCESS", 8) == 0 )
    return getService(POOL_ACCESS_StorageType, refpSvc);
  else if ( ::strncasecmp(imp,"POOL", 4) == 0 )
    return getService(POOL_StorageType, refpSvc);

  for(Services::const_iterator i=m_cnvServices.begin(); i != m_cnvServices.end();++i)  {
    SmartIF<IService> svc((*i).second.conversionSvc());
    if ( svc )  {
      // Check wether this is already an active service: first check by service name
      if ( svc->name() == service_type )  {
        refpSvc = (*i).second.conversionSvc();
        return StatusCode::SUCCESS;
      }
      // Check wether this is already an active service: now check by service type
      if ( System::typeinfoName(typeid(*(svc.get()))) == service_type )  {
        refpSvc = (*i).second.conversionSvc();
        return StatusCode::SUCCESS;
      }
    }
  }
  const std::vector<std::string>& names = m_svcNames;
  // if not, check if the service is in the list and may be requested
  for(std::vector<std::string>::const_iterator i=names.begin(); i != names.end(); i++) {
    Gaudi::Utils::TypeNameString itm(*i);
    if ( itm.name() == service_type || itm.type() == service_type )  {
      IConversionSvc* svc = service(*i);
      if ( svc )  {
        refpSvc = svc;
        return StatusCode::SUCCESS;
      }
    }
  }
  return StatusCode::FAILURE;
}

/// Retrieve the class type of objects the converter produces.
const CLID& PersistencySvc::objType() const    {
  return CLID_NULL;
}

/// Initialize the service.
StatusCode PersistencySvc::initialize()     {
  m_addrCreator = this; // initialize internal pointer to IAddressCreator interface
  // Initialize basic service
  StatusCode status = Service::initialize();
  if ( UNLIKELY(!status.isSuccess()) )   {
    error() << "Error initializing Service base class." << endmsg;
  }
  return status;
}

/// stop the service.
StatusCode PersistencySvc::finalize()      {
  // Release all workers
  m_cnvServices.clear();
  // Release references to this to avoid loops
  m_addrCreator = 0;
  return StatusCode::SUCCESS;
}

void PersistencySvc::svcNamesHandler( Property& p )     {
  DEBMSG << p << endmsg;
}

/// Set enabled flag
bool PersistencySvc::enable(bool value)
{
  bool old = m_enable;
  m_enable = value;
  return old;
}

/// Standard Constructor
PersistencySvc::PersistencySvc(const std::string& name, ISvcLocator* svc)
:  base_class(name, svc),
   m_cnvDefType(TEST_StorageType),
   m_enable(true)
{
  declareProperty("CnvServices", m_svcNames);
  m_svcNames.declareUpdateHandler( &PersistencySvc::svcNamesHandler, this );
}

/// Standard Destructor
PersistencySvc::~PersistencySvc()   {
}
