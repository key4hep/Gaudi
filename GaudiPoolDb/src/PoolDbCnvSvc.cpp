// $Id: PoolDbCnvSvc.cpp,v 1.30 2008/10/27 16:41:33 marcocle Exp $

#ifdef __ICC
// disable icc warning #654: overloaded virtual function "IAddressCreator::createAddress" is only partially overridden in class "PoolDbCnvSvc"
//   TODO: there is only a partial overload of IAddressCreator::createAddress
#pragma warning(disable:654)
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, coming from CORAL
#pragma warning(disable:2259)
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning(disable:4996)
#endif

//====================================================================
//	PoolDbCnvSvc implementation
//--------------------------------------------------------------------
//
//  Description: Implementation of the POOL data storage
//
//	Author     : M.Frank
//
//====================================================================
// Framework include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/CnvFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "GaudiPoolDb/IPoolCacheSvc.h"
#include "GaudiPoolDb/PoolDbCnvSvc.h"
#include "GaudiPoolDb/PoolDbAddress.h"
#include "GaudiPoolDb/PoolDbDataConnection.h"

// POOL include files
#include "POOLCore/Token.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbObject.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DataCallBack.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbInstanceCount.h"
#include "StorageSvc/DbOptionCallback.h"
#include "GaudiUtils/IFileCatalog.h"
#include "GaudiUtils/IIODataManager.h"

//ROOT include files
#include "TInterpreter.h"

#include <sstream>
#include <set>
#include <memory>

using ROOT::Reflex::PluginService;

static pool::DbInstanceCount::Counter* s_count =
  pool::DbInstanceCount::getCounter(typeid(PoolDbCnvSvc));

DECLARE_SERVICE_FACTORY(PoolDbCnvSvc)

typedef std::vector<std::string> StringV;
typedef const std::string& CSTR;

#define S_OK   StatusCode::SUCCESS
#define S_FAIL StatusCode::FAILURE
using pool::DbType;
using pool::DbContainer;
using pool::DbDomain;
using pool::DbDatabase;
using pool::DbTypeInfo;
using pool::DbTransaction;
using pool::DbOptionCallback;
using pool::DbAccessMode;
using pool::Transaction;
using namespace Gaudi;

static void checkAccessMode(DbAccessMode mode, DbDatabase& dbH)  {
  DbAccessMode m = dbH.openMode();
  if ( m&mode || m&pool::UPDATE || m&pool::RECREATE )  {
    return;
  }
  std::string err =
      "The dataset "+dbH.name()+" cannot be opened in mode "+
      +pool::accessMode(mode)+
      +"\nThe database was already open, but in mode "
      +pool::accessMode(m);
  throw std::runtime_error(err);
}

/// Standard constructor
PoolDbCnvSvc::PoolDbCnvSvc(CSTR nam, ISvcLocator* svc)
: base_class( nam, svc, POOL_StorageType),
  m_dataMgr(0), m_cacheSvc(0), m_current(0),
  m_domH(pool::POOL_StorageType),
  m_catalog(0), m_ioMgr(0)
{
  s_count->increment();
  declareProperty("Server",           m_serverConnect    = "");
  declareProperty("DbType",           m_implementation   = "Unknown");
  declareProperty("PoolCache",        m_cacheSvcName     = "PoolDbCacheSvc");
  declareProperty("ShareFiles",       m_shareFiles       = "NO");
  declareProperty("SafeTransactions", m_safeTransactions = false);
  declareProperty("CheckFIDs",        m_checkFIDs        = true);
  declareProperty("EnableIncident",   m_incidentEnabled  = false);
  declareProperty("RecordsName",      m_recordName="/FileRecords");
}

/// Standard destructor
PoolDbCnvSvc::~PoolDbCnvSvc()   {
  s_count->decrement();
}

