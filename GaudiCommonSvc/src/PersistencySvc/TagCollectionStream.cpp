// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IDataSourceMgr.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/GenericAddress.h"
#include "TagCollectionStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT(TagCollectionStream)

static const char SEPARATOR = IDataProviderSvc::SEPARATOR;

// Standard Constructor
TagCollectionStream::TagCollectionStream(const std::string& nam, ISvcLocator* pSvc)
  : OutputStream(nam, pSvc), m_addr{  new GenericAddress() }
{
  declareProperty("AddressLeaf",      m_addrLeaf     = "/Event" );
  declareProperty("AddressColumn",    m_addrColName  = "Address");
  declareProperty("TagCollectionSvc", m_collSvcName  = "NTupleSvc");
  declareProperty("ObjectsFirst",     m_objectsFirst = true);
  declareProperty("Collection",       m_tagName );
}


// Connect address column, if not already connected
StatusCode TagCollectionStream::connectAddress()  {
  NTuplePtr nt(m_collectionSvc, m_tagName);
  if ( !nt )    {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Failed to connect to the tag collection "
        << m_tagName << endmsg;
    return StatusCode::FAILURE;
  }
  m_addrColumn = nt->find(m_addrColName);
  if ( m_addrColumn )  return StatusCode::SUCCESS;
  StatusCode sc = nt->addItem (m_addrColName,m_item);
  if ( sc.isSuccess() )  {
    m_addrColumn = m_item.operator->();
    return sc;
  }
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Failed to add the address column:"
      << m_addrColName << " to the tag collection " << m_tagName
      << endmsg;
  return sc;
}

// initialize data writer
StatusCode TagCollectionStream::initialize() {
  using Parser = Gaudi::Utils::AttribStringParser;
  std::string log_node, log_file, logical_name;
  m_collectionSvc = service(m_collSvcName, true);
  if ( m_collectionSvc )  {
    std::string tmp;
    for(auto attrib: Parser(m_output)) {
      switch( ::toupper(attrib.tag[0]) )    {
      case 'C':
        m_tagName = std::move(attrib.value);
        break;
      case 'A':
        m_addrLeaf = std::move(attrib.value);
        break;
      default:
        tmp += attrib.tag + "='" + attrib.value + "' ";
        break;
      }
    }
    m_output = std::move(tmp);
    std::string::size_type idx = m_tagName[0]==SEPARATOR ? m_tagName.find(SEPARATOR,1) : 0;
    log_node = m_tagName.substr(idx,m_tagName.find(SEPARATOR,idx+1));
    log_file = log_node + " " + m_output + " SHARED='YES'";
  }
  m_addrColumn = nullptr;                           // reset pointer to item column
  auto sc = OutputStream::initialize();            // Now initialize the base class
  if ( sc.isSuccess() )  {
    auto src_mgr = m_collectionSvc.as<IDataSourceMgr>();
    if ( src_mgr )  {
      auto data_mgr = m_collectionSvc.as<IDataManagerSvc>();
      if ( data_mgr )  {
        logical_name = data_mgr->rootName();
        logical_name += SEPARATOR;
        logical_name += log_node;
        m_topLeafName = m_addrLeaf.substr(0,m_addrLeaf.find(SEPARATOR,m_addrLeaf[0]=='/' ? 1 : 0));
        m_isTopLeaf   = m_topLeafName == m_addrLeaf;
        if ( src_mgr->isConnected(logical_name) )  return sc;
        sc = src_mgr->connect(log_file);
        if ( sc.isSuccess() )  return sc;
      }
    }
  }
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Failed to initialize TagCollection Stream." << endmsg;
  return StatusCode::FAILURE;
}

// terminate data writer
StatusCode TagCollectionStream::finalize() {
  MsgStream log(msgSvc(), name());
  StatusCode status = OutputStream::finalize();
  m_collectionSvc.reset();
  m_addrColumn = nullptr;
  return status;
}

// Write tuple data
StatusCode TagCollectionStream::writeTuple() {
  StatusCode sc = m_collectionSvc->writeRecord(m_tagName);
  if ( !sc.isSuccess() )  {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
        << "[Tuple write error]" << endmsg;
  }
  return sc;
}

// Write data objects
StatusCode TagCollectionStream::writeData() {
  StatusCode sc = OutputStream::writeObjects();
  if ( !sc.isSuccess() )  {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
        << "[Object write error]" << endmsg;
  }
  return sc;
}

// Write full event record
StatusCode TagCollectionStream::writeRecord() {
  if ( writeTuple().isSuccess() )  {
    return m_objectsFirst ? StatusCode::SUCCESS : writeData();
  }
  return StatusCode::FAILURE;
}

// Work entry point
StatusCode TagCollectionStream::writeObjects() {
  StatusCode status =  !m_addrColumn ? connectAddress() : StatusCode::SUCCESS;
  if ( status.isSuccess() )  {
    status = m_objectsFirst ? writeData() : StatusCode::SUCCESS;
    if ( status.isSuccess() && !m_addrColName.empty() )  {
      SmartDataPtr<DataObject> top(eventSvc(), m_topLeafName);
      if ( !top )  {
        MsgStream log0(msgSvc(), name());
        log0 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
            << m_topLeafName << " not found." << endmsg;
        return StatusCode::FAILURE;
      }
      IOpaqueAddress* pA = top->registry()->address();
      if ( !pA )  {
        MsgStream log1(msgSvc(), name());
        log1 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
            << m_topLeafName << "'s address not found." << endmsg;
        return StatusCode::FAILURE;
      }
      std::string*    par = (std::string*)m_addr->par();
      unsigned long* ipar = (unsigned long*)m_addr->ipar();
      m_addr->setClID(pA->clID());
      m_addr->setSvcType(pA->svcType());
      par[0]  = pA->par()[0];
      par[1]  = pA->par()[1];
      ipar[0] = pA->ipar()[0];
      ipar[1] = pA->ipar()[1];
      *(IOpaqueAddress**)(m_addrColumn->buffer()) = m_addr.get();
      if ( m_isTopLeaf )  return writeRecord();
      // Handle redirection mode. Normal tag collection mode
      // is the same like leaving this out....
      SmartDataPtr<DataObject> leaf(eventSvc(), m_addrLeaf);
      if ( !leaf )  {
        MsgStream log2(msgSvc(), name());
        log2 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
            << m_addrLeaf << " not found." << endmsg;
        return StatusCode::FAILURE;
      }
      IOpaqueAddress* redir = leaf->registry()->address();
      if ( !redir )  {
        MsgStream log3(msgSvc(), name());
        log3 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
            << m_addrLeaf << "'s address not found." << endmsg;
        return StatusCode::FAILURE;
      }
      par[0]  = redir->par()[0];
      ipar[0] = ~0x0;
      ipar[1] = redir->ipar()[1];
      return writeRecord();
    }
  }
  return status;
}
