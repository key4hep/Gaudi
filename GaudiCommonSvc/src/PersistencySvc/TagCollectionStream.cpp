// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/Tokenizer.h"
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
  : OutputStream(nam, pSvc), m_addrColumn(0),
    m_isTopLeaf(false), m_collectionSvc(0)
{
  declareProperty("AddressLeaf",      m_addrLeaf     = "/Event" );
  declareProperty("AddressColumn",    m_addrColName  = "Address");
  declareProperty("TagCollectionSvc", m_collSvcName  = "NTupleSvc");
  declareProperty("ObjectsFirst",     m_objectsFirst = true);
  declareProperty("Collection",       m_tagName );
  m_addr = new GenericAddress();
}

// Standard Destructor
TagCollectionStream::~TagCollectionStream()   {
  delete m_addr;
}

// Connect address column, if not already connected
StatusCode TagCollectionStream::connectAddress()  {
	MsgStream log(msgSvc(), name());
  NTuplePtr nt(m_collectionSvc, m_tagName);
  if ( nt )    {
    m_addrColumn = nt->find(m_addrColName);
    if ( 0 == m_addrColumn )  {
      StatusCode sc = nt->addItem (m_addrColName,m_item);
      if ( sc.isSuccess() )  {
        m_addrColumn = m_item.operator->();
        return sc;
      }
      log << MSG::ERROR << "Failed to add the address column:"
          << m_addrColName << " to the tag collection " << m_tagName
          << endmsg;
      return sc;
    }
    return StatusCode::SUCCESS;
  }
  log << MSG::ERROR << "Failed to connect to the tag collection "
      << m_tagName << endmsg;
  return StatusCode::FAILURE;
}

// initialize data writer
StatusCode TagCollectionStream::initialize() {
  std::string log_node, log_file, logical_name;
  StatusCode sc = service(m_collSvcName, m_collectionSvc, true);
  if ( sc.isSuccess() )  {
    Tokenizer tok(true);
    tok.analyse(m_output, " ", "", "", "=", "'", "'");
    m_output = "";
    for(Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); ++i)   {
      const std::string& tag = (*i).tag();
      const std::string& val = (*i).value();
      switch( ::toupper(tag[0]) )    {
      case 'C':
        m_tagName = val;
        break;
      case 'A':
        m_addrLeaf = val;
        break;
      default:
        m_output += tag + "='" + val + "' ";
        break;
      }
    }
    std::string::size_type idx = m_tagName[0]==SEPARATOR ? m_tagName.find(SEPARATOR,1) : 0;
    log_node = m_tagName.substr(idx,m_tagName.find(SEPARATOR,idx+1));
    log_file = log_node + " " + m_output + " SHARED='YES'";
  }
  m_addrColumn = 0;                           // reset pointer to item column
  sc = OutputStream::initialize();            // Now initialize the base class
  if ( sc.isSuccess() )  {
    SmartIF<IDataSourceMgr> src_mgr(m_collectionSvc);
    if ( src_mgr.isValid() )  {
      SmartIF<IDataManagerSvc> data_mgr(m_collectionSvc);
      if ( data_mgr.isValid() )  {
        logical_name = data_mgr->rootName();
        logical_name += SEPARATOR;
        logical_name += log_node;
        m_topLeafName = m_addrLeaf.substr(0,m_addrLeaf.find(SEPARATOR,m_addrLeaf[0]=='/' ? 1 : 0));
        m_isTopLeaf   = m_topLeafName == m_addrLeaf;
        if ( src_mgr->isConnected(logical_name) )  {
          return sc;
        }
        sc = src_mgr->connect(log_file);
        if ( sc.isSuccess() )  {
          return sc;
        }
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
  if ( m_collectionSvc ) m_collectionSvc->release();
  m_collectionSvc = 0;
  m_addrColumn = 0;
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
  StatusCode status = m_addrColumn == 0 ? connectAddress() : StatusCode::SUCCESS;
  if ( status.isSuccess() )  {
    status = m_objectsFirst ? writeData() : StatusCode::SUCCESS;
    if ( status.isSuccess() )  {
      if ( !m_addrColName.empty() )  {
        SmartDataPtr<DataObject> top(eventSvc(), m_topLeafName);
        if ( top != 0 )  {
          IOpaqueAddress* pA = top->registry()->address();
          if ( pA != 0 )  {
            std::string*    par = (std::string*)m_addr->par();
            unsigned long* ipar = (unsigned long*)m_addr->ipar();
            m_addr->setClID(pA->clID());
            m_addr->setSvcType(pA->svcType());
            par[0]  = pA->par()[0];
            par[1]  = pA->par()[1];
            ipar[0] = pA->ipar()[0];
            ipar[1] = pA->ipar()[1];
            *(IOpaqueAddress**)(m_addrColumn->buffer()) = m_addr;
            if ( m_isTopLeaf )  {
              return writeRecord();
            }
            // Handle redirection mode. Normal tag collection mode
            // is the same like leaving this out....
            SmartDataPtr<DataObject> leaf(eventSvc(), m_addrLeaf);
            if ( 0 != leaf )  {
              IOpaqueAddress* redir = leaf->registry()->address();
              if ( redir )  {
                par[0]  = redir->par()[0];
                ipar[0] = ~0x0;
                ipar[1] = redir->ipar()[1];
                return writeRecord();
              }
              MsgStream log3(msgSvc(), name());
              log3 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
                  << m_addrLeaf << "'s address not found." << endmsg;
              return StatusCode::FAILURE;
            }
            MsgStream log2(msgSvc(), name());
            log2 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
                << m_addrLeaf << " not found." << endmsg;
            return StatusCode::FAILURE;
          }
          MsgStream log1(msgSvc(), name());
          log1 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
              << m_topLeafName << "'s address not found." << endmsg;
          return StatusCode::FAILURE;
        }
        MsgStream log0(msgSvc(), name());
        log0 << MSG::ERROR << "Failed to write tag collection " << m_tagName << ". "
            << m_topLeafName << " not found." << endmsg;
        return StatusCode::FAILURE;
      }
    }
  }
  return status;
}