/// Initialize the Db data persistency service
StatusCode PoolDbCnvSvc::initialize()  {
  StatusCode status = ConversionSvc::initialize();
  MsgStream log(msgSvc(), name());
  if ( !status.isSuccess() ) {
    log << MSG::ERROR << "Failed to initialize ConversionSvc base class."
        << endmsg;
    return status;
  }
  status = accessStorageType(m_implementation, m_type);
  if ( !status.isSuccess() )  {
    log << MSG::ERROR << "Failed to connect to POOL implementation:"
        << m_implementation << endmsg;
    return status;
  }
  status = service(m_cacheSvcName, m_cacheSvc);
  if ( !status.isSuccess() )  {
    log << MSG::ERROR << "Failed to connect to POOL cache service:"
        << m_cacheSvcName << endmsg;
    return status;
  }
  status = service("IODataManager", m_ioMgr);
  if( !status.isSuccess() ) {
    log << MSG::ERROR
        << "Unable to localize interface from service:IODataManager" << endmsg;
    return status;
  }
  status = service("FileCatalog", m_catalog);
  if( !status.isSuccess() ) {
    log << MSG::ERROR
        << "Unable to localize interface from service:FileCatalog" << endmsg;
    return status;
  }
  status = service("IncidentSvc", m_incidentSvc);
  if( !status.isSuccess() ) {
    log << MSG::ERROR
        << "Unable to localize interface from service:IncidentSvc" << endmsg;
    return status;
  }
  DbOptionCallback cb(m_cacheSvc->callbackHandler());
  if ( m_domH.open(m_cacheSvc->session(),m_type,pool::UPDATE).isSuccess() )  {
    SmartIF<IProperty> prp(m_ioMgr);
    IntegerProperty ageLimit;
    ageLimit.assign(prp->getProperty("AgeLimit"));
    log << MSG::DEBUG << "POOL agelimit is set to " << ageLimit.value() << endmsg;
    m_domH.setAgeLimit(ageLimit.value());
    return S_OK;
  }
  return S_OK;
}

/// Finalize the Db data persistency service
StatusCode PoolDbCnvSvc::finalize()    {
  MsgStream log(msgSvc(),name());
  DbOptionCallback cb(m_cacheSvc->callbackHandler());
  if ( m_ioMgr )  {
    if ( ::toupper(m_shareFiles[0]) != 'Y' )  {
      IIODataManager::Connections cons = m_ioMgr->connections(this);
      for(IIODataManager::Connections::iterator i=cons.begin(); i != cons.end(); ++i)  {
        if ( m_ioMgr->disconnect(*i).isSuccess() )  {
          log << MSG::INFO << "Disconnected data IO:" << (*i)->fid();
	  log << "[" << (*i)->pfn() << "]";
	  log << endmsg;
          delete (*i);
        }
      }
    }
    else  {
      log << MSG::INFO << "File sharing enabled. Do not retire files." << endmsg;
    }
    m_ioMgr->release();
    m_ioMgr = 0;
  }
  m_domH.close();
  m_domH = 0;
  if ( m_dataMgr ) m_dataMgr->clearStore().ignore();
  pool::releasePtr(m_incidentSvc);
  pool::releasePtr(m_dataMgr);
  pool::releasePtr(m_catalog);
  pool::releasePtr(m_cacheSvc);
  StatusCode status = ConversionSvc::finalize();
  log << MSG::DEBUG << "POOL conversion service finalized " << name() << " ";
  log << (const char*)(status.isSuccess() ? "successfully" : "with errors") << endmsg;
  return status;
}

/// Update state of the service
StatusCode PoolDbCnvSvc::updateServiceState(IOpaqueAddress* /* pAddr */)  {
  return S_OK;
}

