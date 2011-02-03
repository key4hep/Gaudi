// $Id: PoolDbBaseCnv.cpp,v 1.19 2008/10/27 16:41:33 marcocle Exp $
//====================================================================
//	PoolDbBaseCnv implementation
//--------------------------------------------------------------------
//
//  Package    : GaudiPoolDb ( The LHCb Offline System)
//
//  Description: Generic Db data converter
//
//	Author     : M.Frank
//====================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, coming from CORAL
#pragma warning(disable:2259)
#endif

// FIXME: missing in CORAL
#include <algorithm>

// Framework include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "GaudiPoolDb/IPoolDbMgr.h"
#include "GaudiPoolDb/PoolDbBaseCnv.h"
#include "GaudiPoolDb/PoolDbAddress.h"
//#include "GaudiPoolDb/PoolDbCnvFactory.h"
#include "GaudiKernel/CnvFactory.h"
#include "GaudiPoolDb/PoolDbLinkManager.h"

#include "PersistencySvc/IPersistencySvc.h"
#include "PersistencySvc/IDataTransform.h"
#include "StorageSvc/DbObjectCallBack.h"
#include "StorageSvc/DbInstanceCount.h"
#include "StorageSvc/DbTransform.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbReflex.h"
#include "Reflex/Reflex.h"
#include "POOLCore/Token.h"

#include <stdexcept>

static pool::DbInstanceCount::Counter* s_count =
  pool::DbInstanceCount::getCounter(typeid(PoolDbBaseCnv));


PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv,
                           ConverterID(POOL_StorageType,CLID_Any),
                           IConverter*(long, CLID, ISvcLocator*) )
PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv,
                           ConverterID(POOL_StorageType,CLID_Any+CLID_ObjectList),
                           IConverter*(long, CLID, ISvcLocator*) )
PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv,
                           ConverterID(POOL_StorageType,CLID_Any+CLID_ObjectVector),
                           IConverter*(long, CLID, ISvcLocator*) )

PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv, 
                           ConverterID(POOL_StorageType,CLID_Any + CLID_ObjectVector+0x00030000),
                           IConverter*(long, CLID, ISvcLocator*) )
PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv, 
                           ConverterID(POOL_StorageType,CLID_Any + CLID_ObjectVector+0x00040000),
                           IConverter*(long, CLID, ISvcLocator*) )
PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv, 
                           ConverterID(POOL_StorageType,CLID_Any + CLID_ObjectVector+0x00050000),
                           IConverter*(long, CLID, ISvcLocator*) )
PLUGINSVC_FACTORY_WITH_ID( PoolDbBaseCnv, 
                           ConverterID(POOL_StorageType,CLID_Any | (1<<31)),
                           IConverter*(long, CLID, ISvcLocator*) )


class PoolDbObjectContext;
static PoolDbObjectContext* s_context = 0;

class PoolDbObjectContext  {
  PoolDbBaseCnv* m_converter;
  IRegistry*     m_registry;
public:
  PoolDbObjectContext(PoolDbBaseCnv* dm, IRegistry* pReg)  {
    m_converter = dm;
    m_registry = pReg;
    s_context = this;
  }
  ~PoolDbObjectContext()  {
    m_converter = 0;
    s_context = 0;
  }
  PoolDbBaseCnv* cnv()  {
    return m_converter;
  }
  IRegistry* registry()  {
    return m_registry;
  }
};


