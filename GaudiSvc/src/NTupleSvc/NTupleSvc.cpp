//====================================================================
//  NTupleSvc.cpp
//--------------------------------------------------------------------
//
//  Package   : GaudiSvc/NTupleSvc ( The LHCb Offline System)
//
//  Description: implementation of the NTuple service
//
//  Author    : M.Frank
//  History   :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// | 29/09/99| Added access to ICnvManager for missing      |
// |         | converters                                   | MF
// | 20/09/00| Connect dynamically to conversion service    |
// |         | for N-tuple persistency                      | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  GAUDISVC_NTUPLESVC_CPP

// Framework include files
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/GenericAddress.h"

#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataSelector.h"

#include "GaudiKernel/Property.h"
#include "GaudiKernel/Selector.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/DataSelectionAgent.h"
#include "GaudiKernel/NTupleImplementation.h"

#include "NTupleSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(NTupleSvc)

/// Selector factory instantiation
DECLARE_NAMESPACE_OBJECT_FACTORY(NTuple,Selector)

#include <sstream>

namespace {
  inline std::string toStr(long id) {
    std::ostringstream s; s << id;
    return s.str();
  }
}

/// Standard Constructor
NTupleSvc::NTupleSvc(const std::string& name, ISvcLocator* svc)
 : base_class(name, svc)
{
  declareProperty("Input",  m_input);
  declareProperty("Output", m_output);
  m_rootName = "/NTUPLES";
  m_rootCLID = CLID_DataObject;
}

/// Standard Destructor
NTupleSvc::~NTupleSvc()   {
}

/// Initialize the service.
StatusCode NTupleSvc::initialize()     {
  StatusCode status = DataSvc::initialize();
  if ( status.isSuccess() )   {
    status = setProperties();
    if ( status.isSuccess() )   {
      StatusCode iret(StatusCode::SUCCESS,true);
      DataObject* root = new NTuple::Directory();
      status = setRoot(m_rootName, root);
      for ( DBaseEntries::iterator i = m_output.begin(); i != m_output.end(); ++i )    {
        iret = connect(*i);
        if ( !iret.isSuccess() )    {
          status = iret;
        }
      }
      for ( DBaseEntries::iterator j = m_input.begin(); j != m_input.end(); ++j )    {
        iret = connect(*j);
        if ( !iret.isSuccess() )    {
          status = iret;
        }
      }
    }
  }
  return status;
}

/// Initialize the service.
StatusCode NTupleSvc::reinitialize() {
  return StatusCode::SUCCESS;
}

// Check if a datasource is connected
bool NTupleSvc::isConnected(const std::string& identifier) const    {
  Connections::const_iterator i = m_connections.find(identifier);
  return !(i==m_connections.end());
}

/// DataSvc override: Retrieve data loader
IConversionSvc* NTupleSvc::getDataLoader(IRegistry* pRegistry)    {
  if ( 0 != pRegistry )    {
    std::string full = pRegistry->identifier();
    size_t len = m_rootName.length();
    size_t idx = full.find(SEPARATOR,len+1);
    std::string path = (idx==std::string::npos) ? full : full.substr(0, idx);
    Connections::iterator i = m_connections.find(path);
    if ( i != m_connections.end() )   {
      return (*i).second.service;
    }
  }
  return 0;
}

