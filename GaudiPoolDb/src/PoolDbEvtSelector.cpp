// $Id: PoolDbEvtSelector.cpp,v 1.8 2008/01/17 13:20:51 marcocle Exp $
//====================================================================
//	PoolDbEvtSelector.cpp
//--------------------------------------------------------------------
//
//	Package    : PoolDbEvtSelector
//
//  Description: The PoolDbEvtSelector component is able
//               to produce a list of event references given
//               a set of "selection criteria".
//
//	Author     : M.Frank
//====================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, coming from CORAL
#pragma warning(disable:2259)
#endif

// Include files
#include "GaudiPoolDb/PoolDbEvtSelector.h"
#include "GaudiPoolDb/PoolDbAddress.h"
#include "GaudiPoolDb/IPoolDbMgr.h"

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "POOLCore/Token.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbInstanceCount.h"

DECLARE_SERVICE_FACTORY(PoolDbEvtSelector)

/** @class PoolDbContext
 *
 *  @author  M.Frank
 *  @version 1.0
 */
class PoolDbContext : public IEvtSelector::Context {
public:
  typedef std::list<std::string> ListName;
private:
  const PoolDbEvtSelector*      m_pSelector;
  mutable pool::DbSelect*       m_iterator;
  mutable pool::Token*          m_token;
  ListName                      m_files;
  std::string                   m_criteria;
  ListName::const_iterator      m_fileIterator;
  std::string                   m_currentInput;

public:
  /// Standard constructor
  PoolDbContext(const PoolDbEvtSelector* pSelector);
  /// Standard destructor
  virtual ~PoolDbContext();
  const std::string& currentInput() const {
    return m_currentInput;
  }
  void setCurrentInput(const std::string& v) {
    m_currentInput = v;
  }
  ListName& files() {
    return m_files;
  }
  virtual void* identifier() const {
    return (void*)m_pSelector;
  }
  void setCriteria(const std::string& crit) {
    m_criteria = crit;
  }
  pool::Token* token() const {
    return m_token;
  }
  void setToken(pool::Token* p) {
    pool::releasePtr(m_token);
    m_token = p;
  }
  ListName::const_iterator fileIterator()        {
    return m_fileIterator;
  }
  void setFileIterator(ListName::const_iterator new_iter)  {
    m_fileIterator = new_iter;
  }
  pool::DbSelect* iterator() const {
    return m_iterator;
  }
  void setIterator(pool::DbSelect* new_iter)  {
    pool::deletePtr(m_iterator);
    m_iterator = new_iter;
  }
  pool::DbSelect* selectInput(IPoolDbMgr* mgr,
				                      const std::string& sel,
				                      const std::string& input,
				                      const std::string& cnt);
};


static pool::DbInstanceCount::Counter* s_countPoolDbContext =
  pool::DbInstanceCount::getCounter(typeid(PoolDbContext));

PoolDbContext::~PoolDbContext() {
  pool::deletePtr(m_iterator);
  pool::releasePtr(m_token);
  s_countPoolDbContext->decrement();
}

PoolDbContext::PoolDbContext(const PoolDbEvtSelector* pSelector)
  : m_pSelector(pSelector), m_iterator(0), m_token(0)
{
  s_countPoolDbContext->increment();
}

pool::DbSelect*
PoolDbContext::selectInput( IPoolDbMgr* mgr,
			                      const std::string& sel,
			                      const std::string& input,
			                      const std::string& cnt)
{
  setIterator(mgr->createSelect(sel,input,cnt));
  iterator() ? setCurrentInput(input) : setCurrentInput("");
  return iterator();
}

PoolDbEvtSelector::PoolDbEvtSelector(const std::string& name,
                                     ISvcLocator* svcloc )
  : base_class( name, svcloc),
    m_rootCLID(CLID_NULL)
{
  declareProperty("CnvService",  m_cnvSvcName);
  pool::DbInstanceCount::increment(this);
}

PoolDbEvtSelector::~PoolDbEvtSelector() {
  pool::DbInstanceCount::decrement(this);
}

