#include "VFSSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IAlgTool.h"

DECLARE_SERVICE_FACTORY(VFSSvc)

//------------------------------------------------------------------------------
VFSSvc::VFSSvc(const std::string& name, ISvcLocator* svc):
  base_class(name,svc) {

  m_urlHandlersNames.push_back("FileReadTool");

  declareProperty("FileAccessTools",m_urlHandlersNames,
                  "List of tools implementing the IFileAccess interface.");

  declareProperty("FallBackProtocol",m_fallBackProtocol = "file",
                  "URL prefix to use if the prefix is not present.");
}
//------------------------------------------------------------------------------
VFSSvc::~VFSSvc(){}
//------------------------------------------------------------------------------
StatusCode VFSSvc::initialize() {
  StatusCode sc = Service::initialize();
  if (sc.isFailure()){
    return sc;
  }

  MsgStream log(msgSvc(), name());

  m_toolSvc = serviceLocator()->service("ToolSvc");
  if (!m_toolSvc.isValid()){
    log << MSG::ERROR << "Cannot locate ToolSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  IAlgTool *tool;
  IFileAccess *hndlr;
  std::vector<std::string>::iterator i;
  for(i = m_urlHandlersNames.begin(); i != m_urlHandlersNames.end(); ++i){
    // retrieve the tool and the pointer to the interface
    sc = m_toolSvc->retrieve(*i,IAlgTool::interfaceID(),tool,0,true);
    if (sc.isFailure()){
      log << MSG::ERROR << "Cannot get tool " << *i << endmsg;
      return sc;
    }
    m_acquiredTools.push_front(tool); // this is one tool that we will have to release
    sc = tool->queryInterface(IFileAccess::interfaceID(),pp_cast<void>(&hndlr));
    if (sc.isFailure()){
      log << MSG::ERROR << *i << " does not implement IFileAccess" << endmsg;
      return sc;
    }
    // We do not need to increase the reference count for the IFileAccess interface
    // because we hold the tool by its IAlgTool interface.
    hndlr->release();
    // loop over the list of supported protocols and add them to the map (for quick access)
    for ( std::vector<std::string>::const_iterator prot = hndlr->protocols().begin();
          prot != hndlr->protocols().end(); ++prot ){
      m_urlHandlers[*prot] = hndlr;
    }
  }

  // Now let's check if we can handle the fallback protocol
  if ( m_urlHandlers.find(m_fallBackProtocol) == m_urlHandlers.end() ) {
    log << MSG::ERROR << "No handler specified for fallback protocol prefix "
        << m_fallBackProtocol << endmsg;
    return StatusCode::FAILURE;
  }

  // Note: the list of handled protocols will be filled only if requested

  return sc;
}
//------------------------------------------------------------------------------
StatusCode VFSSvc::finalize() {
  m_urlHandlers.clear(); // clear the map
  m_protocols.clear();

  if (m_toolSvc) {
    // release the acquired tools (from the last acquired one)
    while ( m_acquiredTools.begin() != m_acquiredTools.end() ){
      m_toolSvc->releaseTool(*m_acquiredTools.begin()).ignore();
      m_acquiredTools.pop_front();
    }
    m_toolSvc->release(); // release the tool service
    m_toolSvc = 0;
  }
  return Service::finalize();
}
//------------------------------------------------------------------------------
std::auto_ptr<std::istream> VFSSvc::open(const std::string &url){

  // get the url prefix endpos
  std::string::size_type pos = url.find("://");

  if (std::string::npos == pos) { // no url prefix
    return m_urlHandlers[m_fallBackProtocol]->open(url);
  }

  std::string url_prefix(url,0,pos);
  if ( m_urlHandlers.find(url_prefix) == m_urlHandlers.end() ) {
    // if we do not have a handler for the URL prefix,
    // use the fall back one and pass only the path
    return m_urlHandlers[m_fallBackProtocol]->open(url.substr(pos+3));
  }
  return m_urlHandlers[url_prefix]->open(url);
}
//------------------------------------------------------------------------------
namespace {
  /// small helper to fill a container with the first element of pairs
  /// (like the keys of a map)
  template <class Container>
  struct append_key
  {
    append_key(Container &C):c(C){}

    template <class PAIR>
    void operator() (const PAIR &x)
    {
      c.push_back(x.first);
    }

    Container &c;
  };
}
const std::vector<std::string> &VFSSvc::protocols() const
{
  if (m_protocols.empty()){
    // prepare the list of handled protocols
    std::for_each(m_urlHandlers.begin(),m_urlHandlers.end(),
                  append_key<std::vector<std::string> >(const_cast<VFSSvc*>(this)->m_protocols));
  }
  return m_protocols;
}