/// Create new Converter using factory
IConverter*
PoolDbCnvSvc::createConverter(long typ,const CLID& wanted,const ICnvFactory*)
{
  IConverter* pConverter;
  ConverterID cnvid(POOL_StorageType, wanted);
  pConverter = PluginService::CreateWithId<IConverter*>(cnvid, typ, wanted, serviceLocator().get());
  if ( 0 == pConverter )  {
    const CLID gen_clids[] = {
    /* ObjectList               */ CLID_Any + CLID_ObjectList,
    /* ObjectVector             */ CLID_Any + CLID_ObjectVector,
    /* Keyed Map                */ CLID_Any + CLID_ObjectVector+0x00030000,
    /* Keyed Hashmap            */ CLID_Any + CLID_ObjectVector+0x00040000,
    /* Keyed redirection array  */ CLID_Any + CLID_ObjectVector+0x00050000,
    /* Standard, non-container  */ CLID_Any
    };
    for ( unsigned int i = 0; i < sizeof(gen_clids)/sizeof(gen_clids[0]); i++ ) {
      if ( (wanted>>16) == (gen_clids[i]>>16) )  {
        ConverterID cnvid1(POOL_StorageType, gen_clids[i]);
        pConverter = PluginService::CreateWithId<IConverter*>(cnvid1, typ, wanted, serviceLocator().get());
        if ( 0 != pConverter ) {
          return pConverter;
        }
      }
    }
    // Check if a converter using object update is needed
    if ( (wanted>>24) != 0 )  {
      ConverterID cnvid1(POOL_StorageType, CLID_Any | 1<<31);
      pConverter = PluginService::CreateWithId<IConverter*>(cnvid1, typ, wanted, serviceLocator().get());
      if ( 0 != pConverter ) {
        return pConverter;
      }
    }
    // If we do not have found any suitable container after searching
    // for standard containers, we will use the "ANY" converter
    // ... and pray for everything will go well.
    ConverterID cnvid1(POOL_StorageType, CLID_Any);
    pConverter = PluginService::CreateWithId<IConverter*>(cnvid1, typ, wanted, serviceLocator().get());
    if ( 0 != pConverter ) {
      MsgStream log(msgSvc(), name());
      log << MSG::INFO << "Using \"Any\" converter "
          << "for objects of type "
          << std::showbase << std::hex << wanted << endmsg;
    }
  }
  return pConverter;
}

void PoolDbCnvSvc::loadConverter(DataObject* pObject) {
  if (pObject) {
    MsgStream log(msgSvc(), name());
    std::string cname = System::typeinfoName(typeid(*pObject));
    log << MSG::DEBUG << "Trying to 'Autoload' dictionary for class " << cname << endmsg;
    gInterpreter->EnableAutoLoading();
    gInterpreter->AutoLoad(cname.c_str());
  }
}

StatusCode PoolDbCnvSvc::setDataProvider(IDataProviderSvc* pDataSvc)  {
  IDataManagerSvc* tmp = m_dataMgr;
  if (pDataSvc)  {
    StatusCode status =
      pDataSvc->queryInterface(IDataManagerSvc::interfaceID(), pp_cast<void>(&m_dataMgr));
    if ( !status.isSuccess() )    {
      return error("Cannot connect to \"IDataManagerSvc\" interface.");
    }
  }
  if ( tmp ) tmp->release();
  return ConversionSvc::setDataProvider(pDataSvc);
}

/// Create access to the POOL persistency mechanism using
StatusCode PoolDbCnvSvc::accessStorageType(CSTR type_string, long& gaudi_type) {
  if ( ::strncasecmp(type_string.c_str(), "POOL_ROOTTREE", 10)==0 ) {
    gaudi_type  = POOL_ROOTTREE_StorageType;
    return S_OK;
  }
  else if ( ::strncasecmp(type_string.c_str(), "POOL_ROOTKEY", 10)==0 ) {
    gaudi_type  = POOL_ROOTKEY_StorageType;
    return S_OK;
  }
  else if ( ::strncasecmp(type_string.c_str(), "POOL_ROOT", 9)==0 ) {
    gaudi_type  = POOL_ROOT_StorageType;
    return S_OK;
  }
  gaudi_type = TEST_StorageType;
  return S_FAIL;
}