/// IService implementation: Db event selector override
StatusCode PoolDbEvtSelector::initialize()    {
  // Initialize base class
  StatusCode status = Service::initialize();
  MsgStream log(msgSvc(), name());
  if ( !status.isSuccess() )    {
    log << MSG::ERROR << "Error initializing base class Service!" << endmsg;
    return status;
  }
  // Retrieve conversion service handling event iteration
  m_dbMgr = serviceLocator()->service(m_cnvSvcName);
  if( !m_dbMgr.isValid() ) {
    log << MSG::ERROR
        << "Unable to localize interface IPoolDbMgr from service:"
        << m_cnvSvcName << endmsg;
    return StatusCode::FAILURE;
  }
  // Get DataSvc
  SmartIF<IDataManagerSvc> eds(serviceLocator()->service("EventDataSvc"));
  if( !eds.isValid() ) {
    log << MSG::ERROR
      << "Unable to localize interface IDataManagerSvc "
      << "from service EventDataSvc"
	    << endmsg;
    return status;
  }
  m_rootCLID = eds->rootCLID();
  m_rootName = eds->rootName();
  log << MSG::DEBUG << "Selection root:" << m_rootName
                    << " CLID:" << m_rootCLID << endmsg;
  return status;
}

/// IService implementation: Service finalization
StatusCode PoolDbEvtSelector::finalize()    {
  // Initialize base class
  MsgStream log(msgSvc(), name());
  m_dbMgr = 0; // release
  StatusCode status = Service::finalize();
  if ( !status.isSuccess() )    {
    log << MSG::ERROR << "Error initializing base class Service!" << endmsg;
  }
  return status;
}

StatusCode
PoolDbEvtSelector::createContext(Context*& refpCtxt) const
{
  refpCtxt = new PoolDbContext(this);
  return StatusCode::SUCCESS;
}

