/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_GENERICADDRESS_H
#define GAUDIKERNEL_GENERICADDRESS_H

// Framework include files
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/Kernel.h"

// Forward declarations
class IRegistry;

/** @class GenericAddress GenericAddress.h GaudiKernel/GenericAddress.h

    Generic Transient Address.
    The generic transient address describes the recipe to load/save
    a persistent object from/to its transient representation.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API GenericAddress : public IOpaqueAddress {
protected:
  /// Reference count
  unsigned long m_refCount = 0;
  /// Storage type
  long m_svcType = 0;
  /// Class id
  CLID m_clID = 0;
  /// String parameters to be accessed
  std::string m_par[3];
  /// Integer parameters to be accessed
  unsigned long m_ipar[2] = {0xFFFFFFFF, 0xFFFFFFFF};
  /// Pointer to corresponding directory
  IRegistry* m_pRegistry = nullptr;

public:
  /// Dummy constructor
  GenericAddress() = default;
  /// Standard Copy Constructor (note: m_refCount is NOT copied)
  GenericAddress( const GenericAddress& copy )
      : IOpaqueAddress( copy ), m_svcType( copy.m_svcType ), m_clID( copy.m_clID ), m_pRegistry( copy.m_pRegistry ) {
    m_par[0]  = copy.m_par[0];
    m_par[1]  = copy.m_par[1];
    m_ipar[0] = copy.m_ipar[0];
    m_ipar[1] = copy.m_ipar[1];
  }
  /// Standard Constructor
  GenericAddress( long svc, const CLID& clid, std::string p1 = "", std::string p2 = "", unsigned long ip1 = 0,
                  unsigned long ip2 = 0 )
      : m_svcType( svc ), m_clID( clid ) {
    m_par[0]  = std::move( p1 );
    m_par[1]  = std::move( p2 );
    m_ipar[0] = ip1;
    m_ipar[1] = ip2;
  }

  /// Add reference to object
  unsigned long addRef() override { return ++m_refCount; }
  /// release reference to object
  unsigned long release() override {
    int cnt = --m_refCount;
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
  void setClID( const CLID& clid ) { m_clID = clid; }
  /// Access : retrieve the storage type of the class id
  long svcType() const override { return m_svcType; }
  /// Access : set the storage type of the class id
  void setSvcType( long typ ) { m_svcType = typ; }
  /// Retrieve string parameters
  const std::string* par() const override { return m_par; }
  /// Retrieve integer parameters
  const unsigned long* ipar() const override { return m_ipar; }
};
#endif // GAUDIKERNEL_GENERICADDRESS_H