/// Connect the output file to the service with open mode.
StatusCode PoolDbCnvSvc::connectOutput(CSTR dsn, CSTR openMode)   {
  StatusCode sc = StatusCode::FAILURE;
  m_current = 0;
  if ( ::strncasecmp(openMode.c_str(),"RECREATE",3)==0 )
    sc = connectDatabase(UNKNOWN, dsn, pool::RECREATE, &m_current);
  else if ( ::strncasecmp(openMode.c_str(),"NEW",1)==0 )
    sc = connectDatabase(UNKNOWN, dsn, pool::CREATE, &m_current);
  else if ( ::strncasecmp(openMode.c_str(),"CREATE",1)==0 )
    sc = connectDatabase(UNKNOWN, dsn, pool::CREATE, &m_current);
  else if ( ::strncasecmp(openMode.c_str(),"UPDATE",1)==0 )
    sc = connectDatabase(UNKNOWN, dsn, pool::UPDATE, &m_current);
  if ( sc.isSuccess() && m_current && m_current->isConnected() )  {
    return S_OK;
  }
  m_incidentSvc->fireIncident(Incident(dsn,IncidentType::FailOutputFile));
  error("The dataset "+dsn+" cannot be opened in mode "+openMode+". [Invalid mode]");
  return sc;
}

// Conect output stream (valid until overwritten)
StatusCode PoolDbCnvSvc::connectOutput(CSTR db_name)  {
  return connectOutput(db_name, "NEW");
}

// Commit pending output on open container
StatusCode  PoolDbCnvSvc::commitOutput(CSTR dsn, bool doCommit) {
  if ( m_current )  {
    try  {
      if ( doCommit )
        m_current->transaction().set(m_safeTransactions ? Transaction::TRANSACT_FLUSH : Transaction::TRANSACT_COMMIT);
      else
        m_current->transaction().set(Transaction::TRANSACT_ROLLBACK);
      if ( m_current->database().transAct(m_current->transaction()).isSuccess() )  {
        return S_OK;
      }
      std::string action(doCommit ? "commit to" : "rollback");
      m_incidentSvc->fireIncident(Incident(dsn,IncidentType::FailOutputFile));
      return error("commitOutput> Cannot "+action+" database:"+dsn);
    }
    catch (std::exception& e)  {
      error(std::string("commitOutput> Caught exception:")+e.what(), false);
    }
    catch (...)   {
      error("commitOutput> Unknown Fatal Exception on commit to "+dsn, false);
    }
  }
  m_incidentSvc->fireIncident(Incident(dsn,IncidentType::FailOutputFile));
  return error("commitOutput> Cannot connect to database: "+dsn);
}

// Connect to a POOL container in read mode
StatusCode PoolDbCnvSvc::connect(CSTR dsn, CSTR cntName, DbContainer& cntH)   {
  return connectContainer(UNKNOWN, dsn, cntName, pool::READ, 0, cntH);
}

