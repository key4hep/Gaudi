// $Id: PoolDbEvtSelector.h,v 1.3 2004/07/06 13:10:21 mato Exp $
//====================================================================
//	PoolDbEvtSelector.h
//--------------------------------------------------------------------
//
//	Package    : GaudiPoolDb
//
//  Description: The PoolDbEvtSelector component is able
//               to produce a list of event references given a set of "selection
//               criteria".
//
//  Author     : M.Frank
//  Created    : 4/01/99
//
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbEvtSelector.h,v 1.3 2004/07/06 13:10:21 mato Exp $

#ifndef POOLDB_POOLDBEVTSELECTOR_H
#define POOLDB_POOLDBEVTSELECTOR_H 1

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/DataObject.h"
#include "PersistencySvc/ITokenIterator.h"
// C++ include files
#include <list>

// Forward declarations
class IPoolDbMgr;

//--------------------------------------------------------------------
// Event Selector
//--------------------------------------------------------------------

class GAUDI_API PoolDbEvtSelector: public extends1<Service, IEvtSelector> {
public:
  /// IService implementation: Db event selector override
  virtual StatusCode initialize();

  /// IService implementation: Service finalization
  virtual StatusCode finalize();

  /// Create a new event loop context
  /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode createContext(Context*& refpCtxt) const;

  /** Access last item in the iteration
    * @param refContext [IN/OUT] Reference to the Context object.
    */
  virtual StatusCode last(Context& refContext) const;

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
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode createAddress(const Context& refCtxt, IOpaqueAddress*&) const;

  /// Release existing event iteration context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode releaseContext(Context*& refCtxt) const;

  /** Will set a new criteria for the selection of the next list of events and will change
      the state of the context in a way to point to the new list.

      @param cr The new criteria string.
      @param c Reference pointer to the Context object.
   */
  virtual StatusCode resetCriteria(const std::string& cr,Context& c)const;

  /// Service Constructor
  PoolDbEvtSelector( const std::string& name, ISvcLocator* svcloc );

  /// Standard destructor
  virtual ~PoolDbEvtSelector();

protected:
  // Data Members
  std::string                   m_cnvSvcName;
  mutable SmartIF<IPoolDbMgr>   m_dbMgr;
  CLID                          m_rootCLID;
  std::string                   m_rootName;
  std::string                   m_criteria;
};
#endif  // POOLDB_POOLDBEVTSELECTOR_H
