//====================================================================
//  Statistics file converter class definition
//
//  Author     : M.Frank
//
//====================================================================
#ifndef GAUDIROOTCNV_ROOTDATABASECNV_H
#define GAUDIROOTCNV_ROOTDATABASECNV_H 1

// Framework include files
#include "RootCnv/RootDirectoryCnv.h"

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi
{

  /** @class RootDatabaseCnv RootDatabaseCnv.h Root/RootDatabaseCnv.h
   *
   * Statistics file converter class definition
   *
   * Description:
   * Definition of the converter to open root database files.
   *
   * @author  M.Frank
   * @version 1.0
   */
  class GAUDI_API RootDatabaseCnv : public RootDirectoryCnv
  {
  public:
    /** Initializing Constructor
     * @param      typ      [IN]     Concrete storage type of the converter
     * @param      svc      [IN]     Pointer to service locator object
     * @param      mgr      [IN]     Pointer to hosting conversion service
     *
     * @return Reference to RootDatabaseCnv object
     */
    RootDatabaseCnv( long typ, const CLID&, ISvcLocator* svc, RootCnvSvc* mgr );

    /** Retrieve the name of the container a given object is placed into
     * @param      pReg     [IN]    Pointer to registry entry.
     *
     * @return     Name of the container the object should be put to.
     */
    const std::string containerName( IRegistry* /* pReg */ ) const override { return "<local>"; }

    /** Converter overrides: Create transient object from persistent data
     *
     * @param    refpAddress [IN]   Pointer to object address.
     * @param    refpObject  [OUT]  Location to pointer to store data object
     *
     * @return Status code indicating success or failure.
     */
    StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& refpObj ) override;
  };
}
#endif // GAUDIROOTCNV_ROOTDATABASECNV_H
