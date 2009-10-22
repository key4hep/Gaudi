#ifndef ROOTHISTCNV_ROOTOBJADDRESS_H
#define ROOTHISTCNV_ROOTOBJADDRESS_H 1

#include "GaudiKernel/IOpaqueAddress.h"

class TObject;

namespace RootHistCnv {

  class RootObjAddress: public IOpaqueAddress {


  protected:

    unsigned long   m_refCount;
    /// Storage type
    long            m_svcType;
    /// Class id
    CLID            m_clID;
    /// String parameters to be accessed
    std::string     m_par[2];
    /// Integer parameters to be accessed
    unsigned long   m_ipar[2];
    /// Pointer to corresponding directory
    IRegistry*      m_pRegistry;
    /// Pointer to TObject
    TObject*        m_tObj;

  public:
    
  /// Dummy constructor
    RootObjAddress() 
      : m_refCount(0), 
      m_svcType(0),
      m_clID(0),
      m_pRegistry(0),
      m_tObj(0)
    {
      m_ipar[0]=m_ipar[1]=0xFFFFFFFF;
    }

    RootObjAddress(const RootObjAddress& copy)  
      : IOpaqueAddress(copy),
	m_refCount(0),
	m_svcType(copy.m_svcType),
	m_clID(copy.m_clID),
	m_pRegistry(copy.m_pRegistry),
	m_tObj(copy.m_tObj)
    {
      m_par[0]  = copy.m_par[0];
      m_par[1]  = copy.m_par[1];
      m_ipar[0] = copy.m_ipar[0];
      m_ipar[1] = copy.m_ipar[1];
    }

    /// Standard Constructor
    RootObjAddress( long svc,
		    const CLID& clid,
		    const std::string& p1="", 
		    const std::string& p2="",
		    unsigned long ip1=0,
		    unsigned long ip2=0,
		    TObject* tObj=0 )
      : m_refCount(0),
      m_svcType(svc),
      m_clID(clid),
      m_pRegistry(0),
      m_tObj(tObj)
    {
      m_par[0]  = p1;
      m_par[1]  = p2;
      m_ipar[0] = ip1;
      m_ipar[1] = ip2;
    }
    
    /// Standard Destructor
    virtual ~RootObjAddress()   {
    }

    /// Add reference to object
    virtual unsigned long addRef   ()   {
      return ++m_refCount;
    }

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
    virtual const CLID& clID()  const   {
      return m_clID;
    }
    /// Access : Set class ID of the link
    virtual void setClID(const CLID& clid)   {
      m_clID = clid;
    }
    /// Access : retrieve the storage type of the class id
    virtual long svcType()  const    {
      return m_svcType;
    }
    /// Access : set the storage type of the class id
    virtual void setSvcType(long typ)    {
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

    /// Retrieve TObject* ptr
    virtual TObject* tObj() const {
      return m_tObj;
    }

  };
};

#endif
