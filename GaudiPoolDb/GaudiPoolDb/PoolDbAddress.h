// $Id: PoolDbAddress.h,v 1.4 2007/12/10 19:33:36 marcocle Exp $
//====================================================================
//	PoolDbAddress.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbAddress.h,v 1.4 2007/12/10 19:33:36 marcocle Exp $
#ifndef POOLDB_POOLDBADDRESS_H
#define POOLDB_POOLDBADDRESS_H

// Framework include files
#include "POOLCore/Token.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Forward declarations
namespace pool {
  class DataCallBack;
  class Token;
}

/** @class PoolDbAddress PoolDbAddress.h PoolDb/PoolDbAddress.h
  *
  * Description:
  *
  * Definition of a transient link which is capable of locating
  * an object in the persistent storage.
  *
  * @author  M.Frank
  * @version 1.0
  */
class GAUDI_API PoolDbAddress: virtual public IOpaqueAddress {
protected:
  /// Reference count
  unsigned long         m_refCount;
  /// Pointer to corresponding directory
  IRegistry*            m_pRegistry;
  /// String parameters to be accessed
  mutable std::string   m_par[3];
  /// Integer (unsignod long) parameters to be accessed
  mutable unsigned long m_ipar[2];
  /// Pointer to call handler
  pool::DataCallBack*   m_handler;
  /// Pointer to POOL token structure
  pool::Token*          m_token;

public:
  /// Full constructor: all arguments MUST be valid, no checks!
  PoolDbAddress(pool::Token* aToken);
  /// Standard Destructor
  virtual ~PoolDbAddress();

  /**@name Implementation of IOpaqueAddress interface              */
  //@{
  /// Add reference to object
  virtual unsigned long addRef()           { return ++m_refCount;            }
  /// Release reference to object
  virtual unsigned long release();
  /// Retrieve class ID of the address
  const CLID& clID()  const                { return *(CLID*)&m_token->classID().Data1;}
  /// Access : retrieve the storage type of the class id
  long svcType()  const                    { return m_token->technology();   }
  /// Retrieve string parameters
  virtual const std::string* par() const;
  /// Retrieve integer parameters
  virtual const unsigned long* ipar() const;
  /// Pointer to directory
  virtual IRegistry* registry() const      { return m_pRegistry;             }
  //@}

  /// Access to POOL data handler (if present)
  pool::DataCallBack* handler() const       { return m_handler;     }
  /// Set the POOL data handler; releases existing data handler
  void setHandler(pool::DataCallBack* h);
  /// Cache handle to container object
  pool::Token* token()                      { return m_token;       }
  /// Check link for validity
  bool isValid()    const;
  /// Set pointer to directory
  virtual void setRegistry(IRegistry* pReg) { m_pRegistry = pReg;   }
};
#endif // POOLDB_PoolDbAddress_H
