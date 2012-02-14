//$Id: EventCollectionSelector.cpp,v 1.16 2007/01/09 16:54:47 hmd Exp $

// Include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataSourceMgr.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/ISelectStatement.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/EventSelectorDataStream.h"

#include "EventCollectionSelector.h"

#include <memory>

DECLARE_SERVICE_FACTORY(EventCollectionSelector)

/** @class EventCollectionContext
 *
 *  @author  M.Frank
 *  @version 1.0
 */
class EventCollectionContext : public IEvtSelector::Context {
public:
  typedef std::list<std::string> ListName;
private:
  GenericAddress*                m_pAddressBuffer;
  const EventCollectionSelector* m_pSelector;
  ListName                      m_files;
  std::string                   m_criteria;
  ListName::const_iterator      m_fileIterator;
  std::string                   m_currentInput;

public:
  /// Standard constructor
  EventCollectionContext(const EventCollectionSelector* pSelector);
  /// Standard destructor
  virtual ~EventCollectionContext();
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
  ListName::const_iterator fileIterator()        {
    return m_fileIterator;
  }
  void setFileIterator(ListName::const_iterator new_iter)
  {  m_fileIterator = new_iter;   }
};

EventCollectionContext::~EventCollectionContext() {
  m_pAddressBuffer->release();
}

EventCollectionContext::EventCollectionContext(const EventCollectionSelector* pSelector)
  : m_pSelector(pSelector)
{
  m_pAddressBuffer = new GenericAddress();
  m_pAddressBuffer->addRef();
}

// Standard constructor
EventCollectionSelector::EventCollectionSelector(const std::string& name, ISvcLocator* svcloc )
  : base_class(name, svcloc)
{
  declareProperty("CnvService",    m_tupleSvcName = "EvtTupleSvc");
  declareProperty("Authentication",m_authentication= "");
  declareProperty("Container",     m_cntName  = "B2PiPi");
  declareProperty("Item",          m_itemName = "Address");
  declareProperty("Criteria",      m_criteria = "");
  declareProperty("DB",            m_database = "");
  declareProperty("DbType",        m_dbType   = "");
  declareProperty("DbService",     m_dbSvc    = "");
  declareProperty("Function",      m_statement= "NTuple::Selector");
}

EventCollectionSelector::~EventCollectionSelector()   {
}

// IService implementation: Db event selector override
StatusCode EventCollectionSelector::initialize()    {
  // Initialize base class
  StatusCode status = Service::initialize();
  MsgStream log(msgSvc(), name());
  if ( !status.isSuccess() )    {
    log << MSG::ERROR << "Error initializing base class Service!" << endmsg;
    return status;
  }
  m_pAddrCreator = serviceLocator()->service("EventPersistencySvc");
  if(!m_pAddrCreator.isValid()) {
    log << MSG::ERROR << "Unable to locate IAddressCreator interface of " << "EventPersistencySvc" << endmsg;
    return status;
  }
  m_tupleSvc = serviceLocator()->service(m_tupleSvcName);
  if( !m_tupleSvc.isValid() )   {
    log << MSG::ERROR << "Unable to locate INTupleSvc interface of " << m_tupleSvcName << endmsg;
    return status;
  }
  return status;
}

// Connect collection to selector
StatusCode
EventCollectionSelector::connectDataSource(const std::string& db, const std::string& /* typ */ )  const {
  StatusCode status = StatusCode::FAILURE;
  SmartIF<IDataSourceMgr> svc(m_tupleSvc);
  if ( svc.isValid( ) && db.length() > 0 )   {
    std::string ident = name() + ' ';
    ident += "DATAFILE='" + m_database.substr(5,m_database.length()) + "' ";
    if ( !m_dbSvc.empty() )
      ident += "SVC='" + m_dbSvc + "' ";
    else
      ident += "TYP='" + m_dbType + "' ";
    ident += "OPT='READ' ";
    if ( m_authentication.length() > 0 )    {
      ident += "AUTH='" + m_authentication + "' ";
    }
    status = svc->connect(ident);
  }
  return status;
}