class PoolDbDataObjectHandler : public pool::DbObjectCallBack  {
protected:
  LinkManager*       m_links;
  PoolDbLinkManager* m_refs;
  PoolDbBaseCnv*     m_converter;
  IRegistry*         m_registry;
public:
  /// Initializing constructor
   PoolDbDataObjectHandler(const ROOT::Reflex::Type& cl)
  : pool::DbObjectCallBack(cl), m_links(0), m_refs(0),
    m_converter(0), m_registry(0)
  {
  }
  /// Initializing constructor
  PoolDbDataObjectHandler(const ROOT::Reflex::Type& cl,
                          PoolDbBaseCnv* dm,
                          IRegistry* pReg)
  : pool::DbObjectCallBack(cl), m_links(0), m_refs(0),
    m_converter(dm), m_registry(pReg)
  {
  }
  /// Standard destructor
  virtual ~PoolDbDataObjectHandler() {
  }
  /// Virtual copy
  virtual pool::DataCallBack* clone() const {
    if ( s_context )  {
      PoolDbDataObjectHandler* p =
        new PoolDbDataObjectHandler( transientType(),
                                     s_context->cnv(),
                                     s_context->registry());
      p->setShape(shape());
      return p;
    }
    PoolDbDataObjectHandler* p =
           new PoolDbDataObjectHandler(transientType(),0,0);
    p->setShape(shape());
    return p;
  }
  /// Callback when a read/write sequence should be started
  /** @param action_type    [IN] Action type: PUT/GET
    * @param data           [IN] Reference to persistent data location (on Write)
    * @param context        [IN] Data context for subsequent calls
    *
    * @return DbStatus indicating success or failure.
    */
  pool::DbStatus start(pool::DataCallBack::CallType action_type,
                       void* data,
			              void** context);
  /// Callback to retrieve the absolute address of a column
  pool::DbStatus bind(pool::DataCallBack::CallType action_type,
 	                   const pool::DbColumn* col_ident,
	                   int          col_number,
	                   void*        context,
	                   void**       data_pointer);
  /// Callback when a read/write sequence should is finished
  pool::DbStatus end(pool::DataCallBack::CallType action_type, void* context );
};

void popCurrentDataObject();
void pushCurrentDataObject(DataObject** pobjAddr);

namespace {
  static bool printLinks()  {
    static bool prt = false;
    return prt;
  }
}

/// Callback to retrieve the absolute address of a column
pool::DbStatus
PoolDbDataObjectHandler::bind(pool::DataCallBack::CallType action_type,
                        const pool::DbColumn* /* col_ident  */,
                        int                      col_number      ,
                        void*                  /* context */,
                        void**                    data_pointer)
{
  DataObject* pObj = (DataObject*)m_objectData;
  switch(action_type)  {
  case GET:
    switch(col_number) {
    case 0:
      *data_pointer = &m_objectData;
      return pool::Success;
    case 1:
      *data_pointer = &m_links;
      return pool::Success;
    case 2:
      *data_pointer = &m_refs;
      return pool::Success;
    }
    break;
  case PUT:
    switch(col_number) {
    case 0:
      *data_pointer = pObj;
      return pool::Success;
    case 1:
      *data_pointer = pObj->linkMgr();
      return pool::Success;
    case 2:
      m_refs = m_converter->createReferences(pObj);
      *data_pointer = m_refs;
      return pool::Success;
    }
    break;
  default:
    break;
  }
  return pool::Error;
}

/// Callback when a read sequence should be started
pool::DbStatus
PoolDbDataObjectHandler::end(pool::DataCallBack::CallType action_type, void* /* context */)
{
  DataObject* pObj = (DataObject*)m_objectData;
  switch(action_type)  {
  case GET:
    m_converter->setReferences(m_refs, m_links, pObj, m_registry).ignore();
    pool::deletePtr(m_links);
    break;
  case PUT:
    m_converter->dumpReferences(m_refs, m_links, pObj, m_registry).ignore();
    break;
  }
  pool::deletePtr(m_refs);
  popCurrentDataObject();
  return pool::Success;
}

pool::DbStatus
PoolDbDataObjectHandler::start( pool::DataCallBack::CallType action_type,
                                void*                    /* data */,
                                void**                   context)
{
  m_refs = 0;
  *context = 0;
  switch(action_type)  {
  case GET:
    m_objectData = 0;
    break;
  case PUT:
    break;
  }
  void **ptr = const_cast<void**>(&m_objectData);
  pushCurrentDataObject((DataObject**)ptr);
  return pool::Success;
}

