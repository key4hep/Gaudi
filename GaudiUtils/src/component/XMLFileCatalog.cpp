#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/sax/SAXParseException.hpp"
#include "xercesc/sax/EntityResolver.hpp"
#include "xercesc/sax/InputSource.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XercesDefs.hpp"
#include "xercesc/util/XMLUni.hpp"
#include "xercesc/util/XMLURL.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOM.hpp"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include <Gaudi/PluginService.h>

#include "XMLFileCatalog.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include "uuid/uuid.h"

#include <boost/format.hpp>

using namespace xercesc;
using namespace Gaudi;
using namespace std;

#if _XERCES_VERSION <= 30000
// API change between XercesC 2 and 3
#define setIdAttribute(a, b) setIdAttribute(a)
#endif


DECLARE_FACTORY(XMLFileCatalog, IFileCatalog::Factory)

namespace {

  typedef const string& CSTR;
  inline string _toString(const XMLCh *toTranscode)  {
    char * buff = XMLString::transcode(toTranscode);
    string tmp(buff==0 ? "" : buff);
    XMLString::release(&buff);
    return tmp;
  }
  struct __Init  {
    __Init() {
      try { XMLPlatformUtils::Initialize();      }
      catch (const XMLException& e)   {
        cout << "Xerces-c error in initialization:" << _toString(e.getMessage()) << endl;
      }
    }
    ~__Init() {
      XMLPlatformUtils::Terminate();
    }
  };
  __Init __In__;

  struct XMLStr  {
    XMLCh* m_xml;
    XMLStr(CSTR c)                { m_xml=XMLString::transcode(c.c_str());  }
    ~XMLStr()                     { if (m_xml) XMLString::release(&m_xml);  }
    operator const XMLCh*() const { return m_xml;                           }
  };
  struct XMLTag : public XMLStr {
    string m_str;
    XMLTag(CSTR s) : XMLStr(s), m_str(s) {  }
    ~XMLTag()                            {  }
    operator CSTR () const  { return m_str; }
  };
  // bool operator==(const XMLTag& b, CSTR c) {  return c==b.m_str; }
  bool operator==(CSTR c, const XMLTag& b) {  return c==b.m_str; }
  struct XMLCollection  {
    DOMElement* m_node;
    XMLCollection(DOMNode* n, bool use_children=true) : m_node((DOMElement*)n) {
      if ( use_children )  {
        if ( m_node ) m_node = (DOMElement*)m_node->getFirstChild();
        if ( m_node && m_node->getNodeType() != DOMNode::ELEMENT_NODE ) ++(*this);
      }
    }
    operator bool()  const                { return 0 != m_node;                         }
    operator DOMNode* () const            { return m_node;                              }
    operator DOMElement* () const         { return m_node;                              }
    DOMElement* operator->() const        { return m_node;                              }
    string attr(const XMLTag& tag) const  { return _toString(m_node->getAttribute(tag));}
    string attr(CSTR tag)  const          { return attr(XMLTag(tag));                   }
    string tag() const                    { return _toString(m_node->getTagName());     }
    void operator++()   {
      while(m_node)  {
        m_node = (DOMElement*)m_node->getNextSibling();
        if ( m_node && m_node->getNodeType() == DOMNode::ELEMENT_NODE ) {
          return;
        }
      }
    }
  };
  struct ErrHandler : public ErrorHandler    {
    /// Reference to catalog object
    IMessageSvc* m_msg;
    /// Constructor
    ErrHandler(IMessageSvc* m) : m_msg(m) {}
    /// Reset errors (Noop)
    void resetErrors()                          {      }
    /// Warnings callback. Ignore them
    void warning(const SAXParseException& /* e */)    {     }
    /// Error handler
    void error(const SAXParseException& e);
    /// Fatal error handler
    void fatalError(const SAXParseException& e);
    virtual ~ErrHandler() {}
  };
  struct DTDRedirect : public EntityResolver  {
    InputSource* resolveEntity(const XMLCh* const /* pubId */, const XMLCh* const /* sysId */)  {
      static const char* dtdID = "redirectinmem.dtd";
      static const char* dtd = \
        "\
        <!ELEMENT POOLFILECATALOG (META*,File*)>\
        <!ELEMENT META EMPTY>\
        <!ELEMENT File (physical,logical,metadata*)>\
        <!ATTLIST META name CDATA #REQUIRED>\
        <!ATTLIST META type CDATA #REQUIRED>\
        <!ELEMENT physical (pfn)+>\
        <!ELEMENT logical (lfn)*>\
        <!ELEMENT metadata EMPTY>\
        <!ELEMENT lfn EMPTY>\
        <!ELEMENT pfn EMPTY>\
        <!ATTLIST File ID ID  #REQUIRED>\
        <!ATTLIST pfn name ID #REQUIRED>\
        <!ATTLIST pfn filetype CDATA #IMPLIED>\
        <!ATTLIST lfn name ID #REQUIRED>\
        <!ATTLIST metadata att_name  CDATA #REQUIRED>\
        <!ATTLIST metadata att_value CDATA #REQUIRED>\
        ";
      static const size_t len = strlen(dtd);
      return new MemBufInputSource((const XMLByte*)dtd,len,dtdID,false);
    }
    virtual ~DTDRedirect() {}
  };

