/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/GenericAddress.h>
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/Service.h>

#include <optional>

class INTuple;
class INTupleSvc;
class IAddressCreator;
class EventSelectorDataStream;
template <class TYPE>
class EventIterator;

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
class EventCollectionSelector : public extends<Service, IEvtSelector> {
public:
  class MyContextType : public IEvtSelector::Context {
  public:
    std::string                                  criteria;
    NTuple::Tuple*                               tuple;
    std::optional<NTuple::Item<IOpaqueAddress*>> item{};
    IOpaqueAddress*                              addressBuffer;
    MyContextType( MyContextType* ctxt = nullptr ) {
      addressBuffer = new GenericAddress();
      addressBuffer->addRef();
      tuple = ( ctxt ) ? ctxt->tuple : nullptr;
      if ( ctxt ) item.emplace( *ctxt->item );
    }
    MyContextType( const MyContextType& ctxt ) : IEvtSelector::Context( ctxt ) {
      addressBuffer = new GenericAddress();
      addressBuffer->addRef();
      tuple = ctxt.tuple;
      item.emplace( *ctxt.item );
    }
    ~MyContextType() override { addressBuffer->release(); }
    MyContextType& operator=( const MyContextType& ) = delete;

    void* identifier() const override { return (void*)addressBuffer; }
    void  setAddress( IOpaqueAddress* pAddr );
  };

protected:
  /// Reference to Tuple service
  mutable SmartIF<INTupleSvc>      m_tupleSvc;
  mutable SmartIF<IAddressCreator> m_pAddrCreator;

  // Properties
  Gaudi::Property<std::string> m_tupleSvcName{ this, "CnvService", "EvtTupleSvc",
                                               "name of the event collection service" };
  Gaudi::Property<std::string> m_authentication{ this, "Authentication", "", "authentication string (if needed)" };
  Gaudi::Property<std::string> m_cntName{ this, "Container", "B2PiPi", "container name" };
  Gaudi::Property<std::string> m_itemName{ this, "Item", "Address", "item name" };
  Gaudi::Property<std::string> m_criteria{ this, "Criteria", "", "criteria" };
  Gaudi::Property<std::string> m_database{ this, "DB", "", "datafile name" };
  Gaudi::Property<std::string> m_dbType{ this, "DbType", "", "database type identifier" };
  Gaudi::Property<std::string> m_dbSvc{ this, "DbService", "", "database service (exclusive property with db type)" };
  Gaudi::Property<std::string> m_statement{ this, "Function", "NTuple::Selector", "selector name" };

public:
  /// Service override: Initialize service
  StatusCode initialize() override;
  /// Service override: Finalize Service
  StatusCode finalize() override;

  /// Create a new event loop context
  /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode createContext( Context*& refpCtxt ) const override;

  /// Get next iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode next( Context& refCtxt ) const override;

  /// Get next iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode next( Context& refCtxt, int jump ) const override;

  /// Get previous iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
   * @param jump      [IN]      Number of events to be skipped
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode previous( Context& refCtxt ) const override;

  /// Get previous iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
   * @param jump      [IN]      Number of events to be skipped
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode previous( Context& refCtxt, int jump ) const override;

  /// Rewind the dataset
  /** @param refCtxt   [IN/OUT]  Reference to the context
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode rewind( Context& refCtxt ) const override;

  /// Create new Opaque address corresponding to the current record
  /** @param refCtxt   [IN/OUT]  Reference to the context
   * @param refpAddr  [OUT]     Reference to address pointer
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode createAddress( const Context& refCtxt, IOpaqueAddress*& refpAddr ) const override;

  /// Release existing event iteration context
  /** @param refCtxt   [IN/OUT]  Reference to the context
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode releaseContext( Context*& refCtxt ) const override;

  /** Will set a new criteria for the selection of the next list of events and will change
   * the state of the context in a way to point to the new list.
   *
   * @param cr The new criteria string.
   * @param c  Reference pointer to the Context object.
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode resetCriteria( const std::string& cr, Context& c ) const override;

  /** Access last item in the iteration
   * @param refCtxt [IN/OUT] Reference to the Context object.
   *
   * @return StatusCode indicating success or failure
   */
  StatusCode last( Context& refCtxt ) const override;

  /// Connect collection to selector
  virtual StatusCode connectCollection( MyContextType* ctxt ) const;

  /// Connect collection's data source to selector
  virtual StatusCode connectDataSource( const std::string& db, const std::string& typ ) const;
  /// Connect to existing N-tuple
  virtual StatusCode connectTuple( const std::string& nam, const std::string& itName, NTuple::Tuple*& tup,
                                   std::optional<NTuple::Item<IOpaqueAddress*>>& item ) const;
  /// Connect selection statement to refine data access
  virtual StatusCode connectStatement( const std::string& typ, const std::string& crit, INTuple* tuple ) const;
  /// Read next record of the N-tuple
  virtual StatusCode getNextRecord( NTuple::Tuple* tuple ) const;
  /// Read next record of the N-tuple
  virtual StatusCode getPreviousRecord( NTuple::Tuple* tuple ) const;

  /// inherit constructor
  using extends::extends;
};
