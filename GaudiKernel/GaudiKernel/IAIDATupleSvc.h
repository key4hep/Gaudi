// $Id: IAIDATupleSvc.h,v 1.2 2005/12/06 18:55:19 hmd Exp $
#ifndef GAUDIKERNEL_IAIDATUPLESVC_H
#define GAUDIKERNEL_IAIDATUPLESVC_H 1

//Include files:
#include "GaudiKernel/IDataProviderSvc.h"

//Forward declarations:
class DataObject;

//Added to allow use of AIDA in applications
//without prefix AIDA:: (as before):
namespace AIDA {
  class ITuple;
  class IAnnotation;
}

using AIDA::ITuple;
using AIDA::IAnnotation;

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
  DeclareInterfaceID(IAIDATupleSvc,3,0);

  // ===================================
  // Book a tuple in the transient store
  // ===================================
  virtual AIDA::ITuple*	book( const std::string& fullPath,
	                      const std::string& title,
	                      const std::string& columns ) = 0;


  // =======================================
  // Register a tuple in the transient store
  // =======================================
  virtual StatusCode registerObject( const std::string& dirPath,
                                     const std::string& objPath,
                                     AIDA::ITuple* tObj ) = 0;


  virtual StatusCode registerObject( DataObject* pParent,
                                     const std::string& objPath,
                                     AIDA::ITuple* tObj ) = 0;


  // =========================================
  // Unregister a tuple in the transient store
  // =========================================
  virtual StatusCode unregisterObject( AIDA::ITuple* tObj ) = 0;

  virtual StatusCode unregisterObject( AIDA::ITuple* tObj,
                                       const std::string& objectPath ) = 0;


  // ====================================
  // Retrieve a tuple from the data store
  // ====================================
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::ITuple*& tObj ) = 0;


  // ==============================
  // Find a tuple in the data store
  // ==============================
  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::ITuple*& tObj ) = 0;

  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::ITuple*& tObj ) = 0;

  /// Set criteria for reading, filtering the input
  virtual void setCriteria(  AIDA::ITuple*& tObj,
                             const std::string& criteria ) = 0;


  // ==============
  // Helper methods
  // ==============

	//Create all directories in a given full path
	virtual DataObject* createPath( const std::string& newPath )= 0;

  //Create a sub-directory in a directory.
	virtual DataObject* createDirectory(const std::string& parentDir,
										                  const std::string& subDir)	= 0;

  virtual StatusCode myTest() = 0;

  //virtual std::string storeName(AIDA::ITuple*& tObj) = 0;
};

#endif //GAUDIKERNEL_IAIDATUPLESVC_H
