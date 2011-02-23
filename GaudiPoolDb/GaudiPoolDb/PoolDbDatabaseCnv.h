// $Id: PoolDbDatabaseCnv.h,v 1.2 2006/11/30 14:50:44 mato Exp $
//====================================================================
//  Statistics file converter class definition
//
//  Author     : M.Frank
//
//====================================================================
#ifndef POOLDB_POOLDBDATABASECNV_H
#define POOLDB_POOLDBDATABASECNV_H 1

// Include files
#include "GaudiPoolDb/PoolDbDirectoryCnv.h"

/** @class PoolDbDatabaseCnv PoolDbDatabaseCnv.h PoolDb/PoolDbDatabaseCnv.h
  *
  * Statistics file converter class definition
  *
  * Description:
  * Definition of the converter to open Database file.
  * Wether this is a real file or only a connection to
  * a database depends on the underlying engine.
  *
  * @author  M.Frank
  * @version 1.0
  */
class GAUDI_API PoolDbDatabaseCnv: public PoolDbDirectoryCnv {
public:

  /** Initializing Constructor
    * @param      typ      [IN]     Concrete storage type of the converter
    * @param      svc      [IN]     Pointer to service locator object
    *
    * @return Reference to PoolDbDatabaseCnv object
    */
  PoolDbDatabaseCnv(long typ, const CLID&, ISvcLocator* svc );

  /// Standard destructor
  virtual ~PoolDbDatabaseCnv();

  /** Retrieve the name of the container a given object is placed into
    * @param      pReg     [IN]    Pointer to registry entry.
    *
    * @return     Name of the container the object should be put to.
    */
  virtual const std::string containerName(IRegistry* /* pReg */)  const  {
    return "<local>";
  }

  /** Converter overrides: Create transient object from persistent data
    *
    * @param    refpAddress [IN]   Pointer to object address.
    * @param    refpObject  [OUT]  Location to pointer to store data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& refpObj);
};
#endif    // POOLDB_POOLDBDATABASECNV_H
