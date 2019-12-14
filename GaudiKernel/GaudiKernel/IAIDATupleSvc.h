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
#ifndef GAUDIKERNEL_IAIDATUPLESVC_H
#define GAUDIKERNEL_IAIDATUPLESVC_H 1

// Include files:
#include "GaudiKernel/IDataProviderSvc.h"

// Forward declarations:
class DataObject;

// Added to allow use of AIDA in applications
// without prefix AIDA:: (as before):
namespace AIDA {
  class ITuple;
  class IAnnotation;
} // namespace AIDA

using AIDA::IAnnotation;
using AIDA::ITuple;

/** @class IAIDATupleSvc IAIDATupleSvc.h GaudiKernel/IAIDATupleSvc.h
 *
 *  Definition of the IAIDATupleSvc interface class
 *
 *  @author Manuel Domenech Saavedra
 *  @date   2005-05-23
 */

class GAUDI_API IAIDATupleSvc : virtual public IDataProviderSvc {

public:
  /// InterfaceID
  DeclareInterfaceID( IAIDATupleSvc, 3, 0 );

  // ===================================
  // Book a tuple in the transient store
  // ===================================
  virtual AIDA::ITuple* book( const std::string& fullPath, const std::string& title, const std::string& columns ) = 0;

  // =======================================
  // Register a tuple in the transient store
  // =======================================
  using IDataProviderSvc::registerObject;

  virtual StatusCode registerObject( const std::string& dirPath, const std::string& objPath, AIDA::ITuple* tObj ) = 0;

  virtual StatusCode registerObject( DataObject* pParent, const std::string& objPath, AIDA::ITuple* tObj ) = 0;

  // =========================================
  // Unregister a tuple in the transient store
  // =========================================
  using IDataProviderSvc::unregisterObject;

  virtual StatusCode unregisterObject( AIDA::ITuple* tObj ) = 0;

  virtual StatusCode unregisterObject( AIDA::ITuple* tObj, const std::string& objectPath ) = 0;

  // ====================================
  // Retrieve a tuple from the data store
  // ====================================
  using IDataProviderSvc::retrieveObject;

  virtual StatusCode retrieveObject( const std::string& fullPath, AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode retrieveObject( const std::string& parentPath, const std::string& objPath,
                                     AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode retrieveObject( DataObject* parentObj, const std::string& objPath, AIDA::ITuple*& tObj ) = 0;

  // ==============================
  // Find a tuple in the data store
  // ==============================
  using IDataProviderSvc::findObject;

  virtual StatusCode findObject( const std::string& fullPath, AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode findObject( IRegistry* pDirectory, const std::string& path, AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode findObject( const std::string& parentPath, const std::string& objPath, AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode findObject( DataObject* parentObj, const std::string& objPath, AIDA::ITuple*& tObj ) = 0;

  /// Set criteria for reading, filtering the input
  virtual void setCriteria( AIDA::ITuple*& tObj, const std::string& criteria ) = 0;

  // ==============
  // Helper methods
  // ==============

  // Create all directories in a given full path
  virtual DataObject* createPath( const std::string& newPath ) = 0;

  // Create a sub-directory in a directory.
  virtual DataObject* createDirectory( const std::string& parentDir, const std::string& subDir ) = 0;

  virtual StatusCode myTest() = 0;
};

#endif // GAUDIKERNEL_IAIDATUPLESVC_H
