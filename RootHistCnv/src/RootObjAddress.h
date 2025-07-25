/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef ROOTHISTCNV_ROOTOBJADDRESS_H
#define ROOTHISTCNV_ROOTOBJADDRESS_H 1

#include <GaudiKernel/IOpaqueAddress.h>
#include <string_view>

class TObject;

namespace RootHistCnv {

  class RootObjAddress : public IOpaqueAddress {

  protected:
    unsigned long m_refCount = 0;
    /// Storage type
    long m_svcType = 0;
    /// Class id
    CLID m_clID = 0;
    /// String parameters to be accessed
    std::string m_par[2];
    /// Integer parameters to be accessed
    unsigned long m_ipar[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
    /// Pointer to corresponding directory
    IRegistry* m_pRegistry = nullptr;
    /// Pointer to TObject
    TObject* m_tObj = nullptr;

  public:
    /// Dummy constructor
    RootObjAddress() = default;

    RootObjAddress( const RootObjAddress& copy )
        : IOpaqueAddress( copy )
        , m_refCount( 0 )
        , m_svcType( copy.m_svcType )
        , m_clID( copy.m_clID )
        , m_pRegistry( copy.m_pRegistry )
        , m_tObj( copy.m_tObj ) {
      m_par[0]  = copy.m_par[0];
      m_par[1]  = copy.m_par[1];
      m_ipar[0] = copy.m_ipar[0];
      m_ipar[1] = copy.m_ipar[1];
    }

    /// Standard Constructor
    RootObjAddress( long svc, const CLID& clid, std::string_view p1 = {}, std::string_view p2 = {},
                    unsigned long ip1 = 0, unsigned long ip2 = 0, TObject* tObj = nullptr )
        : m_svcType( svc ), m_clID( clid ), m_tObj( tObj ) {
      m_par[0]  = std::string{ p1 };
      m_par[1]  = std::string{ p2 };
      m_ipar[0] = ip1;
      m_ipar[1] = ip2;
    }

    /// Add reference to object
    unsigned long addRef() override { return ++m_refCount; }

    unsigned long release() override {
      unsigned long cnt = --m_refCount;
      if ( 0 == cnt ) delete this;
      return cnt;
    }
    /// Pointer to directory
    IRegistry* registry() const override { return m_pRegistry; }
    /// Set pointer to directory
    void setRegistry( IRegistry* pRegistry ) override { m_pRegistry = pRegistry; }
    /// Access : Retrieve class ID of the link
    const CLID& clID() const override { return m_clID; }
    /// Access : Set class ID of the link
    virtual void setClID( const CLID& clid ) { m_clID = clid; }
    /// Access : retrieve the storage type of the class id
    long svcType() const override { return m_svcType; }
    /// Access : set the storage type of the class id
    virtual void setSvcType( long typ ) { m_svcType = typ; }
    /// Retrieve string parameters
    const std::string* par() const override { return m_par; }
    /// Retrieve integer parameters
    const unsigned long* ipar() const override { return m_ipar; }

    /// Retrieve TObject* ptr
    virtual TObject* tObj() const { return m_tObj; }
  };
} // namespace RootHistCnv

#endif
