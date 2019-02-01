// dear emacs, this is -*- C++ -*-
#ifndef GAUDIKERNEL_ICLASSIDSVC_H
#define GAUDIKERNEL_ICLASSIDSVC_H

//<<<<<< INCLUDES                                                       >>>>>>
#include <string>

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/StatusCode.h"

//<<<<<< FORWARD DECLARATIONS                                           >>>>>>
namespace Athena {
  class PackageInfo;
}
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class IClassIDSvc
 * @brief  interface to the CLID database
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

class IClassIDSvc : virtual public IService {
public:
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
  /// get type name associated with clID (if any)
  virtual StatusCode getPackageInfoForID( const CLID& id, Athena::PackageInfo& info ) const = 0;
  /// associate type name, package info and type-info name with clID
  virtual StatusCode setTypePackageForID( const CLID&, const std::string& typeName, const Athena::PackageInfo&,
                                          const std::string& typeInfoName = "" ) = 0;
  /// Gaudi boilerplate
  static const InterfaceID& interfaceID();
};

//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>
inline const InterfaceID& IClassIDSvc::interfaceID() {
  static const InterfaceID _IID( "IClassIDSvc", 1, 0 );
  return _IID;
}

#endif // GAUDIKERNEL_ICLASSIDSVC_H