/// Connect the output file to the service with open mode.
StatusCode
PoolDbCnvSvc::connectDatabase(int typ, CSTR dataset, DbAccessMode mode, PoolDbDataConnection** con)  {
  try {
    DbOptionCallback cb(m_cacheSvc->callbackHandler());
    IDataConnection* c = m_ioMgr->connection(dataset);
    bool fire_incident = false;
    if ( !c )  {
      DbType dbType(DbType(m_type).majorType());
      std::auto_ptr<IDataConnection> connection(new PoolDbDataConnection(this,dataset,typ,mode,m_domH));
      StatusCode sc = (mode == pool::READ)
	? m_ioMgr->connectRead(false,connection.get())
	: m_ioMgr->connectWrite(connection.get(),IDataConnection::IoType(mode),dbType.storageName());
      c = sc.isSuccess() ? m_ioMgr->connection(dataset) : 0;
      if ( c )   {
	fire_incident = m_incidentEnabled && (0 != (mode&(pool::UPDATE|pool::READ)));
	connection.release();
      }
      else  {
	return sc;
      }
    }
    PoolDbDataConnection* pc = dynamic_cast<PoolDbDataConnection*>(c);
    if ( pc )  {
      if ( !pc->isConnected() ) pc->connectRead();
      static int wr  = pool::RECREATE|pool::CREATE|pool::UPDATE;
      // Got a valid connection. Now rearm the transaction
      checkAccessMode(mode, pc->database());
      if ( (mode&wr) != 0 )  {
	pc->transaction().set(pool::Transaction::TRANSACT_START);
	if ( !pc->database().transAct(pc->transaction()).isSuccess() )
	  return StatusCode::FAILURE;
      }
      *con = pc;
      pc->resetAge();
    }
    if ( *con )  {
      if ( fire_incident ) {
	PoolDbAddress* pAddr = 0;
	MsgStream log(msgSvc(), name());
	pool::Token* token = 0, *prev = 0;
	DbDatabase& dbH = pc->database();
	if ( dbH.cntToken(m_recordName) )   {
	  std::auto_ptr<pool::DbSelect> sel(createSelect("*",dbH,m_recordName));
	  if ( sel.get() ) {
	    while(1) {
	      if ( prev ) prev->release();
	      prev = token;
	      if ( prev ) prev->addRef();
	      if ( !sel->next(token).isSuccess() ) break;
	    }
	  }
	  if ( prev ) {
	    log << MSG::INFO << "Records token:" << prev->toString() << endmsg;
	    if ( !createAddress(prev,&pAddr).isSuccess() ) {
	      prev->release();
	    }
	  }
	  else {
	    log << MSG::INFO << "No Record " << m_recordName << " entries present in:" << c->fid() << endmsg;
	  }
	}
	else {
	  log << MSG::INFO << "No Records " << m_recordName << " present in:" << c->fid() << endmsg;
	}
	m_incidentSvc->fireIncident(ContextIncident<IOpaqueAddress*>(c->fid(),"FILE_OPEN_READ",pAddr));
      }
      return StatusCode::SUCCESS;
    }
    m_incidentSvc->fireIncident(Incident(dataset,IncidentType::FailOutputFile));
    return StatusCode::FAILURE;
  }
  catch (std::exception& e)  {
    m_incidentSvc->fireIncident(Incident(dataset,IncidentType::FailOutputFile));
    return error(std::string("connectDatabase> Caught exception:")+e.what(), false);
  }
  catch (...)   {
    m_incidentSvc->fireIncident(Incident(dataset,IncidentType::FailOutputFile));
    return error("connectDatabase> Unknown Fatal Exception for "+dataset, false);
  }
}

StatusCode
PoolDbCnvSvc::connectContainer(int type,
                               CSTR dbName,
                               CSTR cntName,
                               DbAccessMode mode,
                               const DbTypeInfo* info,
                               DbContainer& cntH)
{
  PoolDbDataConnection* c = 0;
  StatusCode sc = connectDatabase(type, dbName, mode, &c);
  if( sc.isSuccess() )  {
    return connectContainer(c->database(), cntName, mode, info, cntH);
  }
  return sc;
}

StatusCode
PoolDbCnvSvc::connectContainer(DbDatabase& dbH,
                               CSTR cntName,
                               DbAccessMode mode,
                               const DbTypeInfo* shapeH,
                               DbContainer& cntH)
{
  std::string pfn, fid;
  int all = pool::READ + pool::CREATE + pool::UPDATE;
  int wr  = pool::CREATE + pool::UPDATE;
  if ( dbH.isValid() )  {
    fid = dbH.token()->dbID();
    pfn = dbH.token()->contID();
    int m   = dbH.openMode();
    if ( (m&all) && mode == pool::READ )    {
    }
    else if ( m&wr && mode&pool::CREATE )   {
    }
    else if ( m&wr && mode&pool::UPDATE )   {
    }
    else  {
      dbH.reopen(pool::UPDATE);
    }
  }
  // No Else!
  if ( !dbH.isValid() )  {
    m_incidentSvc->fireIncident(Incident(pfn,IncidentType::FailOutputFile));
    error("Cannot connect to Database: FID="+fid+" PFN="+pfn+
          " [Invalid database handle]",false);
    return StatusCode::FAILURE;
  }
  cntH = DbContainer(dbH.find(cntName));
  if ( cntH.isValid() )  {
    return StatusCode::SUCCESS;
  }
  if ( mode&pool::READ )  {
    shapeH = dbH.contShape(cntName);
  }
  if ( shapeH )  {
    if ( cntH.open(dbH, cntName, shapeH, m_type, mode).isSuccess() )  {
      return StatusCode::SUCCESS;
    }
    return error("connectContainer> Failed to open container:"+cntName+
      " in "+dbH.name(),false);
  }
  return error("connectContainer> No shape present for container:"+cntName+
    " in "+dbH.name(),false);
}

