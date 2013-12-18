#include "GaudiKernel/strcasecmp.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/PluginService.h>
#include "MultiFileCatalog.h"
#include <stdexcept>
#include <algorithm>

namespace Gaudi { std::string createGuidAsString(); }

using namespace Gaudi;
using namespace std;

DECLARE_COMPONENT(MultiFileCatalog)

namespace {
  template <class V,class F>
  bool _findX0Bool(V& array, F pmf, bool invert) {
    for(typename V::const_iterator i=array.begin(); i != array.end(); ++i) {
      bool res = invert ? !((*i)->*pmf)() : ((*i)->*pmf)();
      if ( !res ) return false;
    }
    return true;
  }
}
// ----------------------------------------------------------------------------
MultiFileCatalog::MultiFileCatalog(const std::string& nam, ISvcLocator* svc)
  : base_class(nam, svc), m_started(false), m_oldNames()
{
  declareProperty("Catalogs", m_catalogNames, "The list of Catalogs")
    -> declareUpdateHandler ( &Gaudi::MultiFileCatalog::propHandler, this ) ;
  m_catalogNames.push_back("xmlcatalog_file:test_catalog.xml");
}
// ----------------------------------------------------------------------------
MultiFileCatalog::~MultiFileCatalog()   {
}
// ----------------------------------------------------------------------------
StatusCode MultiFileCatalog::initialize()  {
  CatalogNames::const_iterator i;
  std::string current = "";
  if ( !Service::initialize().isSuccess() )  {
    printError("Failed to initialize service base class.",false);
    return StatusCode::SUCCESS;
  }
  try {
    for(i=m_catalogNames.begin(); i != m_catalogNames.end(); ++i)  {
      current = *i;
      addCatalog(*i);
    }
    init();
    return StatusCode::SUCCESS;
  }
  catch(const std::exception& /* e */)  {
    printError("Cannot add file catalog:"+current,false);
  }
  return StatusCode::FAILURE;
}
// ----------------------------------------------------------------------------
StatusCode MultiFileCatalog::finalize()  {
  commit();
  _exec(&IFileCatalog::release);
  m_catalogs.clear();
  m_started = false;
  return Service::finalize();
}
// ----------------------------------------------------------------------------
/// Create file identifier using UUID mechanism
std::string MultiFileCatalog::createFID()  const {
  return createGuidAsString();
}
// ----------------------------------------------------------------------------
MultiFileCatalog::CSTR MultiFileCatalog::connectInfo() const {
  static string s("MultiCatalog");
  return s;
}
// ----------------------------------------------------------------------------
IFileCatalog* MultiFileCatalog::getCatalog(CSTR fid,
                                           bool throw_if_not,
                                           bool writable,
                                           bool prt) const
{
  for(Catalogs::const_iterator i=m_catalogs.begin(); i != m_catalogs.end(); ++i)  {
    IFileCatalog* c = *i;
    if ( c )  {
      if ( writable && c->readOnly() )
        continue;
      else if ( fid.empty() )
        return c;
      else if ( !fid.empty() && c->existsFID(fid) )
        return c;
    }
  }
  if ( prt )  {
    printError("No writable file catalog found which contains FID:"+fid,throw_if_not);
  }
  else  {
    MsgStream log(msgSvc(),name());
    log << MSG::DEBUG << "No writable file catalog found which contains FID:" << fid << endmsg;
  }
  return 0;
}
// ----------------------------------------------------------------------------
IFileCatalog* MultiFileCatalog::findCatalog(CSTR connect, bool must_be_writable) const  {
  for(Catalogs::const_iterator i=m_catalogs.begin(); i != m_catalogs.end(); ++i) {
    if ( connect == (*i)->connectInfo() )
      return (must_be_writable && (*i)->readOnly()) ? 0 : *i;
  }
  return 0;
}
// ----------------------------------------------------------------------------
MultiFileCatalog::Catalogs::iterator
MultiFileCatalog::i_findCatalog(CSTR connect, bool must_be_writable)  {
  for(Catalogs::iterator i=m_catalogs.begin(); i != m_catalogs.end(); ++i) {
    if ( connect == (*i)->connectInfo() ) {
      return (must_be_writable && (*i)->readOnly()) ? m_catalogs.end() : i;
    }
  }
  return m_catalogs.end();
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::printError(CSTR msg, bool rethrow)  const  {
  MsgStream log(msgSvc(),name());
  if ( rethrow )  {
    log << MSG::FATAL << msg << endmsg;
    throw runtime_error("Catalog> "+msg);
  }
  log << MSG::ERROR << msg << endmsg;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::addCatalog(CSTR con)  {
  if ( !con.empty() )  {
    if ( 0 == findCatalog(con,false) )  {
      static const string xml_typ = "Gaudi::XMLFileCatalog";
      string::size_type id0 = con.find("_");
      string typ = con.substr(0,id0);
      string url = con.substr(id0+1);
      IInterface* cat = 0;
      if ( strncasecmp("xml",typ.c_str(),3) == 0 )    {
        cat = IFileCatalog::Factory::create(xml_typ,url,msgSvc().get());
      }
      else {
        using Gaudi::PluginService::Details::Registry;
        Registry& registry = Registry::instance();
        if (registry.getInfo(typ).type ==
                   typeid(Service::Factory::FuncType).name()) {
          cat = Service::Factory::create(typ,url,serviceLocator().get());
        } else if (registry.getInfo(typ).type ==
            typeid(IFileCatalog::Factory::FuncType).name()) {
          cat = IFileCatalog::Factory::create(typ,url,msgSvc().get());
        }
      }
      if ( cat )  {
        IFileCatalog* fileCat = 0;
        if ( cat->queryInterface(IFileCatalog::interfaceID(),pp_cast<void>(&fileCat)).isSuccess() ) {
          addCatalog(fileCat);
          cat->release();
          return;
        }
      }
      printError("Failed to create catalog connection:"+con,true);
    }
    /// Catalog is already present ... silently ignore request
    return;
  }
  printError("Got invalid (empty) catalog connection string.",true);
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::addCatalog(IFileCatalog* cat)  {
  if ( cat )  {
    cat->addRef();
    m_catalogs.push_back(cat);
    return;
  }
  printError("Got invalid catalog to be added to multi catalog.",true);
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::removeCatalog(CSTR con)  {
  if ( con.empty() || con == "*" )  {
    _exec(&IFileCatalog::release);
    m_catalogs.clear();
    return;
  }
  removeCatalog(findCatalog(con,false));
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::removeCatalog(const IFileCatalog* cat)  {
  if ( cat )  {
    Catalogs::iterator i=find(m_catalogs.begin(),m_catalogs.end(),cat);
    if ( i != m_catalogs.end() )  {
      (*i)->release();
      m_catalogs.erase(i);
      return;
    }
    printError("Unknown file catalog -- cannot be removed.",true);
  }
  printError("Invalid file catalog.",true);
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::setWriteCatalog(IFileCatalog* cat)  {
  if ( cat )  {
    if ( !cat->readOnly() )  {
      Catalogs::iterator i=find(m_catalogs.begin(),m_catalogs.end(),cat);
      if ( i != m_catalogs.end() )  {
        m_catalogs.erase(i);
        m_catalogs.insert(m_catalogs.begin(),cat);
        return;
      }
      printError("The catalog "+cat->connectInfo()+" is not known.",true);
    }
    printError("The catalog "+cat->connectInfo()+" is not writable.",true);
  }
  printError("Invalid file catalog.",true);
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::setWriteCatalog(CSTR connect)  {
  Catalogs::iterator i = i_findCatalog(connect,true);
  if ( i == m_catalogs.end() ) {
    addCatalog(connect);
    setWriteCatalog(findCatalog(connect,true));
    return;
  }
  setWriteCatalog(*i);
}
// ----------------------------------------------------------------------------
string MultiFileCatalog::getMetaDataItem(CSTR fid,CSTR attr) const  {
  std::string result;
  for(Catalogs::const_iterator i=m_catalogs.begin(); i != m_catalogs.end(); ++i)
    if ( !(result= (*i)->getMetaDataItem(fid,attr)).empty() ) break;
  return result;
}
/// Create a FileID and DOM Node of the PFN with all the attributes
void MultiFileCatalog::registerPFN(CSTR fid, CSTR pfn, CSTR ftype) const {
  IFileCatalog* c = getCatalog(fid,false,true,false);
  if ( !c ) c = getCatalog("",true,true,true);
  c->registerPFN(fid, pfn, ftype);
}
/// Create a FileID and DOM Node of the LFN with all the attributes
void MultiFileCatalog::registerLFN(CSTR fid, CSTR lfn) const  {
  IFileCatalog* c = getCatalog(fid,false,true,false);
  if ( !c ) c = getCatalog("",true,true,true);
  c->registerLFN(fid, lfn);
}
// ----------------------------------------------------------------------------
bool MultiFileCatalog::readOnly() const
{  return _findX0Bool(m_catalogs,&IFileCatalog::readOnly,false);              }
// ----------------------------------------------------------------------------
bool MultiFileCatalog::dirty() const
{  return _findX0Bool(m_catalogs,&IFileCatalog::dirty,true);                  }
// ----------------------------------------------------------------------------
void MultiFileCatalog::propHandler(Property& /* p */)
{
  // not yet initialized
  if ( !m_started ) { m_oldNames = m_catalogNames; return; } // RETURN
  // no real change - no action
  if ( m_catalogNames == m_oldNames ) { return; }
  m_oldNames = m_catalogNames ;
  // remove ALL catalogs
  removeCatalog("") ;
  // add new catalogs
  for ( CatalogNames::const_iterator inew = m_catalogNames.begin() ;
        m_catalogNames.end() != inew ; ++inew ) { addCatalog ( *inew ) ; }
  // start
  init() ;
  //
  MsgStream log ( msgSvc() , name() ) ;
  log << MSG::DEBUG
      << "New catalogs to be used: "
      << Gaudi::Utils::toString ( m_catalogNames ) << endmsg ;
}
