// $Id: GenericAddress.h,v 1.7 2003/08/05 13:49:15 mato Exp $
#ifndef GAUDIKERNEL_GENERICADDRESS_H
#define GAUDIKERNEL_GENERICADDRESS_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Forward declarations
class IRegistry;

/** @class GenericAddress GenericAddress.h GaudiKernel/GenericAddress.h

    Generic Transient Address.
    The generic transient address describes the recipe to load/save
    a persistent object from/to its transient representation.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API GenericAddress: public IOpaqueAddress  {
protected:
  /// Reference count
  unsigned long   m_refCount;
  /// Storage type
  long            m_svcType;
  /// Class id
  CLID            m_clID;
  /// String parameters to be accessed
  std::string     m_par[3];
  /// Integer parameters to be accessed
  unsigned long   m_ipar[2];
  /// Pointer to corresponding directory
  IRegistry*      m_pRegistry;

public:
  /// Dummy constructor
  GenericAddress()
    : m_refCount(0),
      m_svcType(0),
      m_clID(0),
      m_pRegistry(0)
  {
    m_ipar[0]=m_ipar[1]=0xFFFFFFFF;
  }
  /// Standard Constructor
  GenericAddress(const GenericAddress& copy)
    : IOpaqueAddress(copy),
      m_refCount(0),
      m_svcType(copy.m_svcType),
      m_clID(copy.m_clID),
      m_pRegistry(copy.m_pRegistry)
  {
    m_par[0]  = copy.m_par[0];
    m_par[1]  = copy.m_par[1];
    m_ipar[0] = copy.m_ipar[0];
    m_ipar[1] = copy.m_ipar[1];
  }
  /// Standard Constructor
  GenericAddress( long svc,
                  const CLID& clid,
                  const std::string& p1="",
                  const std::string& p2="",
                  unsigned long ip1=0,
                  unsigned long ip2=0)
    : m_refCount(0),
      m_svcType(svc),
      m_clID(clid),
      m_pRegistry(0)
  {
    m_par[0]  = p1;
    m_par[1]  = p2;
    m_ipar[0] = ip1;
    m_ipar[1] = ip2;
  }

  /// Standard Destructor
  virtual ~GenericAddress()   {
  }

  /// Add reference to object
  virtual unsigned long addRef   ()   {
    return ++m_refCount;
  }
  /// release reference to object
  virtual unsigned long release  ()   {
    int cnt = --m_refCount;
    if ( 0 == cnt )   {
      delete this;
    }
    return cnt;
  }
  /// Pointer to directory
  virtual IRegistry* registry()   const     {
    return m_pRegistry;
  }
  /// Set pointer to directory
  virtual void setRegistry(IRegistry* pRegistry)   {
    m_pRegistry = pRegistry;
  }
  /// Access : Retrieve class ID of the link
  const CLID& clID()  const   {
    return m_clID;
  }
  /// Access : Set class ID of the link
  void setClID(const CLID& clid)   {
    m_clID = clid;
  }
  /// Access : retrieve the storage type of the class id
  long svcType()  const    {
    return m_svcType;
  }
  /// Access : set the storage type of the class id
  void setSvcType(long typ)    {
    m_svcType = typ;
  }
  /// Retrieve string parameters
  virtual const std::string* par() const   {
    return m_par;
  }
  /// Retrieve integer parameters
  virtual const unsigned long* ipar()  const  {
    return m_ipar;
  }
};
#endif // GAUDIKERNEL_GENERICADDRESS_H

