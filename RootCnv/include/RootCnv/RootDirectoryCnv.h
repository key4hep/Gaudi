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
//------------------------------------------------------------------------------
// Definition of class :  RootDirectoryCnv
//--------------------------------------------------------------------
//
//  Package    : DbCnv/NTuples ( The LHCb Offline System)
//  Author     : M.Frank
//
//------------------------------------------------------------------------------
#pragma once

// Include files
#include <RootCnv/RootStatCnv.h>

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class RootDirectoryCnv RootDirectoryCnv.h src/RootDirectoryCnv.h
   *
   * Description:
   * NTuple directory converter class definition
   * Definition of the converter to manage the
   * directories in an database representing N-Tuples.
   *
   * @author  M.Frank
   * @version 1.0
   */
  class GAUDI_API RootDirectoryCnv : public RootStatCnv {
  public:
    /** Initializing Constructor
     * @param      typ      [IN]     Concrete storage type of the converter
     * @param      clid     [IN]     Class identifier of the object
     * @param      svc      [IN]     Pointer to service locator object
     *
     * @return Reference to RootBaseCnv object
     */
    RootDirectoryCnv( long typ, const CLID& clid, ISvcLocator* svc, RootCnvSvc* mgr );

    /** Converter overrides: Create transient object from persistent data
     *
     * @param    pAddr       [IN]   Pointer to object address.
     * @param    refpObj     [OUT]  Location to pointer to store data object
     *
     * @return Status code indicating success or failure.
     */
    StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& refpObj ) override;

    /** Converter overrides: Resolve the references of the transient object.
     *
     * @param    pAddr       [IN]   Pointer to object address.
     * @param    pObj        [IN]   Pointer to data object
     *
     * @return Status code indicating success or failure.
     */
    StatusCode fillObjRefs( IOpaqueAddress* pAddr, DataObject* pObj ) override;

    /** Converter overrides: Update transient object from persistent data
     *
     * @param    pAddr       [IN]   Pointer to object address.
     * @param    pObj        [IN]   Pointer to data object
     *
     * @return Status code indicating success or failure.
     */
    StatusCode updateObj( IOpaqueAddress* pAddr, DataObject* pObj ) override;

    /** Converter overrides: Update the references of
     * an updated transient object.
     *
     * @param    pAddr       [IN]   Pointer to object address.
     * @param    pObj        [IN]   Pointer to data object
     *
     * @return Status code indicating success or failure.
     */
    StatusCode updateObjRefs( IOpaqueAddress* pAddr, DataObject* pObj ) override;

    /** Converter overrides: Convert the transient object to the
     * requested representation.
     *
     * @param    pObj        [IN]   Pointer to data object
     * @param    refpAddr    [OUT]  Location to store pointer to object address.
     *
     * @return Status code indicating success or failure.
     */
    StatusCode createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) override;

    /** Converter overrides: Fill references of persistent object representation.
     *
     * @param    pObj        [IN]   Pointer to data object
     * @param    refpAddr    [OUT]  Location to store pointer to object address.
     *
     * @return Status code indicating success or failure.
     */
    StatusCode fillRepRefs( IOpaqueAddress* pAddr, DataObject* pObj ) override;

    /** Converter overrides: Update persistent object representation.
     *
     * @param    pObj        [IN]   Pointer to data object
     * @param    refpAddr    [OUT]  Location to store pointer to object address.
     *
     * @return Status code indicating success or failure.
     */
    StatusCode updateRep( IOpaqueAddress* pAddr, DataObject* pObj ) override;

    /** Converter overrides: Update references of persistent object representation.
     *
     * @param    pObj        [IN]   Pointer to data object
     * @param    refpAddr    [OUT]  Location to store pointer to object address.
     *
     * @return Status code indicating success or failure.
     */
    StatusCode updateRepRefs( IOpaqueAddress* pAddr, DataObject* pObj ) override;
  };
} // namespace Gaudi