/// Standard Constructor
PoolDbBaseCnv::PoolDbBaseCnv(long typ, const CLID& clid, ISvcLocator* svc)
  : Converter(typ, clid, svc), m_dbMgr(0), m_dataMgr(0), 
    m_objGuid(pool::Guid::null()), m_call(0)
{
  s_count->increment();
  m_objGuid.Data1 = clid;
}

/// Standard Destructor
PoolDbBaseCnv::~PoolDbBaseCnv()  {
  pool::releasePtr(m_dbMgr);
  s_count->decrement();
}

/// Initialize the converter
StatusCode PoolDbBaseCnv::initialize()   {
  StatusCode status = Converter::initialize();
  MsgStream log(msgSvc(),"PoolDbBaseCnv");
  if ( !status.isSuccess() )   {
    log << MSG::ERROR << "Cannot initialize base class \"Converter\"" << endmsg;
    return status;
  }
  IPoolDbMgr **ptr1 = &m_dbMgr;
  status = conversionSvc()->queryInterface(IPoolDbMgr::interfaceID(), (void**)ptr1);
  if ( !status.isSuccess() )    {
    log << MSG::ERROR << "Cannot connect to \"IPoolDbMgr\" interface." << endmsg;
    return status;
  }
  IDataManagerSvc **ptr2 = &m_dataMgr;
  status = dataProvider()->queryInterface(IDataManagerSvc::interfaceID(), (void**)ptr2);
  if ( !status.isSuccess() )    {
    log << MSG::ERROR << "Cannot connect to \"IDataManagerSvc\" interface." << endmsg;
    return status;
  }
  const pool::DbTypeInfo* shapeH = 0;
  const ROOT::Reflex::Type refH = ROOT::Reflex::Type::ByName("PoolDbLinkManager");
  if ( !refH )  {
    log << MSG::ERROR << "Dictionary for class \"PoolDbLinkManager\" missing." << endmsg;
    return StatusCode::FAILURE;
  }
  const ROOT::Reflex::Type lnkH = ROOT::Reflex::Type::ByName("LinkManager");
  if ( !lnkH )  {
    log << MSG::ERROR << "Dictionary for class \"LinkManager\" missing." << endmsg;
    return StatusCode::FAILURE;
  }
  m_class = pool::DbReflex::forGuid(m_objGuid);
  if ( !m_class )  {
    // This may fail if the dictionary is not yet loaded, therefore it is not going to report an ERROR
    log << MSG::DEBUG << "Dictionary for class with GUID:" << m_objGuid.toString()
        << " missing." << endmsg;
    return StatusCode::FAILURE;
  }
  if ( !pool::DbTransform::getShape(m_objGuid, shapeH).isSuccess() )  {
    std::vector<const pool::DbColumn*> c;
    c.push_back(new pool::DbColumn(m_class.Name(),
                m_class.Name(ROOT::Reflex::SCOPED),
                pool::DbColumn::POINTER,
                0));
    c.push_back(new pool::DbColumn("Links",
                lnkH.Name(ROOT::Reflex::SCOPED),
                pool::DbColumn::POINTER,
                0));
    c.push_back(new pool::DbColumn("Refs",
                refH.Name(ROOT::Reflex::SCOPED),
                pool::DbColumn::POINTER,
                0));
    shapeH = pool::DbTypeInfo::create(m_objGuid, c);
  }
  if ( shapeH ) {
    log << MSG::DEBUG << "Created object shape for class:"
        << m_class.Name(ROOT::Reflex::SCOPED) << endmsg
        << shapeH->toString() << endmsg;
    shapeH->addRef();
    m_call = new PoolDbDataObjectHandler(m_class);
    m_call->setShape(shapeH);
  }
  else  {
    return makeError("Failed to create POOL shape information for GUID:"+
                      m_objGuid.toString(),false);
  }
  return status;
}

