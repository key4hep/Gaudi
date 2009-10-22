// $Id: PoolDbKeyedContainerCnv.h,v 1.1 2006/11/22 18:10:44 hmd Exp $
//====================================================================
//	PoolDbKeyedContainerCnv definition
//
//	Author     : M.Frank
//====================================================================
#ifndef GAUDIPOOLDB_POOLDBKEYEDCONTAINERCNV_H
#define GAUDIPOOLDB_POOLDBKEYEDCONTAINERCNV_H

// Framework include files
#include "GaudiPoolDb/PoolDbBaseCnv.h"

/** @class PoolDbKeyedContainerCnv PoolDbKeyedContainerCnv.h src/PoolDbKeyedContainerCnv .h
 *
 *  Description:
 *
 *  Definition of the generic Db data converter for container objects
 *  like ObjectVector<TYPE> or ObjectList<TYPE>.
 *  The only thing this converter relies on is an object factory,
 *  which is capable of creating objects of the requested type.
 *
 *  For a detailed description of the overridden function see the the
 *  base classes.
 *
 *  Author:  M.Frank
 *  Version: 1.0
 */
class GAUDI_API PoolDbKeyedContainerCnv: public PoolDbBaseCnv {
public:
  /// Standard Constructor
  PoolDbKeyedContainerCnv(long typ, const CLID& clid, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~PoolDbKeyedContainerCnv();

  /// Update the references of an updated transient object.
  virtual StatusCode updateObjRefs(IOpaqueAddress* pAddr, DataObject* pObj);
};
#endif // GAUDIPOOLDB_POOLDBKEYEDCONTAINERCNV_H