/// Access last item in the iteration
StatusCode PoolDbEvtSelector::last(Context& /*refContext*/) const  {
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::next(Context& ctxt) const
{
  PoolDbContext* pCtxt = dynamic_cast<PoolDbContext*>(&ctxt);
  if ( pCtxt != 0 ) {
    pool::DbSelect* it = pCtxt->iterator();
    if ( !it ) {
      PoolDbContext::ListName::const_iterator fileit = pCtxt->fileIterator();
      if ( pCtxt->currentInput().length() > 0 ) {
        pCtxt->setToken(0);
        m_dbMgr->disconnect(pCtxt->currentInput());
      }
      if ( fileit != pCtxt->files().end() ) {
        std::string input = *fileit;
        pCtxt->setFileIterator(++fileit);
        it = pCtxt->selectInput(m_dbMgr, "", input, m_rootName);
      }
    }
    if ( it ) {
      pool::Token* token = 0;
      if ( it->next(token).isSuccess() )  {
        pCtxt->setToken(token);
        if( 0 == pCtxt->token() ) {
          pCtxt->setIterator(0);
          return next(ctxt);
        }
        return StatusCode::SUCCESS;
      }
      return StatusCode::FAILURE;
    }
  }
  if ( pCtxt ) {
    if ( pCtxt->currentInput().length() > 0 ) {
      m_dbMgr->disconnect( pCtxt->currentInput() );
    }
    pCtxt->setToken(0);
    pCtxt->setIterator(0);
    pCtxt->setCurrentInput("");
    pCtxt->setFileIterator(pCtxt->files().end());
  }
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::next(Context& ctxt,int jump) const
{
  if ( jump > 0 ) {
    for ( int i = 0; i < jump; ++i ) {
      StatusCode status = next(ctxt);
      if ( !status.isSuccess() ) {
        return status;
      }
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::previous(Context& /* ctxt */) const
{
  MsgStream log(msgSvc(), name());
  log << MSG::FATAL
    << " EventSelector Iterator, operator -- not supported " << endmsg;
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::previous(Context& ctxt, int jump) const
{
  if ( jump > 0 ) {
    for ( int i = 0; i < jump; ++i ) {
      StatusCode status = previous(ctxt);
      if ( !status.isSuccess() ) {
        return status;
      }
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::rewind(Context& ctxt) const
{
  PoolDbContext* pCtxt = dynamic_cast<PoolDbContext*>(&ctxt);
  if ( pCtxt ) {
    if ( pCtxt->currentInput().length() > 0 ) {
      m_dbMgr->disconnect( pCtxt->currentInput() );
    }
    pCtxt->setToken(0);
    pCtxt->setIterator(0);
    pCtxt->setCurrentInput("");
    pCtxt->setFileIterator(pCtxt->files().begin());
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::createAddress(const Context& ctxt, IOpaqueAddress*& pAddr) const
{
  const PoolDbContext* pctxt = dynamic_cast<const PoolDbContext*>(&ctxt);
  if ( pctxt ) {
    pool::Token* tok = pctxt->token();
    if ( 0 != tok )  {
      PoolDbAddress* pA = 0;
      if ( m_dbMgr->createAddress(tok, &pA).isSuccess() )  {
        pAddr = pA;
        return StatusCode::SUCCESS;
      }
      pool::releasePtr(tok);
    }
  }
  pAddr = 0;
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::releaseContext(Context*& ctxt) const
{
  PoolDbContext* pCtxt = dynamic_cast<PoolDbContext*>(ctxt);
  if ( pCtxt ) {
    pool::deletePtr(pCtxt);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode
PoolDbEvtSelector::resetCriteria(const std::string& criteria,
                                 Context& context)  const
{
  MsgStream log(msgSvc(), name());
  PoolDbContext* ctxt = dynamic_cast<PoolDbContext*>(&context);
  std::string db, typ, item, sel, stmt, aut, addr;
  if ( ctxt )  {
    if ( criteria.substr(0,5) == "FILE " )  {
      // The format for the criteria is:
      //        FILE  filename1, filename2 ...
      db = criteria.substr(5);
    }
    else  {
      Tokenizer tok(true);
      tok.analyse(criteria," ","","","=","'","'");
      for(Tokenizer::Items::iterator i=tok.items().begin(); i!=tok.items().end();i++) {
        std::string tmp = (*i).tag().substr(0,3);
        if(tmp=="DAT")  {
          db = (*i).value();
        }
        if(tmp=="OPT")   {
          if((*i).value() != "REA")   {
            log << MSG::ERROR << "Option:\"" << (*i).value() << "\" not valid" << endmsg;
            return StatusCode::FAILURE;
          }
        }
        if (tmp=="TYP") {
          typ = (*i).value();
        }
        if(tmp=="ADD")  {
          item = (*i).value();
        }
        if(tmp=="SEL")  {
          sel = (*i).value();
        }
        if(tmp=="FUN")  {
          stmt = (*i).value();
        }
        if(tmp=="AUT")  {
          aut = (*i).value();
        }
        if(tmp=="COL")  {
          addr = (*i).value();
        }
      }
    }
    // It's now time to parse the criteria for the event selection
    // The format for the criteria is:
    //        FILE  filename1, filename2 ...
    //        JOBID number1-number2, number3, ...
    std::string rest = db;
    ctxt->files().clear();
    while(true)  {
      int ipos = rest.find_first_not_of(" ,");
      if (ipos == -1 ) break;
      rest = rest.substr(ipos, std::string::npos);// remove blanks before
      int lpos  = rest.find_first_of(" ,");       // locate next blank
      ctxt->files().push_back( rest.substr(0,lpos )); // insert in list
      if (lpos == -1 ) break;
      rest = rest.substr(lpos, std::string::npos);// get the rest
    }
    ctxt->setFileIterator(ctxt->files().begin());
    ctxt->setCriteria(criteria);
    return StatusCode::SUCCESS;
  }
  log << MSG::ERROR << "Invalid iteration context." << endmsg;
  return StatusCode::FAILURE;
}