/// Finalize the Db converter
StatusCode PoolDbBaseCnv::finalize()   {
  pool::DbTypeInfo* shapeH = 0;
  if ( m_call ) {
    shapeH = dynamic_cast<pool::DbTypeInfo*>((pool::Shape*)m_call->shape());
    if ( shapeH ) shapeH->deleteRef();
  }
  pool::releasePtr(m_call);
  pool::releasePtr(m_dbMgr);
  pool::releasePtr(m_dataMgr);
  return Converter::finalize();
}

/// Retrieve the name of the container a given object is placed into
const std::string
PoolDbBaseCnv::containerName(IRegistry* pRegistry)  const {
  if ( 0 != pRegistry )   {
    return pRegistry->identifier();
  }
  return "";
}

StatusCode PoolDbBaseCnv::makeError(const std::string& msg, bool rethrow)
{
  MsgStream log(msgSvc(),"PoolDbBaseCnv");
  log << MSG::ERROR << "Trouble with class:" << m_objGuid.toString() << " ";
  if ( m_class ) log << "<" << m_class.Name(ROOT::Reflex::SCOPED) << "> ";
  log << endmsg;
  log << msg << endmsg;
  if ( rethrow )  {
    pool::debugBreak("Error:"+msg, "PoolDbBaseCnv", true);
  }
  return StatusCode::FAILURE;
}