// Disconnect from an existing data stream.
StatusCode PoolDbCnvSvc::disconnect(CSTR dataset)  {
  IDataConnection* c = m_ioMgr->connection(dataset);
  return c ? m_ioMgr->disconnect(c) : S_FAIL;
}

// Request an iterator over a container from the service
pool::DbSelect* PoolDbCnvSvc::createSelect(CSTR criteria, CSTR db, CSTR cnt) {
  PoolDbDataConnection* c = 0;
  StatusCode sc = connectDatabase(UNKNOWN, db, pool::READ, &c);
  if ( sc.isSuccess() )  {
    return createSelect(criteria,c->database(), cnt);
  }
  error("createSelect> Cannot open database:"+db, false);
  m_incidentSvc->fireIncident(Incident(db,IncidentType::FailInputFile));
  return 0;
}

// Request an iterator over a container from the service
pool::DbSelect* PoolDbCnvSvc::createSelect(CSTR criteria, DbDatabase& dbH, CSTR cnt) {
  // Now select according to criteria
  std::auto_ptr<pool::DbSelect> sel(new pool::DbSelect(criteria));
  if ( sel->start(dbH, cnt).isSuccess() )  {
    return sel.release();
  }
  return 0;
}

/// IAddressCreator implementation: Address creation
StatusCode PoolDbCnvSvc::createAddress( long  typ,
                                        const CLID& clid,
                                        const std::string* par,
                                        const unsigned long* ip,
                                        IOpaqueAddress*& refpAddress)
{
  PoolDbAddress* pA = 0;
  pool::Guid guid = pool::Guid::null();
  std::auto_ptr<pool::Token> tok(new pool::Token());
  guid.Data1 = clid;
  tok->setDb(par[0]);
  tok->setCont(par[1]);
  tok->setTechnology(typ);
  tok->setClassID(guid);
  tok->oid().first = ip[0];
  tok->oid().second = ip[1];
  if ( createAddress(tok.get(), &pA).isSuccess() )  {
    refpAddress = pA;
    tok->release();
    tok.release();
    return S_OK;
  }
  return error("createAddress> Failed to create opaque address.");
}

// Create an opaque address from a POOL token structure
StatusCode PoolDbCnvSvc::createAddress(pool::Token* pTok,PoolDbAddress** ppAdd) {
  if ( pTok )  {
    *ppAdd = new PoolDbAddress(pTok);
    return S_OK;
  }
  return error("createAddress> Failed to create opaque address from POOL token.");
}

