// $Id: PoolDbNTupleCnv.h,v 1.2 2006/11/30 14:50:44 mato Exp $
//====================================================================
// NTuple converter class definition
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
#ifndef POOLDB_POOLDBNTUPLECNV_H
#define POOLDB_POOLDBNTUPLECNV_H 1

// Include files
#include "GaudiPoolDb/PoolDbStatCnv.h"

// Forward declarations
class PoolDbTupleCallback;
class INTuple;

/** @class PoolDbNTupleCnv PoolDbNTupleCnv.h PoolDb/PoolDbNTupleCnv.h
  *
  * NTuple converter class definition
  *
  * Description:
  * Definition of the generic converter for row wise and column wise
  * Ntuples.
  *
  * @author  M.Frank
  * @version 1.0
  */
class GAUDI_API PoolDbNTupleCnv: public PoolDbStatCnv {
protected:
  /// Callback to bind N-tuple data for reading
  /**
    * @param   pTuple    [IN]  Pointer to N-tuple structure
    * @param   call      [IN]  Pointer to N-tuple callback structure
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode bindRead(          INTuple*              pTuple,
                                PoolDbTupleCallback*  call);

  /// Callback to extract N-tuple data which needed to be put into BLOBs
  /**
    * @param   pTuple    [IN]  Pointer to N-tuple structure
    * @param   call      [IN]  Pointer to N-tuple callback structure
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode readData(          INTuple*              pTuple,
                                PoolDbTupleCallback*  call);

  /// Callback to bind N-tuple data for writing
  /**
    * @param   pTuple    [IN]  Pointer to N-tuple structure
    * @param   call      [IN]  Pointer to N-tuple callback structure
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode bindWrite(         INTuple*              pTuple,
                                PoolDbTupleCallback*  call);

public:
  /// Standard constructor
  /**
    * @param clid        [IN]  Class ID of the object type to be converted.
    * @param db          [IN]  Pointer to POOL database interface
    * @param svc         [IN]  Pointer to Gaudi service locator
    *
    * @return Reference to PoolDbNTupleCnv object
    */
  PoolDbNTupleCnv(long typ, const CLID& clid, ISvcLocator* svc);

  /// Standard destructor
  virtual ~PoolDbNTupleCnv();

  /** Converter overrides: Convert the transient object
    * to the requested representation.
    *
    * @param pObject     [IN]  Valid pointer to DataObject
    * @param refpAddress [OUT] Location to store pointer to the
    *                          object's opaque address
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode createRep(   DataObject*       pObject,
                                  IOpaqueAddress*&  refpAddress );

  /// Resolve the references of the converted object.
  /**
    * @param pAddress   [IN]   Valid pointer to the object's opaque address
    * @param pObject    [IN]   Valid pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode fillRepRefs( IOpaqueAddress*   pAddress,
                                  DataObject*       pObject);

  /** Converter overrides: Update the references of an
    * updated transient object.
    *
    * @param pAddress    [IN]  Valid pointer to the object's opaque address
    * @param refpObject [OUT]  Location to store pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode createObj(   IOpaqueAddress*   pAddress,
                                  DataObject*&      refpObject);

  /** Converter overrides: Update the references of an
    * updated transient object.
    *
    * @param pAddress   [IN]   Valid pointer to the object's opaque address
    * @param pObject    [IN]   Valid pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode fillObjRefs( IOpaqueAddress*  /* pAddress */,
                                  DataObject*      /* pObject  */)
  {  return StatusCode::SUCCESS;                                      }

  /** Update the transient object: NTuples end here when reading records
    *
    * @param pAddress   [IN]   Valid pointer to the object's opaque address
    * @param pObject    [IN]   Valid pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode updateObj(   IOpaqueAddress*   pAddress,
                                  DataObject*       pObject);

  /** Converter overrides: Update the references of an
    * updated transient object.
    *
    * @param pAddress   [IN]   Valid pointer to the object's opaque address
    * @param pObject    [IN]   Valid pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode updateObjRefs( IOpaqueAddress* /* pAddress */,
                                    DataObject*     /* pObject  */ )
  {  return StatusCode::SUCCESS;                                      }

  /** Converter overrides: Update the converted representation
    * of a transient object.
    *
    * @param pAddress   [IN]   Valid pointer to the object's opaque address
    * @param pObject    [IN]   Valid pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode updateRep(     IOpaqueAddress* /* pAddress */,
                                    DataObject*     /* pObject  */)
  {  return StatusCode::SUCCESS;                                      }

  /** Converter overrides: Update the converted representation of a
    * transient object.
    *
    * @param pAddress   [IN]   Valid pointer to the object's opaque address
    * @param pObject    [IN]   Valid pointer to DataObject
    *
    * @return Status code indicating success or failure
    */
  virtual StatusCode updateRepRefs( IOpaqueAddress* /* pAddress */,
                                    DataObject*     /* pObject  */ )
  {  return StatusCode::SUCCESS;                                      }
};

#endif    // POOLDB_POOLDBNTUPLECNV_H