StatusCode NTupleSvc::updateDirectories()   {
  typedef std::vector<IRegistry*> Leaves;
  long need_update = 0;
  DataObject* pO = 0;
  StatusCode iret = findObject(m_rootName, pO);
  MsgStream log ( msgSvc(), name() );
  //  log << MSG::DEBUG << "in finalize()" << endmsg;
  if ( iret.isSuccess() )   {
    Leaves leaves;
    iret = objectLeaves(pO, leaves);
    if ( iret.isSuccess() )    {
      // Only traverse the tree below the files
      for ( Leaves::iterator d = leaves.begin(); d != leaves.end(); d++ )    {
        if ( (*d)->object() )   {
          IOpaqueAddress* pA = (*d)->address();
          if ( pA )   {
            unsigned long typ = pA->ipar()[1];
            if ( typ == 'R' || typ == 'N' || typ == 'U' )   {
              // ...starting from the file entries: first save the directories/ntuples
              IConversionSvc* svc = getDataLoader(*d);
              if ( 0 != svc )   {
                StatusCode status;
                DataSelectionAgent agent;
                IDataSelector*     sel = agent.selectedObjects();
                traverseSubTree ( (*d)->object(), &agent ).ignore();
                for(int i = sel->size()-1; i >= 0; i-- )    {
                  DataObject* o = (*sel)[i];
                  IRegistry*  r = o->registry();
                  status = svc->updateRep(r->address(), o);
                  if ( !status.isSuccess() )    {
                    iret = status;
                  }
                }
                for(int j = sel->size()-1; j >= 0; j-- )    {
                  DataObject* o = (*sel)[j];
                  IRegistry*  r = o->registry();
                  status = svc->updateRepRefs(r->address(), o);
                  if ( !status.isSuccess() )    {
                    iret = status;
                  }
                }
                if ( iret.isSuccess() ) need_update += sel->size();
              }
            }
          }
        }
      }
    }
  }
  if ( !iret.isSuccess() )    {
    log << MSG::ERROR << "ERROR while saving NTuples" << endmsg;
    return iret;
  }
  else if ( need_update > 0 )    {
    log << MSG::INFO << "NTuples saved successfully" << endmsg;
  }
  return iret;
}

// Finalize single service
void NTupleSvc::releaseConnection(Connection& c)  {
  SmartIF<IService> isvc( c.service );
  if ( isvc.isValid( ) )   {
    isvc->finalize().ignore();
  }
  c.service->release();
  c.service = 0;
}

