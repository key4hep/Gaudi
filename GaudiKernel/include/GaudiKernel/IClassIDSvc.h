/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_ICLASSIDSVC_H
#define GAUDIKERNEL_ICLASSIDSVC_H

#include <string>

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/StatusCode.h"

/** @class IClassIDSvc
 *  @brief  interface to the CLID database
 *  @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

class IClassIDSvc : virtual public IService {
public:
  /// InterfaceID
  DeclareInterfaceID( IClassIDSvc, 1, 0 );

  /// get next available CLID
  /// @throws std::runtime_error if no CLID can be allocated
  virtual CLID nextAvailableID() const = 0;
  /// check if id is used
  virtual bool isIDInUse( const CLID& id ) const = 0;
  /// check if name is used
  virtual bool isNameInUse( const std::string& name ) const = 0;
  /// get user assigned type name associated with clID
  virtual StatusCode getTypeNameOfID( const CLID& id, std::string& typeName ) const = 0;
  /// get user assigned type-info name associated with clID
  virtual StatusCode getTypeInfoNameOfID( const CLID& id, std::string& typeInfoName ) const = 0;
  /// get id associated with type name (if any)
  virtual StatusCode getIDOfTypeName( const std::string& typeName, CLID& id ) const = 0;
  /// get id associated with type-info name (if any)
  virtual StatusCode getIDOfTypeInfoName( const std::string& typeInfoName, CLID& id ) const = 0;
  /// associate type name and type-info name with clID
  virtual StatusCode setTypeForID( const CLID&, const std::string& typeName, const std::string& typeInfoName = "" ) = 0;
};

#endif // GAUDIKERNEL_ICLASSIDSVC_H
