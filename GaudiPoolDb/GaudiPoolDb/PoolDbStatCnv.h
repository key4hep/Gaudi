// $Id: PoolDbStatCnv.h,v 1.2 2005/09/15 14:48:33 hmd Exp $
//------------------------------------------------------------------------------
// Definition of class :  PoolDbStatCnv
//
//  Author     : M.Frank
//
//------------------------------------------------------------------------------
#ifndef POOLDB_POOLDBSTATCNV_H
#define POOLDB_POOLDBSTATCNV_H 1

// Include files
#include "GaudiPoolDb/PoolDbBaseCnv.h"

/** @class PoolDbStatCnv PoolDbStatCnv.h PoolDb/PoolDbStatCnv.h
  *
  * Description:
  * NTuple directory converter class definition
  * Definition of the converter to manage the
  * directories in an database representing N-Tuples.
  *
  * @author  M.Frank
  * @version 1.0
  */
class GAUDI_API PoolDbStatCnv: public PoolDbBaseCnv {
protected:

  /** Retrieve the name of the container a given object is placed into
    * @param      pReg     [IN]    Pointer to registry entry.
    *
    * @return     Name of the container the object should be put to.
    */
  virtual const std::string containerName(IRegistry* pDir)  const;

  /** Retrieve the name of the file a given object is placed into
    * @param      pReg     [IN]    Pointer to registry entry.
    *
    * @return     Name of the file the object should be put to.
    */
  virtual const std::string fileName(IRegistry* pReg) const;

  /** Retrieve the full path of the file a given object is placed into
    * @param      pReg     [IN]    Pointer to registry entry.
    *
    * @return     Name of the file the object should be put to.
    */
  virtual const std::string topLevel(IRegistry* pReg) const;

  /** Save statistics object description.
    * @param      path     [IN]    Path of file to save the description on.
    * @param      ident    [IN]    Identifier of description to be saved.
    * @param      desc     [IN]    Description
    * @param      opt      [IN]    Optional description
    * @param      guid     [IN]    Object giud of described object
    * @param      openMode [IN]    Mode of opening the file to save description
    *
    * @return    Status code indicating success or failure.
    */
  StatusCode saveDescription( const std::string&  path,
                              const std::string&  ident,
                              const std::string&  desc,
                              const std::string&  opt,
                              const pool::Guid&   guid,
                              const CLID&         clid,
                              const std::string&  openMode);

public:

  /** Initializing Constructor
    * @param      typ      [IN]     Concrete storage type of the converter
    * @param      clid     [IN]     Class identifier of the object
    * @param      svc      [IN]     Pointer to service locator object
    *
    * @return Reference to PoolDbBaseCnv object
    */
  PoolDbStatCnv(long typ, const CLID& clid, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~PoolDbStatCnv()   {  }
};
#endif // POOLDB_PoolDbStatCnv_H