StatusCode
PoolDbBaseCnv::dumpReferences(PoolDbLinkManager* /* mgr */,
                              LinkManager*       /* pLinks */,
                              DataObject*           pObj,
                              IRegistry*         /* pReg  */)
{
  if ( printLinks() )  {
    MsgStream log(msgSvc(), "dumpReferences");
    LinkManager* mgr = pObj->linkMgr();
    for ( int i = 0; i < mgr->size(); ++i)  {
      LinkManager::Link* lnk = mgr->link(i);
      log << MSG::ALWAYS << "PUT> " << pObj->registry()->identifier() << "[" << i << "] = " << lnk->path() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
PoolDbBaseCnv::setReferences(PoolDbLinkManager* mgr,
                             LinkManager* pLinks,
                             DataObject* pObj,
                             IRegistry* pReg)
{
  std::string tmp;
  LinkManager* pmgr = pObj->linkMgr();
  for(int i = 0; i < pLinks->size(); ++i)  {
    LinkManager::Link* lnk = pLinks->link(i);
    pmgr->addLink(lnk->path(), 0);
  }
  size_t ref_size  = mgr->references().size();
  for(size_t j = 0; j < ref_size; ++j) {
    IOpaqueAddress* pA = 0;
    const pool::Token* token = mgr->references()[j];
    std::string& location = mgr->links()[j];
    long typ = token->technology();
    CLID clid = token->classID().Data1;
    std::string spar[]   = {token->dbID(),
                            token->contID()};
    unsigned long ipar[] = {token->oid().first,
                            token->oid().second};
    StatusCode sc = addressCreator()->createAddress(typ,clid,spar,ipar,pA);
    if ( sc.isSuccess() )  {
      // Temporary fix for all files written with previous release(s)
      // The fix should be removed when these files are obsolete
      if ( location.substr(0,7) == "/Event/" )  {
        size_t idx = location.rfind("/");
        tmp = (idx==std::string::npos) ? location : location.substr(idx);
        location = tmp;
      }
      sc = m_dataMgr->registerAddress(pReg, location, pA);
      if ( sc.isSuccess() )  {
        // MsgStream log(msgSvc(), "Reflection");
        // log << MSG::ALWAYS << "Registered:" << pReg->identifier() << " as " << location << endmsg;
        continue;
      }
      pA->release();
      ROOT::Reflex::Type classH = pool::DbReflex::forGuid(token->classID());
      MsgStream err(msgSvc(), "Reflection");
      if ( 0 == classH ) {
        err << MSG::ERROR << "No reflection information availible for class ID:"
            << token->classID().toString() << endmsg;
      }
      else {
        err << MSG::ERROR << "Reflection information availible for class ID:"
            << token->classID().toString() << endmsg
            << classH.Name(ROOT::Reflex::SCOPED) << endmsg;
      }
      makeError("setReferences> Failed to register opaque address:"+location+
                "\n"+token->toString());
    }
  }
  return StatusCode::SUCCESS;
}

PoolDbLinkManager*
PoolDbBaseCnv::createReferences(DataObject* pObj)
{
  typedef std::vector<IRegistry*> Leaves;
  PoolDbLinkManager* refs = new PoolDbLinkManager;
  Leaves leaves;
  StatusCode status = m_dataMgr->objectLeaves(pObj, leaves);
  if ( status.isSuccess() )  {
    for(Leaves::iterator i=leaves.begin(), iend=leaves.end(); i != iend; ++i)  {
      IRegistry* pR = *i;
      IOpaqueAddress* pA = pR->address();
      if ( 0 != pA )  {
        PoolDbAddress*  pdbA = dynamic_cast<PoolDbAddress*>(pA);
        if ( 0 != pdbA )  {
          pool::Token* tok = const_cast<pool::Token*>(pdbA->token());
          if ( tok )  {
            tok->addRef();
            refs->references().push_back(tok);
            refs->links().push_back(pR->name());
          }
          else  {
            // What should we do here ?
            // Not so clear...
          }
        }
        else  {
          const std::string*   spar = pA->par();
          const unsigned long* ipar = pA->ipar();
          pool::Guid guid(pool::Guid::null());
          guid.Data1 = pA->clID();
          pool::Token* tok = new pool::Token();
          tok->setClassID(guid);
          tok->setTechnology(pA->svcType());
          tok->setDb(spar[0]).setCont(spar[1]);
          tok->oid().first  = ipar[0];
          tok->oid().second = ipar[1];
          refs->references().push_back(tok);
          refs->links().push_back(pR->name());
        }
      }
      else    {
        // This is not really an error, it only means that the object
        // pointed to is not persistent. Hence, this link cannot be followed
        // from the data written.
      }
    }
  }
  return refs;
}


/// Db specific implementation: Create an empty transient object: Default: DataObject
StatusCode
PoolDbBaseCnv::createObj(IOpaqueAddress* pAddress, DataObject*& refpObject)
{
  try   {
    PoolDbAddress* pA = dynamic_cast<PoolDbAddress*>(pAddress);
    if ( pA )  {
      PoolDbObjectContext ctxt(this, pAddress->registry());
      pool::DataCallBack* call = m_call->clone();
      StatusCode sc = m_dbMgr->read(call, pA);
      if ( sc.isSuccess() )  {
        refpObject = (DataObject*)call->object();
        call->release();
        return StatusCode::SUCCESS;
      }
      call->release();
      std::string tag = pA->par()[0]+":"+pA->par()[1];
      if ( sc.getCode() == IPoolDbMgr::BAD_DATA_CONNECTION )  {
        if ( m_badFiles.find(tag) != m_badFiles.end() ) {
          return sc;  // Let's save us the additional printout
        }
        m_badFiles.insert(tag);
      }
      MsgStream log(msgSvc(),"PoolDbBaseCnv");
      log << MSG::ERROR << "createObj> Cannot access the object:"+tag << endmsg;
      return StatusCode::FAILURE;
    }
    return makeError("createObj> Invalid object address.", false);
  }
  catch (std::exception& e)  {
    return makeError(std::string("createObj> Caught exception:")+e.what(), false);
  }
  catch(...)  {
  }
  return makeError("createObj> Unknown exception occurred.", false);
}

/// Resolve the references of the created transient object.
StatusCode
PoolDbBaseCnv::fillObjRefs(IOpaqueAddress* pAddress, DataObject* pObject)
{
  return updateObjRefs(pAddress, pObject);
}

/// Db specific implementation: Update a transient object: Default: DataObject
StatusCode
PoolDbBaseCnv::updateObj(IOpaqueAddress* pAddr, DataObject* /* pObj */)
{
  try  {
    PoolDbAddress* pA = dynamic_cast<PoolDbAddress*>(pAddr);
    if ( 0 != pA ) {
      return makeError("updateObj> Sorry folks, not yet implemented...");
    }
    return makeError("updateObj> Invalid opaque object address.");
  }
  catch (std::exception& e)  {
    return makeError(std::string("updateObj> Caught exception:")+e.what());
  }
  catch (...)  {
  }
  return makeError("updateObj> Unknown Error - exception was thrown.");
}

/// Update the references of an updated transient object.
StatusCode
PoolDbBaseCnv::updateObjRefs(IOpaqueAddress* /*pAddr*/, DataObject* pObj)
{
  if ( 0 != pObj )  {
    // First call the reconfiguration callback
    StatusCode sc = pObj->update();
    if ( sc.isSuccess() && printLinks() )  {
      MsgStream log(msgSvc(), "updateObjRefs");
      LinkManager* mgr = pObj->linkMgr();
      std::string id = pObj->registry()->identifier();
      for ( int i = 0; i < mgr->size(); ++i)  {
        LinkManager::Link* lnk = mgr->link(i);
        log << MSG::ALWAYS << "GET> " << id << "[" << i << "] = " << lnk->path() << endmsg;
      }
    }
    return sc;
  }
  return makeError("updateObjRefs> Invalid object reference.");
}

/// Create persistent representation of the object
StatusCode
PoolDbBaseCnv::createRep(DataObject* pObj, IOpaqueAddress*& refpA)
{
  try   {
    refpA = 0;
    if ( pObj )  {
      IRegistry* pReg = pObj->registry();
      if ( pReg )  {
        PoolDbAddress* pA = 0;
        std::string cnt = containerName(pReg);
        PoolDbObjectContext ctxt(this, pObj->registry());
        pool::DataCallBack* call = m_call->clone();
        call->setObject(pObj);
        StatusCode sc = m_dbMgr->markWrite(call, cnt, &pA);
        if ( sc.isSuccess() )  {
          refpA = pA;
          return sc;
        }
      }
      return makeError("createRep> Invalid object registry entry.");
    }
    return makeError("createRep> Invalid object reference.");
  }
  catch (std::exception& e)  {
    makeError(std::string("createRep> Caught exception:")+e.what());
  }
  catch (...)   {
    return makeError("createRep> Unknown Fatal Exception.");
  }
  return makeError("createRep> Fatal Error.");
}

/// Update the converted representation of a transient object.
StatusCode
PoolDbBaseCnv::updateRep(IOpaqueAddress* pAddr, DataObject* pObj)  {
  try   {
    PoolDbAddress* pA = dynamic_cast<PoolDbAddress*>(pAddr);
    if ( 0 != pA )  {
      if ( pObj )  {
        PoolDbObjectContext ctxt(this, pAddr->registry());
        pool::DataCallBack* call = m_call->clone();
        call->setObject(pObj);
        return m_dbMgr->markUpdate(call, pA);
      }
      return makeError("updateRep> Invalid object reference.");
    }
    return makeError("updateRep> Invalid opaque object address.");
  }
  catch (std::exception& e)  {
    makeError(std::string("updateRep> Caught exception:")+e.what());
  }
  catch (...)   {
    return makeError("updateRep> Unknown Fatal Exception.");
  }
  return makeError("updateRep> Fatal Error.");
}

/// Resolve the references of the converted object.
StatusCode
PoolDbBaseCnv::fillRepRefs(IOpaqueAddress* pAddr, DataObject* pObj)
{
  return updateRepRefs(pAddr, pObj);
}

/// Update the references of an already converted object.
StatusCode
PoolDbBaseCnv::updateRepRefs(IOpaqueAddress* /* pAddr */, DataObject* pObj)
{
  if ( pObj ) {
    return StatusCode::SUCCESS;
  }
  return makeError("updateRepRefs> Invalid object reference.");
}
