//====================================================================
//	EventCollectionSelector.h
//--------------------------------------------------------------------
//
//	Package    : EventCollectionSelector  (LHCb Event Selector Package)
//
//	Author     : M.Frank
//  Created    : 4/10/00
//
//====================================================================
#ifndef GAUDISVC_EventCollectionSelector_EventCollectionSelector_H
#define GAUDISVC_EventCollectionSelector_EventCollectionSelector_H 1

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/NTuple.h"

// Forward declarations
class INTuple;
class INTupleSvc;
class IAddressCreator;
class EventSelectorDataStream;
template <class TYPE> class EventIterator;

/** Definition of class EventCollectionSelector

    Basic event selector service. The event selector service
    itself is able to connect other services to attached streams.

    History:
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    | 3/10/00 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
   @author Markus Frank
   @version 1.0
*/
class EventCollectionSelector : public extends1<Service, IEvtSelector> {
public:
  class MyContextType : public IEvtSelector::Context {
  public:
    std::string                    criteria;
    NTuple::Tuple*                 tuple;
    NTuple::Item<IOpaqueAddress*>* item;
    IOpaqueAddress*                addressBuffer;
    MyContextType(NTuple::Tuple* t, NTuple::Item<IOpaqueAddress*>* i)    {
      addressBuffer = new GenericAddress();
      addressBuffer->addRef();
      tuple = t;
      item = i;
    }
    MyContextType(MyContextType* ctxt=0)  {
      addressBuffer = new GenericAddress();
      addressBuffer->addRef();
      tuple = (ctxt) ? ctxt->tuple : 0;
      item  = (ctxt) ? ctxt->item : 0;
    }
    MyContextType(const MyContextType& ctxt)
      : IEvtSelector::Context(ctxt)
    {
      addressBuffer = new GenericAddress();
      addressBuffer->addRef();
      tuple = ctxt.tuple;
      item  = ctxt.item;
    }
    virtual ~MyContextType()    {
      addressBuffer->release();
    }
    virtual void* identifier() const {
      return (void*)addressBuffer;
    }
    void setAddress(IOpaqueAddress* pAddr);
  };
protected:
  /// Reference to Tuple service
  mutable SmartIF<INTupleSvc>      m_tupleSvc;
  mutable SmartIF<IAddressCreator> m_pAddrCreator;
  /// Name of the event collection service name
  std::string              m_tupleSvcName;
  /// Authentication string (if needed)
  std::string              m_authentication;
  /// Container name
  std::string              m_cntName;
  /// Item name
  std::string              m_itemName;
  /// Criteria
  std::string              m_criteria;
  /// Datafile name
  std::string              m_database;
  /// Database type identifier
  std::string              m_dbType;
  /// Database service (exclusive property with db type)
  std::string              m_dbSvc;
  /// Selector name
  std::string              m_statement;
public:

  /// Service override: Initialize service
  virtual StatusCode initialize();
  /// Service override: Finalize Service
  virtual StatusCode finalize();

  /// Create a new event loop context
  /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode createContext(Context*& refpCtxt) const;

  /// Get next iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode next(Context& refCtxt) const;

  /// Get next iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode next(Context& refCtxt,int jump) const;

  /// Get previous iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param jump      [IN]      Number of events to be skipped
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode previous(Context& refCtxt) const;

  /// Get previous iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param jump      [IN]      Number of events to be skipped
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode previous(Context& refCtxt,int jump) const;

  /// Rewind the dataset
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode rewind(Context& refCtxt) const;

  /// Create new Opaque address corresponding to the current record
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param refpAddr  [OUT]     Reference to address pointer
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode createAddress(const Context& refCtxt, IOpaqueAddress*& refpAddr) const;

  /// Release existing event iteration context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode releaseContext(Context*& refCtxt) const;

  /** Will set a new criteria for the selection of the next list of events and will change
    * the state of the context in a way to point to the new list.
    *
    * @param cr The new criteria string.
    * @param c  Reference pointer to the Context object.
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode resetCriteria(const std::string& cr,Context& c)const;

  /** Access last item in the iteration
    * @param refCtxt [IN/OUT] Reference to the Context object.
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode last(Context& refCtxt) const;

  /// Connect collection to selector
  virtual StatusCode connectCollection(MyContextType* ctxt) const;

  /// Connect collection's data source to selector
  virtual StatusCode connectDataSource(const std::string& db, const std::string& typ) const;
  /// Connect to existing N-tuple
  virtual StatusCode connectTuple(const std::string& nam, const std::string& itName, NTuple::Tuple*& tup, NTuple::Item<IOpaqueAddress*>*& item) const;
  /// Connect selection statement to refine data access
  virtual StatusCode connectStatement(const std::string& typ, const std::string& crit, INTuple* tuple) const;
  /// Read next record of the N-tuple
  virtual StatusCode getNextRecord(NTuple::Tuple* tuple)  const;
  /// Read next record of the N-tuple
  virtual StatusCode getPreviousRecord(NTuple::Tuple* tuple)  const;

  /// Standard Constructor
  EventCollectionSelector( const std::string& name, ISvcLocator* svcloc );
  /// Standard Destructor
  virtual ~EventCollectionSelector();
};

#endif  // GAUDISVC_EventCollectionSelector_EventCollectionSelector_H