// Close all open connections
StatusCode NTupleSvc::disconnect(const std::string& nam)      {
  Connections::iterator i = m_connections.find(nam);
  if ( i != m_connections.end() )    {
    releaseConnection((*i).second);
    m_connections.erase(i);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

// Close all open connections
StatusCode NTupleSvc::disconnectAll()      {
  for(Connections::iterator i = m_connections.begin(); i != m_connections.end(); ++i) {
    releaseConnection((*i).second);
  }
  m_connections.erase(m_connections.begin(), m_connections.end());
  return StatusCode::SUCCESS;
}

/// stop the service.
StatusCode NTupleSvc::finalize()      {
  StatusCode status = updateDirectories();
  status = clearStore();
  status = DataSvc::finalize();
  status = disconnectAll();
  return status;
}

StatusCode NTupleSvc::connect(const std::string& ident)    {
  std::string logName;
  return connect(ident, logName);
}

StatusCode NTupleSvc::connect(const std::string& ident, std::string& logname)    {
  MsgStream log ( msgSvc(), name() );
  DataObject* pO = 0;
  StatusCode status = findObject(m_rootName, pO);
  if ( status.isSuccess() )   {
    char typ=0;
    Tokenizer tok(true);
    std::vector<Prop> props;
    long loc = ident.find(" ");
    std::string filename, auth, svc = "", db_typ = "";
    logname = ident.substr(0,loc);
    tok.analyse(ident.substr(loc+1,ident.length()), " ", "", "", "=", "'", "'");
    for ( Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); ++i)    {
      const std::string& tag = (*i).tag();
      switch( ::toupper(tag[0]) )   {
      case 'A':
        break;
      case 'F':   // FILE='<file name>'
      case 'D':   // DATAFILE='<file name>'
        filename = (*i).value();
        break;
      case 'O':   // OPT='<NEW<CREATE,WRITE>, UPDATE, READ>'
        switch( ::toupper((*i).value()[0]) )   {
        case 'C':
        case 'N':
        case 'W':
          typ = 'N';
          break;
        case 'U':
          typ = 'U';
          break;
        case 'O':
        case 'R':
          typ = 'O';
          break;
        default:
          typ = 0;
          break;
        }
        break;
      case 'T':   // TYP='<HBOOK,ROOT,OBJY,...>'
        db_typ = (*i).value();
        break;
      default:
        props.push_back( Prop((*i).tag(), (*i).value()));
        break;
      }
    }
    if ( 0 != typ )    {
      IConversionSvc* pSvc = 0;
      status = createService(name()+'.'+logname, db_typ, props, pSvc);
      if ( status.isSuccess() )   {
        status = attachTuple(filename, logname, typ, pSvc->repSvcType());
        if ( status.isSuccess() )    {
          m_connections.insert(Connections::value_type(m_rootName+'/'+logname,Connection(pSvc)));
          return StatusCode::SUCCESS;
        }
      }
    }
  }
  log << MSG::ERROR << "Cannot add " << ident << " invalid filename!" << endmsg;
  return StatusCode::FAILURE;
}

StatusCode NTupleSvc::createService(const std::string&       /* nam */,
                                    const std::string&          typ,
                                    const std::vector<Prop>& /* props */,
                                    IConversionSvc*&            pSvc)
{
  MsgStream log ( msgSvc(), name() );
  /// CGL: set the storage type
  // Get the value of the Stat persistancy mechanism from the AppMgr
  IProperty*   appPropMgr = 0;
  StatusCode sts = serviceLocator()->queryInterface(IProperty::interfaceID(), pp_cast<void>(&appPropMgr) );
  if( !sts.isSuccess() ) {
   // Report an error and return the FAILURE status code
   log << MSG::ERROR << "Could not get PropMgr" << endmsg;
   return sts;
  }

  StringProperty sp("HistogramPersistency","");
  sts = appPropMgr->getProperty( &sp );
  if ( !sts.isSuccess() ) {
   log << MSG::ERROR << "Could not get NTuple Persistency format"
       << " from ApplicationMgr properties" << endmsg;
   return sts;
  }

  long storage_typ = TEST_StorageType;
  if ( sp.value() == "HBOOK" ) {
    storage_typ = HBOOK_StorageType;
  }
  else if ( sp.value() == "ROOT" ) {
    storage_typ = ROOT_StorageType;
  }
  else {
    appPropMgr->release();
    log << MSG::ERROR << "Unknown NTuple Persistency format: " << sp.value() << endmsg;
    return StatusCode::FAILURE;
  }
  // Clean up
  appPropMgr->release();

  if ( typ.length() > 0 && typ != sp.value() )    {
    log << MSG::WARNING << "NTuple persistency type is "
        << sp.value() << "." << endmsg
        << "Type given by job option "
        << "NTupleSvc.Input/Output ignored!" << endmsg;
  }

  //      log << MSG::DEBUG << "storage type: " << m_storageType << endmsg;

  // FIXME: (MCl) why NTupleSvc has to directly create a ConversionSvc?
  IService* pService = 0;
  IInterface* iface = new ConversionSvc(name()+"Conversions", serviceLocator(), storage_typ);
  StatusCode status = iface->queryInterface(IService::interfaceID(), pp_cast<void>(&pService));
  if ( status.isSuccess() )   {
    status = iface->queryInterface(IConversionSvc::interfaceID(), pp_cast<void>(&pSvc));
    if ( !status.isSuccess() )   {
      pService->release();
      return status;
    }
  }
  status = pService->sysInitialize();
  if ( !status.isSuccess() )    {
    return status;
  }
  pService->release();
  status = pSvc->setDataProvider(this);
  if ( !status.isSuccess() )    {
    return status;
  }
  return status;
}

/// Create requested N tuple (Hide constructor)
StatusCode NTupleSvc::create(const CLID& typ, const std::string& title, NTuple::Tuple*& refpTuple)     {
  NTuple::TupleImp* pTuple = 0;
  StatusCode status = StatusCode::FAILURE;
  if ( typ == CLID_ColumnWiseTuple )    {
    pTuple = new NTuple::ColumnWiseTuple( title );
  }
  else if ( typ == CLID_RowWiseTuple )    {
    pTuple = new NTuple::RowWiseTuple( title );
  }
  else    {
    /// Eventually allow loading through factory?
  }
  if ( 0 != pTuple )      {
    pTuple->setTupleService(this);
    status = StatusCode::SUCCESS;
  }
  refpTuple = pTuple;
  return status;
}

/// Book Ntuple and register it with the data store.
NTuple::Tuple* NTupleSvc::book (const std::string& fullPath, const CLID& type, const std::string& title)  {
  DataObject* pObj = 0;
  std::string path = fullPath;
  MsgStream log(msgSvc(), name());
  if ( path[0] != SEPARATOR )   {
    path = m_rootName;
    path += SEPARATOR;
    path += fullPath;
  }
  StatusCode status = retrieveObject(path, pObj);
  if ( !status.isSuccess() )    {
    int sep = path.rfind(SEPARATOR);
    if ( sep > 0 )    {
      std::string p_path (path, 0, sep);
      std::string o_path (path, sep, path.length());
      DataObject* dir = createDirectory(p_path);
      if ( 0 != dir )   {
        NTuple::Tuple* tup = book( dir, o_path, type, title);
        if ( 0 == tup )   {
          log << MSG::ERROR << "Cannot book N-tuple " << path << " (Unknown reason)" << endmsg;
        }
        return tup;
      }
      log << MSG::ERROR << "Cannot book N-tuple " << path << " (Invalid parent directory)" << endmsg;
      return 0;
    }
    log << MSG::ERROR << "Cannot book N-tuple " << path << " (Invalid path)" << endmsg;
    return 0;
  }
  log << MSG::ERROR << "Cannot book N-tuple " << path << " (Exists already)" << endmsg;
  return 0;
}

/// Book Ntuple and register it with the data store.
NTuple::Tuple* NTupleSvc::book (const std::string& dirPath, const std::string& relPath, const CLID& type, const std::string& title)  {
  std::string full = dirPath;
  if (relPath[0] != SEPARATOR) full += SEPARATOR;
  full += relPath;
  return book(full, type, title);
}

/// Book Ntuple and register it with the data store.
NTuple::Tuple* NTupleSvc::book (const std::string& dirPath, long id, const CLID& type, const std::string& title)  {
  return book( dirPath, toStr(id), type, title);
}

/// Book Ntuple and register it with the data store.
NTuple::Tuple* NTupleSvc::book (DataObject* pParent, const std::string& relPath, const CLID& type, const std::string& title)  {
  NTuple::Tuple* pObj = 0;
  // Check if object is already present
  StatusCode status = findObject(pParent, relPath, *pp_cast<DataObject>(&pObj));
  // No ? Then create it!
  if ( !status.isSuccess() )    {
    status = create( type, title, pObj);
    if ( status.isSuccess() )   {
      // Finally register the created N tuple with the store
      status = registerObject(pParent, relPath, pObj);
      if ( status.isSuccess() )    {
        return pObj;
      }
      pObj->release();
    }
  }
  return 0;
}

/// Book Ntuple and register it with the data store.
NTuple::Tuple* NTupleSvc::book (DataObject* pParent,
                                long id,
                                const CLID& type,
                                const std::string& title)  {
  return book( pParent, toStr(id), type, title);
}

/// Create Ntuple directory and register it with the data store.
NTuple::Directory* NTupleSvc::createDirectory (DataObject* pParent,
                                               const std::string& relPath)   {
  if ( 0 != pParent )   {
    IRegistry* pDir = pParent->registry();
    if ( 0 != pDir )    {
      std::string full = pDir->identifier();
      full += (relPath[0]=='/') ? "" : "/";
      full += relPath;
      return createDirectory(full);
    }
  }
  return 0;
}

/// Create Ntuple directory and register it with the data store.
NTuple::Directory* NTupleSvc::createDirectory (DataObject* pParent, long id)    {
  return createDirectory( pParent, toStr(id) );
}

/// Create Ntuple directory and register it with the data store.
NTuple::Directory* NTupleSvc::createDirectory (const std::string& dirPath, long id)    {
  return createDirectory( dirPath, toStr(id) );
}

/// Create Ntuple directory and register it with the data store.
NTuple::Directory* NTupleSvc::createDirectory (const std::string& dirPath, const std::string& relPath )    {
  std::string full = dirPath;
  full += (relPath[0]=='/') ? "" : "/";
  full += relPath;
  return createDirectory(full);
}

StatusCode NTupleSvc::attachTuple(const std::string& filename, const std::string& logname, const char typ, const long t)   {
  MsgStream log(msgSvc(), name());
  DataObject* p;
  // First get the root object
  StatusCode status = retrieveObject(m_rootName, p);
  if ( status.isSuccess() )   {
    // Now add the registry entry to the store
    std::string entryname = m_rootName;
    entryname += '/';
    entryname += logname;
    GenericAddress* pA =
       new GenericAddress(t, CLID_NTupleFile, filename, entryname, 0, typ);
    status = registerAddress(p, logname, pA);
    if ( status.isSuccess() )    {
      log << MSG::INFO << "Added stream file:" << filename << " as " << logname << endmsg;
      return status;
    }
    pA->release();
  }
  log << MSG::ERROR << "Cannot add file:" << filename << " as " << logname << endmsg;
  return status;
}

/// Create Ntuple directory and register it with the data store.
NTuple::Directory* NTupleSvc::createDirectory (const std::string& fullPath)   {
  NTuple::Directory* p = 0;
  StatusCode status = findObject(fullPath, *pp_cast<DataObject>(&p));
  if ( !status.isSuccess() )   {
    int sep2 = fullPath.rfind(SEPARATOR);
    if ( sep2 > 0 )   {
      std::string relPath = fullPath.substr(0, sep2);
      p = createDirectory(relPath);
      if ( 0 != p )    {
        p = new NTuple::Directory();
        // Finally register the created N tuple with the store
        status = registerObject(fullPath, p);
        if ( status.isSuccess() )    {
          // ...starting from the file entries
          IConversionSvc* svc = getDataLoader(p->registry());
          if ( 0 != svc )   {
            IOpaqueAddress* pAddr = 0;
            status = svc->createRep (p, pAddr);
            if ( status.isSuccess() )   {
              p->registry()->setAddress(pAddr);
              status = svc->fillRepRefs (pAddr, p);
              if ( status.isSuccess() )   {
                return p;
              }
            }
          }
          unregisterObject(p);
        }
        p->release();
        p = 0;
      }
    }
  }
  try {
    p = dynamic_cast<NTuple::Directory*>(p);
    return p;
  }
  catch (...) {
  }
  return 0;
}

/// Access N tuple on disk
NTuple::Tuple* NTupleSvc::access(const std::string&, const std::string&)  {
  MsgStream log ( msgSvc(), name() );
  return 0;
}

/// Save N tuple to disk. Must be called in order to close the ntuple file properly
StatusCode NTupleSvc::save(const std::string& fullPath)  {
  MsgStream log ( msgSvc(), name() );
  NTuple::Tuple* pObj = 0;
  StatusCode status = findObject(fullPath, *pp_cast<DataObject>(&pObj));  // Check if object is  present
  if ( status.isSuccess() )   {
    return save ( pObj );
  }
  return INVALID_OBJ_PATH;
}

/// Save N tuple to disk. Must be called in order to close the ntuple file properly
StatusCode NTupleSvc::save(NTuple::Tuple* n_tuple)  {
  NTuple::TupleImp* tuple = (NTuple::TupleImp*)n_tuple;
  if ( 0 != tuple )   {
    try   {
      IConversionSvc* pSvc = tuple->conversionService();
      IRegistry*      pReg = tuple->registry();
      if ( 0 != pSvc && 0 != pReg )    {
        IOpaqueAddress* pAddr = pReg->address();
        StatusCode status = pSvc->updateRep(pAddr, n_tuple);
        if ( status.isSuccess() )   {
          status = pSvc->updateRepRefs(pAddr, n_tuple);
        }
        return status;
      }
      return IDataProviderSvc::NO_DATA_LOADER;
    }
    catch(...)    {
    }
  }
  return INVALID_OBJECT;
}

/// Save N tuple to disk. Must be called in order to close the ntuple file properly
StatusCode NTupleSvc::save(DataObject* pParent, const std::string& relPath)  {
  NTuple::Tuple* pObj = 0;
  StatusCode status = findObject(pParent, relPath, *pp_cast<DataObject>(&pObj));  // Check if object is  present
  if ( status.isSuccess() )   {
    return save ( pObj );
  }
  return INVALID_OBJ_PATH;
}

/// Write single record to N tuple.
StatusCode NTupleSvc::writeRecord( NTuple::Tuple* n_tuple )   {
  NTuple::TupleImp* tuple = (NTuple::TupleImp*)n_tuple;
  if ( 0 != tuple )   {
    try   {
      IConversionSvc* pSvc = tuple->conversionService();
      if ( 0 == pSvc )    {
        pSvc = getDataLoader(n_tuple->registry());
        tuple->setConversionService(pSvc);
      }
      if ( 0 != pSvc )    {
        IRegistry* pReg = n_tuple->registry();
        IOpaqueAddress* pAddr = pReg->address();
        StatusCode status = pSvc->createRep(n_tuple, pAddr);
        if ( status.isSuccess() )   {
          pReg->setAddress(pAddr);
          status = pSvc->fillRepRefs(pAddr, n_tuple);
        }
        return status;
      }
      return IDataProviderSvc::NO_DATA_LOADER;
    }
    catch(...)    {
    }
  }
  return INVALID_OBJECT;
}

/// Write single record to N tuple.
StatusCode NTupleSvc::writeRecord(const std::string& fullPath )   {
  NTuple::Tuple* pObj = 0;
  StatusCode status = findObject(fullPath, *pp_cast<DataObject>(&pObj));  // Check if object is  present
  if ( status.isSuccess() )   {
    return writeRecord ( pObj );
  }
  return INVALID_OBJ_PATH;
}

/// Write single record to N tuple.
StatusCode NTupleSvc::writeRecord( DataObject* pParent, const std::string& relPath)   {
  NTuple::Tuple* pObj = 0;
  StatusCode status = findObject(pParent, relPath, *pp_cast<DataObject>(&pObj));  // Check if object is  present
  if ( status.isSuccess() )   {
    return writeRecord ( pObj );
  }
  return INVALID_OBJ_PATH;
}

/// Read single record from N tuple.
StatusCode NTupleSvc::readRecord( NTuple::Tuple* n_tuple )  {
  StatusCode status = INVALID_OBJECT;
  NTuple::TupleImp* tuple = (NTuple::TupleImp*)n_tuple;
  if ( 0 != tuple )   {
    try   {
      IConversionSvc* pSvc = tuple->conversionService();
      if ( 0 == pSvc )    {
        pSvc = getDataLoader(n_tuple->registry());
        tuple->setConversionService(pSvc);
      }
      if ( 0 != pSvc )    {
        IRegistry*      pReg  = n_tuple->registry();
        IOpaqueAddress* pAddr = pReg->address();
        status = pSvc->updateObj(pAddr, n_tuple);
        if ( status.isSuccess() )   {
          status = pSvc->updateObjRefs(pAddr, n_tuple);
        }
        return status;
      }
      status = IDataProviderSvc::NO_DATA_LOADER;
    }
    catch(...)    {
      status = INVALID_OBJECT;
    }
  }
  return status;
}

/// Read single record from N tuple.
StatusCode NTupleSvc::readRecord(const std::string& fullPath)  {
  NTuple::Tuple* pObj = 0;
  StatusCode status = findObject(fullPath, *pp_cast<DataObject>(&pObj));  // Check if object is  present
  if ( status.isSuccess() )   {
    return readRecord ( pObj );
  }
  return INVALID_OBJ_PATH;
}

/// Read single record from N tuple.
StatusCode NTupleSvc::readRecord(DataObject* pParent, const std::string& relPath)  {
  NTuple::Tuple* pObj = 0;
  StatusCode status = findObject(pParent, relPath, *pp_cast<DataObject>(&pObj));  // Check if object is  present
  if ( status.isSuccess() )   {
    return readRecord ( pObj );
  }
  return INVALID_OBJ_PATH;
}
