/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	RootTreeEvtSelector.h
//--------------------------------------------------------------------
//
//	Package    : RootCnv
//
//  Description: The RootEvtSelector component is able
//               to produce a list of event references given a set of "selection
//               criteria".
//
//  Author     : M.Frank
//  Created    : 4/01/99
//
//====================================================================

#ifndef GAUDIROOTCNV_ROOTEVTSELECTOR_H
#define GAUDIROOTCNV_ROOTEVTSELECTOR_H 1

// Framework include files
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/Service.h"

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi {

  // Forward declarations
  class RootCnvSvc;

  /** @class RootEvtSelector RootEvtSelector.h GAUDIROOT/RootEvtSelector.h
   *
   *  Concrete event selector implementation to access ROOT files.
   *
   *  @author  M.Frank
   *  @version 1.0
   *  @date    20/12/2009
   */
  class GAUDI_API RootEvtSelector : public extends<Service, IEvtSelector> {

    /// Helper method to issue error messages
    StatusCode error( const std::string& msg ) const;

  public:
    /// Service Constructor
    using extends::extends;

    /// IService implementation: Db event selector override
    StatusCode initialize() override;

    /// IService implementation: Service finalization
    StatusCode finalize() override;

    /// Create a new event loop context
    /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
     *
     * @return StatusCode indicating success or failure
     */
    StatusCode createContext( Context*& refpCtxt ) const override;

    /** Access last item in the iteration
     * @param refContext [IN/OUT] Reference to the Context object.
     */
    StatusCode last( Context& refContext ) const override;

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
     *
     * @return StatusCode indicating success or failure
     */
    StatusCode createAddress( const Context& refCtxt, IOpaqueAddress*& ) const override;

    /// Release existing event iteration context
    /** @param refCtxt   [IN/OUT]  Reference to the context
     *
     * @return StatusCode indicating success or failure
     */
    StatusCode releaseContext( Context*& refCtxt ) const override;

    /** Will set a new criteria for the selection of the next list of events and will change
     *  the state of the context in a way to point to the new list.
     *
     * @param cr The new criteria string.
     * @param c Reference pointer to the Context object.
     */
    StatusCode resetCriteria( const std::string& cr, Context& c ) const override;

  protected:
    // Data Members
    /// Reference to the corresponding conversion service
    mutable RootCnvSvc* m_dbMgr;
    /// Class id of root node to create opaque address
    CLID m_rootCLID = CLID_NULL;

    Gaudi::Property<std::string> m_persName{ this, "EvtPersistencySvc", "EventPersistencySvc",
                                             "Name of the persistency service to search for conversion service" };
    Gaudi::Property<std::string> m_dummy{ this, "DbType", "", "dummy property to fake backwards compatibility" };

    /// Property; Name of the concversion service used to create opaque addresses
    std::string m_cnvSvcName = "Gaudi::RootCnvSvc/RootCnvSvc";
    /// Property: Name of the ROOT entry name
    std::string m_rootName;
    /// Property: File criteria to define item iteration
    std::string m_criteria;
  };
} // namespace Gaudi
#endif // GAUDIROOTCNV_ROOTEVTSELECTOR_H