  void ErrHandler::error(const SAXParseException& e)  {
    string m(_toString(e.getMessage()));
    if (m.find("The values for attribute 'name' must be names or name tokens")!=string::npos ||
      m.find("The values for attribute 'ID' must be names or name tokens")!=string::npos   ||
      m.find("for attribute 'name' must be Name or Nmtoken")!=string::npos                 ||
      m.find("for attribute 'ID' must be Name or Nmtoken")!=string::npos                   ||
      m.find("for attribute 'name' is invalid Name or NMTOKEN value")!=string::npos        ||
      m.find("for attribute 'ID' is invalid Name or NMTOKEN value")!=string::npos      )
      return;
    string sys(_toString(e.getSystemId()));
    MsgStream log(m_msg,"XMLCatalog");
    log << MSG::ERROR << "Error at file \""  << sys
      << "\", line " << e.getLineNumber() << ", column " << e.getColumnNumber() << endmsg
      << "Message: " << m << endmsg;
  }
  void ErrHandler::fatalError(const SAXParseException& e)  {
    MsgStream log(m_msg,"XMLCatalog");
    string m(_toString(e.getMessage()));
    string sys(_toString(e.getSystemId()));
    log << MSG::ERROR << "Fatal Error at file \"" << sys
      << "\", line " << e.getLineNumber() << ", column " << e.getColumnNumber() << endmsg
      << "Message: " << m << endmsg;
    throw runtime_error( "Standard pool exception : Fatal Error on the DOM Parser" );
  }

  const XMLTag EmptyCatalog("<!-- Edited By POOL -->\n"
                              "<!DOCTYPE POOLFILECATALOG SYSTEM \"InMemory\">\n"
                              "<POOLFILECATALOG>\n"
                              "</POOLFILECATALOG>\n");
  const XMLTag PFNCOLL         ("physical");
  const XMLTag LFNCOLL         ("logical");
  const XMLTag PFNNODE         ( "pfn");
  const XMLTag LFNNODE         ( "lfn");
  const XMLTag Attr_type       ( "type");
  const XMLTag Attr_ID         ( "ID");
  const XMLTag Attr_name       ( "name");
  const XMLTag Attr_ftype      ( "filetype");
  const XMLTag MetaNode        ( "metadata");
  const XMLTag Attr_metaName   ( "att_name");
  const XMLTag Attr_metaValue  ( "att_value");
}