// Mark an object for write given an object reference
StatusCode
PoolDbCnvSvc::markWrite(pool::DataCallBack* call,CSTR cntName,PoolDbAddress** ppAddr)
{
  if ( m_current )  {
    DbOptionCallback call_back(m_cacheSvc->callbackHandler());
    const DbTypeInfo* shapeH = (const DbTypeInfo*)call->shape();
    DbDatabase& dbH = m_current->database();
    if ( dbH.transactionActive() )  {
      std::auto_ptr<pool::Token> token(new pool::Token());
      token->setDb(dbH.name()).setCont(cntName);
      token->setClassID(shapeH->shapeID());
      token->setTechnology(m_type);
      DbContainer cntH(POOL_StorageType);
      StatusCode sc = connectContainer(dbH,cntName,pool::UPDATE|pool::CREATE,shapeH,cntH);
      if ( sc.isSuccess() )  {
        call->setHandler(&m_handler);
        if ( cntH.allocate(call, token->oid()).isSuccess() )  {
          if ( dbH.makeLink(token.get(), token->oid()).isSuccess() )  {
            if ( ppAddr )  {
              if ( *ppAddr ) {
                (*ppAddr)->token()->setData(token.get());
      	      }
              else {
                *ppAddr = new PoolDbAddress(token.get());
	      }
            }
            long cnt = token->release();
            if ( cnt > 1 ) {
              MsgStream log(msgSvc(), name());
              log << MSG::ERROR << "markWrite> Token count wrong: "
                  << cnt << " expect memory leak. " << endmsg
                  << "run again with environment POOLDB_TRACE=ON and"
                  << " check instance counts." << endmsg;
            }
            token.release();
            return S_OK;
          }
        }
      }
    }
    return error("markWrite> No active transaction present for "+dbH.name());
  }
  return error("markWrite> Current Database is invalid!");
}

// Mark an object for update
StatusCode PoolDbCnvSvc::markUpdate(pool::DataCallBack* call, PoolDbAddress* pA)  {
  if ( pA && call )  {
    pool::Token* tok = pA->token();
    DbContainer cntH(POOL_StorageType);
    const DbTypeInfo* info = (const DbTypeInfo*)call->shape();
    StatusCode sc = connectContainer(FID,tok->dbID(),tok->contID(),pool::UPDATE,info,cntH);
    if ( sc.isSuccess() )  {
      call->setHandler(&m_handler);
      if ( cntH.update(call, tok->oid()).isSuccess() )  {
        return S_OK;
      }
      return error("markUpdate> Failed to update object:"+tok->toString());
    }
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "markUpdate> The container "
        << tok->dbID() << "::" << tok->contID()
        << " cannot be accessed." << endmsg
        << "Was this object really read from the container "
        << "the object token claims?"
        << endmsg;
    return S_FAIL;
  }
  return error("createRep> Invalid object address for update.");
}

/// Read existing object. Open transaction in read mode if not active
StatusCode PoolDbCnvSvc::read(pool::DataCallBack* call, PoolDbAddress* pA)  {
  StatusCode sc = read(call, *pA->token());
  if ( sc.isSuccess() || sc == BAD_DATA_CONNECTION )  {
    return sc;
  }
  std::string err="read> Cannot read object:"+pA->registry()->identifier()+" ";
  return error(err, false);
}

/// Read existing object. Open transaction in read mode if not active
StatusCode PoolDbCnvSvc::read(pool::DataCallBack* call, pool::Token& tok)  {
  try  {
    DbContainer cntH(tok.technology());
    const DbTypeInfo* info = (const DbTypeInfo*)call->shape();
    StatusCode sc = connectContainer(FID,tok.dbID(),tok.contID(),pool::READ,info,cntH);
    if ( sc.isSuccess() )  {
      typedef pool::DbObjectHandle<pool::DbObject> ObjH;
      call->setHandler(&m_handler);
      return ObjH::openEx(cntH, tok, call, pool::READ).isSuccess() ? S_OK : S_FAIL;
    }
    else if ( sc == BAD_DATA_CONNECTION )  {
      return sc;
    }
  }
  catch (std::exception& e)  {
    std::string err="read> Cannot read object:"+tok.toString()+" ";
    return error(err+e.what(), false);
  }
  catch (...)   {
    std::string err = "read> Cannot read object:"+tok.toString();
    return error(err+" [Unknown fatal exception]", false);
  }
  return error("read> Cannot read object:"+tok.toString(), false);
}

// Small routine to issue exceptions
StatusCode PoolDbCnvSvc::error(CSTR msg, bool rethrow)  {
  if ( rethrow )  {
    pool::debugBreak("PoolDbCnvSvc", "Error:"+msg, rethrow);
  }
  else  {
    MsgStream log(msgSvc(),name());
    log << MSG::ERROR << "Error: " << msg << endmsg;
  }
  return S_FAIL;
}