/// Connect to existing N-tuple
StatusCode
EventCollectionSelector::connectTuple(const std::string& nam, const std::string& itName, NTuple::Tuple*& tup, NTuple::Item<IOpaqueAddress*>*& item)  const {
  std::string top = "/NTUPLES/" + name() + '/' + nam;
  StatusCode status = m_tupleSvc->retrieveObject(top, (DataObject*&)tup);
  if ( status.isSuccess() )    {
    item = new NTuple::Item<IOpaqueAddress*>();
    status = tup->item(itName, *item);
    if ( status.isSuccess() )   {
      return status;
    }
    else    {
      MsgStream log(msgSvc(), name());
      log << MSG::ERROR << "Item " << itName << " is not part of the collection:" << top << endmsg;
    }
    delete item;
    item = 0;
  }
  else  {
    MsgStream err(msgSvc(), name());
    err << MSG::ERROR << "Cannot connect to collection:" << top << endmsg;
  }
  tup = 0;
  return status;
}

/// Connect selection statement to refine data access
StatusCode
EventCollectionSelector::connectStatement(const std::string& typ, const std::string& crit, INTuple* tuple)  const {
  std::string seltyp = typ;
  if ( seltyp.length() > 0 || crit.length() > 0 )   {
    if ( crit.length() > 0 && seltyp.length() == 0 ) seltyp = "NTuple::Selector";
    SmartIF<ISelectStatement> stmt(ROOT::Reflex::PluginService::Create<IInterface*>(seltyp, (IInterface*)serviceLocator()));
    if ( stmt.isValid( ) )    {
      if ( crit.length() > 0 ) stmt->setCriteria(crit);
      tuple->attachSelector(stmt).ignore();
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// Read next record of the N-tuple
StatusCode EventCollectionSelector::getNextRecord(NTuple::Tuple* tuple)   const {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != tuple )   {
    do {
      status = m_tupleSvc->readRecord(tuple);
      if ( status.isSuccess() )   {
        ISelectStatement* statement = tuple->selector();
        bool use_it = (statement) ? (*statement)(tuple) : true;
        if ( use_it )   {
          return status;
        }
      }
    } while ( status.isSuccess() );
  }
  return status;
}

/// Read previous record of the N-tuple
StatusCode EventCollectionSelector::getPreviousRecord(NTuple::Tuple* tuple )  const  {
  StatusCode status = StatusCode::FAILURE;
  if ( tuple )  {
    IRegistry* pReg = tuple->registry();
    if ( pReg )  {
      IOpaqueAddress* pAddr = pReg->address();
      if ( pAddr )  {
        long* ip = (long*)pAddr->ipar();
        do {
          if ( ip[1] > 1 )  {
            ip[1] -= 2;
            status = m_tupleSvc->readRecord(tuple);
            if ( status.isSuccess() )   {
              ISelectStatement* statement = tuple->selector();
              bool use_it = (statement) ? (*statement)(tuple) : true;
              if ( use_it )   {
                return status;
              }
            }
          }
          else  {
            return StatusCode::FAILURE;
          }
        } while ( status.isSuccess() );
      }
    }
  }
  return StatusCode::FAILURE;
}

// Connect collection to selector
StatusCode EventCollectionSelector::connectCollection(MyContextType* ctxt) const
{
  if ( ctxt )  {
    StatusCode status = connectDataSource(m_database, m_dbType);
    if ( status.isSuccess() )   {
      status = connectTuple(m_cntName, m_itemName, ctxt->tuple, ctxt->item);
      if ( status.isSuccess() )   {
        status = connectStatement(m_statement, m_criteria, ctxt->tuple);
        if ( status.isSuccess() )   {
          *(ctxt->item) = 0;
          return status;
        }
      }
    }
    return status;
  }
  return StatusCode::FAILURE;
}

// Finalize service
StatusCode EventCollectionSelector::finalize()    {
  // release services
  m_pAddrCreator = 0;
  m_tupleSvc = 0;
  return Service::finalize();
}

/// Create a new event loop context
StatusCode
EventCollectionSelector::createContext(Context*& refpCtxt) const
{
  refpCtxt = 0;
  std::auto_ptr<MyContextType> ctxt(new MyContextType());
  StatusCode status = connectCollection(ctxt.get());
  if( !status.isSuccess() )  {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Unable to connect Collection file \"" << m_database << "\"" << endmsg;
    return status;
  }
  refpCtxt = ctxt.release();
  return StatusCode::SUCCESS;
}

/// Get next iteration item from the event loop context
StatusCode
EventCollectionSelector::next(Context& refCtxt) const
{
  return next(refCtxt, 1);
}

/// Get next iteration item from the event loop context, but skip jump elements
StatusCode
EventCollectionSelector::next(Context& refCtxt,int jump) const
{
  MyContextType *ctxt  = dynamic_cast<MyContextType*>(&refCtxt);
  if ( ctxt )    {
    *(ctxt->item) = ctxt->addressBuffer;
    StatusCode sc = StatusCode::SUCCESS;
    for ( int i=0; i<jump && sc.isSuccess(); ++i ) {
      sc = getNextRecord(ctxt->tuple);
    }
    return sc;
  }
  return StatusCode::FAILURE;
}

/// Get previous iteration item from the event loop context
StatusCode
EventCollectionSelector::previous(Context& refCtxt) const
{
  return previous(refCtxt, 1);
}

/// Get previous iteration item from the event loop context, but skip jump elements
StatusCode
EventCollectionSelector::previous(Context& refCtxt,int jump) const
{
  MyContextType *ctxt  = dynamic_cast<MyContextType*>(&refCtxt);
  if ( ctxt )    {
    *(ctxt->item) = ctxt->addressBuffer;
    StatusCode sc = StatusCode::SUCCESS;
    for ( int i=0; i<jump && sc.isSuccess(); ++i ) {
      sc = getPreviousRecord(ctxt->tuple);
    }
    return sc;
  }
  return StatusCode::FAILURE;
}

/// Rewind the dataset
StatusCode
EventCollectionSelector::rewind(Context& /* refCtxt */ ) const
{
  return StatusCode::FAILURE;
}

/// Create new Opaque address corresponding to the current record
StatusCode
EventCollectionSelector::createAddress(const Context& refCtxt, IOpaqueAddress*& refpAddr) const
{
  const MyContextType* ctxt = dynamic_cast<const MyContextType*>(&refCtxt);
  if ( ctxt )   {
    IOpaqueAddress* pA = *(ctxt->item);
    if ( pA )  {
      IOpaqueAddress* pAddress = 0;
      StatusCode status = m_pAddrCreator->createAddress(pA->svcType(),
                                                        pA->clID(),
                                                        pA->par(),
                                                        pA->ipar(),
                                                        pAddress);
      if ( status.isSuccess() )   {
        refpAddr = pAddress;
        return StatusCode::SUCCESS;
      }
      else    {
        MsgStream log(msgSvc(), name());
        log << MSG::ERROR << "Failed to access " << pA->par()[0]
            << ":" << pA->par()[1]
            << " SvcTyp:" << long(pA->svcType())
            << " CLID:"   << pA->clID()
            << endmsg;
      }
    }
  }
  return StatusCode::FAILURE;
}

/// Release existing event iteration context
StatusCode
EventCollectionSelector::releaseContext(Context*& refCtxt) const
{
  MyContextType *ctxt  = dynamic_cast<MyContextType*>(refCtxt);
  if ( ctxt )   {
    delete ctxt;
    ctxt = 0;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Will set a new criteria for the selection of the next list of events and will change
/// the state of the context in a way to point to the new list.
StatusCode
EventCollectionSelector::resetCriteria(const std::string& cr,Context& refCtxt) const
{
  MyContextType *ctxt  = dynamic_cast<MyContextType*>(&refCtxt);
  if ( ctxt )   {
    ctxt->criteria = cr;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Access last item in the iteration
StatusCode
EventCollectionSelector::last(Context& /* refCtxt */ ) const
{
  return StatusCode::FAILURE;
}