/// Create file identifier using UUID mechanism
std::string Gaudi::createGuidAsString()  {
  uuid_t uuid;
  ::uuid_generate_time(uuid);
  struct Guid {
    unsigned int   Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
  } *g = (Guid*)&uuid;

  boost::format text("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X");
  text % g->Data1 % g->Data2 % g->Data3;
  for (int i = 0; i < 8; ++i)
    text % (unsigned short)g->Data4[i];
  return text.str();
}
// ----------------------------------------------------------------------------
XMLFileCatalog::XMLFileCatalog(CSTR uri, IMessageSvc* m)
: m_rdOnly(false),m_update(false),m_doc(0),m_parser(0),m_errHdlr(0),
  m_file(uri), m_msgSvc(m)
{
}
// ----------------------------------------------------------------------------
XMLFileCatalog::~XMLFileCatalog()   {
  if (m_parser) delete m_parser;
  m_parser = 0;
  if (m_errHdlr) delete m_errHdlr;
  m_errHdlr = 0;
  m_doc = 0;
}
// ----------------------------------------------------------------------------
/// Create file identifier using UUID mechanism
std::string XMLFileCatalog::createFID()  const {
  return createGuidAsString();
}
// ----------------------------------------------------------------------------
DOMDocument* XMLFileCatalog::getDoc(bool throw_if_no_exists)  const  {
  if ( !m_doc && throw_if_no_exists )
    printError("The XML catalog was not started.",true);
  return m_doc;
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::printError(CSTR msg, bool rethrow)  const  {
  MsgStream log(m_msgSvc,"XMLCatalog");
  log << MSG::FATAL << msg << endmsg;
  if ( rethrow )  {
    throw runtime_error("XMLFileCatalog> "+msg);
  }
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::init()   {
  string xmlFile = getfile(false);
  try{
    if ( m_parser ) delete m_parser;
    m_parser = new XercesDOMParser;
    m_parser->setValidationScheme(XercesDOMParser::Val_Auto);
    m_parser->setDoNamespaces(false);
    DTDRedirect dtdinmem;
    m_parser->setEntityResolver(&dtdinmem);
    if ( ! m_errHdlr ) m_errHdlr = new ErrHandler(m_msgSvc);
    m_parser->setErrorHandler(m_errHdlr);
    if ( !xmlFile.empty() )  {
      m_parser->parse(xmlFile.c_str());
    }
    else  {
      const std::string& s = EmptyCatalog;
      MemBufInputSource src((const XMLByte*)s.c_str(),s.length(),"MemCatalog");
      m_parser->parse(src);
    }
    m_doc = m_parser->getDocument();
  }
  catch (const XMLException& e) {
    printError("XML parse error["+xmlFile+"]: "+_toString(e.getMessage()));
  }
  catch (const DOMException& e) {
    printError("XML parse error["+xmlFile+"]: "+_toString(e.getMessage()));
  }
  catch (...)  {
    printError("UNKNOWN XML parse error in file "+xmlFile);
  }
}
// ----------------------------------------------------------------------------
string XMLFileCatalog::lookupFID(const std::string& fid)  const  {
  std::string result;
  DOMNode* e = element(fid,false);
  e = e ? e->getParentNode() : 0;    // Mode up to <logical>
  e = e ? e->getParentNode() : 0;    // Mode up to <File>
  if ( e ) {
    if ( e->getAttributes() ) { // Need to check this. The node may be no DOMElement
      char* nam = XMLString::transcode(((DOMElement*)e)->getAttribute(Attr_ID));
      if ( nam ) result = nam;
      XMLString::release(&nam);
    }
  }
  return result;
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::getFID(Strings& fids) const {
  fids.clear();
  DOMNode* fde = getDoc(true)->getElementsByTagName(XMLStr("*"))->item(0);
  for(XMLCollection c(child(fde,"File"), false); c; ++c)
    fids.push_back(c.attr(Attr_ID));
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::getPFN(CSTR fid, Files& files)  const {
  files.clear();
  for(XMLCollection c(child(child(element(fid,false),PFNCOLL),PFNNODE), false); c; ++c)
    files.push_back(make_pair(c.attr(Attr_name),c.attr(Attr_ftype)));
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::getLFN(CSTR fid, Files& files) const  {
  files.clear();
  for(XMLCollection c(child(child(element(fid,false),LFNCOLL),LFNNODE), false); c; ++c)
    files.push_back(make_pair(c.attr(Attr_name),fid));
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::getMetaData(CSTR fid, Attributes& attr)  const  {
  attr.clear();
  for(XMLCollection c(child(element(fid),MetaNode), false); c; ++c)
    attr.push_back(make_pair(c.attr(Attr_metaName),c.attr(Attr_metaValue)));
  if ( attr.size() > 0 )
    attr.push_back(make_pair("guid",fid));
}
// ----------------------------------------------------------------------------
DOMNode* XMLFileCatalog::child(DOMNode* par,CSTR tag,CSTR attr,CSTR val) const {
  for(XMLCollection c(par); c; ++c ) {
    if( c.tag() == tag )  {
      if( !attr.empty() && c.attr(attr) != val) continue;
      return c;
    }
  }
  return 0;
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::setMetaData(CSTR fid, CSTR attr, CSTR val)  const  {
  if ( !readOnly() )  {
    DOMNode*    node = element(fid);
    DOMElement* mnod = (DOMElement*)child(node,MetaNode,Attr_metaName,attr);
    if (!mnod){
      mnod = getDoc(true)->createElement(MetaNode);
      node->appendChild(mnod);
      mnod->setAttribute(Attr_metaName,XMLStr(attr));
    }
    mnod->setAttribute(Attr_metaValue,XMLStr(val));
    m_update = true;
    return;
  }
  printError("Cannot update readonly catalog!");
}
// ----------------------------------------------------------------------------
string XMLFileCatalog::getMetaDataItem(CSTR fid,CSTR attr) const  {
  XMLCollection c(child(getDoc(true)->getElementById(XMLStr(fid)),MetaNode,Attr_metaName,attr));
  return c ? c.attr(attr) : string("");
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::dropMetaData(CSTR fid,CSTR attr) const  {
  vector<DOMNode*> gbc;
  DOMNode* fn = getDoc(true)->getElementById(XMLStr(fid));
  for(XMLCollection c(child(fn,MetaNode)); c; ++c)
    if ( attr[0]=='*' || !c.attr(attr).empty() ) gbc.push_back(c);
  for(vector<DOMNode*>::iterator i=gbc.begin(); i != gbc.end(); i++)
    fn->removeChild(*i);
}
// ----------------------------------------------------------------------------
DOMNode* XMLFileCatalog::element(CSTR element_name,bool print_err) const {
  DOMNode* node = getDoc(true)->getElementById(XMLStr(element_name));
  if ( !node && print_err ) printError("Cannot find element:"+element_name);
  return node;
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::deleteFID(CSTR fid) const {
  DOMNode *pn = 0, *fn = element(fid);
  if ( fn ) pn = fn->getParentNode();
  if ( pn ) pn->removeChild(fn);
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::registerFID(CSTR fid) const {
  if ( !fid.empty() ) {
    std::pair<DOMElement*, DOMElement*> res = i_registerFID(fid);
    if ( res.first == 0 || res.second == 0 )  {
      printError("Failed to register FID:"+fid);
    }
    return;
  }
  throw runtime_error("XMLFileCatalog> Cannot register LFN for invalid FID:"+fid);
}
// ----------------------------------------------------------------------------
std::pair<DOMElement*,DOMElement*> XMLFileCatalog::i_registerFID(CSTR fid) const {
  if ( !readOnly() )  {
    /// It creates a new node File with name = fid in the XML file catalog
    DOMElement *file = (DOMElement*)element(fid,false), *phyelem = 0, *logelem = 0;
    DOMDocument* doc = getDoc(true);
    if ( !file )  {
      DOMNode* fde = doc->getElementsByTagName(XMLStr("*"))->item(0);
      file = m_doc->createElement(XMLStr("File"));
      file->setAttribute(Attr_ID, XMLStr(fid));
      file->setIdAttribute(Attr_ID, true);
      fde->appendChild(file);
      m_update = true;
    }
    for(XMLCollection c1(file); c1; ++c1 )  {
      char* nam = XMLString::transcode(c1->getNodeName());
      if ( nam == PFNCOLL ) phyelem = c1;
      if ( nam == LFNCOLL ) logelem = c1;
      XMLString::release(&nam);
    }
    if ( !phyelem )  {
      phyelem = doc->createElement(PFNCOLL);
      file->appendChild(phyelem);
      m_update = true;
    }
    if ( !logelem )  {
      logelem = doc->createElement(LFNCOLL);
      file->appendChild(logelem);
      m_update = true;
    }
    return std::make_pair(logelem,phyelem);
  }
  printError("Cannot update readonly catalog!");
  return std::pair<DOMElement*, DOMElement*>(0,0);
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::registerPFN(CSTR fid, CSTR pfn, CSTR ftype) const {
  if ( !fid.empty() )  {
    std::pair<DOMElement*,DOMElement*> res = i_registerFID(fid);
    DOMElement* phyelem = res.second, *fnelem = 0;
    for(XMLCollection c(phyelem); c; ++c )  {
      char* nam = XMLString::transcode(c->getNodeName());
      if ( nam == PFNNODE )  {
        XMLString::release(&nam);
        nam = XMLString::transcode(c->getAttribute(Attr_name));
        if ( nam == pfn )  {
          XMLString::release(&nam);
          fnelem = c;
          break;
        }
      }
      XMLString::release(&nam);
    }
    if ( !fnelem )  {
      fnelem = getDoc(true)->createElement(PFNNODE);
      phyelem->appendChild(fnelem);
      fnelem->setAttribute(Attr_ftype,XMLStr(ftype));
      fnelem->setAttribute(Attr_name,XMLStr(pfn));
      fnelem->setIdAttribute(Attr_name, true);
      m_update = true;
    }
    return;
  }
  throw runtime_error("XMLFileCatalog> Cannot register PFN for invalid FID:"+fid);
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::registerLFN(CSTR fid, CSTR lfn) const {
  if ( !fid.empty() )  {
    std::pair<DOMElement*, DOMElement*> res = i_registerFID(fid);
    DOMElement* logelem = res.first, *fnelem = 0;
    for(XMLCollection c(logelem); c; ++c )  {
      char* nam = XMLString::transcode(c->getNodeName());
      if ( nam == LFNNODE )  {
        XMLString::release(&nam);
        nam = XMLString::transcode(c->getAttribute(Attr_name));
        if ( nam == lfn )  {
          XMLString::release(&nam);
          fnelem = c;
          break;
        }
      }
    }
    if ( !fnelem )  {
      fnelem = getDoc(true)->createElement(LFNNODE);
      logelem->appendChild(fnelem);
      fnelem->setAttribute(Attr_name,XMLStr(lfn));
      fnelem->setIdAttribute(Attr_name, true);
      m_update = true;
    }
    return;
  }
  throw runtime_error("XMLFileCatalog> Cannot register LFN for invalid FID:"+fid);
}
// ----------------------------------------------------------------------------
void XMLFileCatalog::commit()    {
  try {
    if ( dirty() && !readOnly() )  {
      string xmlfile = getfile(true);
      XMLStr ii("LS");
      DOMImplementation *imp = DOMImplementationRegistry::getDOMImplementation(ii);
      XMLFormatTarget   *tar = new LocalFileFormatTarget(xmlfile.c_str());
#if _XERCES_VERSION <= 30000
      DOMWriter         *wr  = imp->createDOMWriter();
      wr->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
      wr->writeNode(tar, *m_doc);
      wr->release();
#else
      DOMLSOutput       *output = imp->createLSOutput();
      output->setByteStream(tar);
      DOMLSSerializer   *wr     = imp->createLSSerializer();
      wr->getDomConfig()->setParameter(XMLStr("format-pretty-print"), true);
      wr->write(m_doc, output);
      output->release();
      wr->release();
#endif
      delete  tar;
    }
  }
  catch ( exception& e )  {
    printError(string("Cannot open output file:")+e.what());
  }
  catch (...)  {
    printError("Unknown IO rrror.");
  }
}
// ----------------------------------------------------------------------------
string XMLFileCatalog::getfile(bool create)   {
  string protocol, path;
  XMLURL xerurl;
  try{
    xerurl   = (const XMLCh*)XMLStr(m_file);
    protocol = _toString(xerurl.getProtocolName());
    path     = _toString(xerurl.getPath());
  }
  catch (const XMLException& e ) {
    printError(_toString(e.getMessage()));
  }
  if ( protocol.empty() )    {
    printError("Missing protocol.");
  }
  else if ( protocol == "http" || protocol == "ftp" )  {
    m_rdOnly = true;
  }
  else if ( protocol == "file" ) {
    m_rdOnly = false;
    struct stat buff;
    int exist = ::stat(path.c_str(),&buff) != -1;
    if ( create && !exist )  {
      MsgStream log(m_msgSvc,"XMLCatalog");
      log << MSG::INFO << "File '" << path << "' does not exist. New file created." << endmsg;
      ofstream out(path.c_str());
      if( !m_rdOnly && out.is_open() ) {
        out << (CSTR)EmptyCatalog << endl;
      }
      else     {
        printError("Problem creating file "+path);
      }
      out.close();
    }
    else if ( exist )  {
      return path;
    }
    else if ( !create )  {
      return "";
    }
  }
  else  {
    printError(protocol + ": protocol not supported.");
  }
  return path;
}
