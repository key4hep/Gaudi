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
//====================================================================
// NTuple converter class definition
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
#pragma once

// Include files
#include <RootCnv/RootAddress.h>
#include <RootCnv/RootStatCnv.h>

// Forward declarations
class INTuple;
class TBranch;

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class RootNTupleCnv RootNTupleCnv.h Root/RootNTupleCnv.h
   *
   * NTuple converter class definition for NTuples writted/read using ROOT.
   *
   * Description:
   * Definition of the generic converter for row wise and column wise
   * Ntuples.
   *
   * @author  M.Frank
   * @version 1.0
   */
  class GAUDI_API RootNTupleCnv : public RootStatCnv {
  protected:
    /// Update the transient object: NTuples end here when reading records
    StatusCode i__updateObjRoot( RootAddress* rpA, INTuple* tupl, TTree* tree, RootDataConnection* con );

#ifdef __POOL_COMPATIBILITY
    /// Update the transient object: NTuples end here when reading records
    StatusCode i__updateObjPool( RootAddress* rpA, INTuple* tupl, TTree* tree, RootDataConnection* con );
#endif

  public:
    /// Standard constructor
    /**
     * @param clid        [IN]  Class ID of the object type to be converted.
     * @param db          [IN]  Pointer to POOL database interface
     * @param svc         [IN]  Pointer to Gaudi service locator
     *
     * @return Reference to RootNTupleCnv object
     */
    RootNTupleCnv( long typ, const CLID& clid, ISvcLocator* svc, RootCnvSvc* mgr )
        : RootStatCnv( typ, clid, svc, mgr ) {}

    /** Converter overrides: Convert the transient object
     * to the requested representation.
     *
     * @param pObj        [IN]  Valid pointer to DataObject
     * @param refpAdd     [OUT] Location to store pointer to the
     *                          object's opaque address
     *
     * @return Status code indicating success or failure
     */
    StatusCode createRep( DataObject* pObj, IOpaqueAddress*& refpAdd ) override;

    /// Resolve the references of the converted object.
    /**
     * @param pAdd       [IN]   Valid pointer to the object's opaque address
     * @param pObj       [IN]   Valid pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode fillRepRefs( IOpaqueAddress* pAdd, DataObject* pObj ) override;

    /** Converter overrides: Update the references of an
     * updated transient object.
     *
     * @param pAddress    [IN]  Valid pointer to the object's opaque address
     * @param refpObject [OUT]  Location to store pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject ) override;

    /** Converter overrides: Update the references of an
     * updated transient object.
     *
     * @param pAddress   [IN]   Valid pointer to the object's opaque address
     * @param pObject    [IN]   Valid pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode fillObjRefs( IOpaqueAddress* /* pAddress */, DataObject* /* pObject  */ ) override {
      return StatusCode::SUCCESS;
    }

    /** Update the transient object: NTuples end here when reading records
     *
     * @param pAddress   [IN]   Valid pointer to the object's opaque address
     * @param pObject    [IN]   Valid pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode updateObj( IOpaqueAddress* pAddress, DataObject* pObject ) override;

    /** Converter overrides: Update the references of an
     * updated transient object.
     *
     * @param pAddress   [IN]   Valid pointer to the object's opaque address
     * @param pObject    [IN]   Valid pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode updateObjRefs( IOpaqueAddress* /* pAddress */, DataObject* /* pObject  */ ) override {
      return StatusCode::SUCCESS;
    }

    /** Converter overrides: Update the converted representation
     * of a transient object.
     *
     * @param pAddress   [IN]   Valid pointer to the object's opaque address
     * @param pObject    [IN]   Valid pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode updateRep( IOpaqueAddress* /* pAddress */, DataObject* /* pObject  */ ) override {
      return StatusCode::SUCCESS;
    }

    /** Converter overrides: Update the converted representation of a
     * transient object.
     *
     * @param pAddress   [IN]   Valid pointer to the object's opaque address
     * @param pObject    [IN]   Valid pointer to DataObject
     *
     * @return Status code indicating success or failure
     */
    StatusCode updateRepRefs( IOpaqueAddress* /* pAddress */, DataObject* /* pObject  */ ) override {
      return StatusCode::SUCCESS;
    }
  };
} // namespace Gaudi
