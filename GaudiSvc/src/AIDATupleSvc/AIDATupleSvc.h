// $Id: AIDATupleSvc.h,v 1.4 2006/01/27 13:01:19 hmd Exp $
#ifndef AIDATUPLESVC_AIDATUPLESVC_H
#define AIDATUPLESVC_AIDATUPLESVC_H 1

// Include files
#include <vector>
#include <map>

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IAIDATupleSvc.h"

#include "GaudiPI/AIDA_Proxy/AIDA_Proxy.h"

/** @class AIDATupleSvc AIDATupleSvc.h AIDATupleSvc/AIDATupleSvc.h
 *
 *  AIDATupleSvc class definition
 *
 *  @author Manuel Domenech Saavedra
 *  @date   2005-05-23
 */


// Forward Declarations
template <class T> class SvcFactory;

class AIDATupleSvc : public extends1<DataSvc, IAIDATupleSvc>
{
private:
  /// Allow SvcFactory to instantiate the service.
  friend class SvcFactory<AIDATupleSvc>;

protected:
  /// Typedefs for connections
  typedef std::vector<std::string>                      DBaseEntries;
  typedef std::map<std::string, pi_aida::Proxy_Store*>  Connections;

  /// Output streams
  DBaseEntries m_output;

  /// Input streams
  DBaseEntries m_input;

  /// Container of connection points
  Connections  m_connections;

  /// Standard constructor
  AIDATupleSvc( const std::string& name, ISvcLocator* svc );

  /// Standard destructor
  virtual ~AIDATupleSvc();


  /** Split full path into its components
   *  @param  fullPath  [IN]:  Full path of the store
   *  @param  dirPath   [OUT]: Path in the transient store to the tuple's parent dir
   *  @param  objPath   [OUT]: Name of the tuple in the transient store starting with '/'
   *  @param  storePath [OUT]: Path in the transient store of the tuple store
   *  @param  storeObj  [OUT]: Full name of the tuple inside the tuple store, including
   *                           parent dirs.File is a flat structure.
   */
  void parsePath( const std::string& fullPath,
                  std::string& dirPath,
                  std::string& objPath,
                  std::string& storePath,
                  std::string& storeObj );


  /** Create all directories recursively in a given full path
   *  @param   newPath [IN]:  Path until the tuple in the transient store
   *  returns pointer to DataObject corresponding to the top level directory of the path
   */
  virtual DataObject* createPath( const std::string& newPath );


  /** Create a sub-directory in a given directory
   *  @param  parentDir [IN]:  Path to the parent directory, already existing
   *  @param  subDir    [IN]:  Name of the subdirectory
   *  returns pointer to DataObject corresponding to the new subdirectory
   */
  virtual DataObject* createDirectory( const std::string& parentDir,
                                       const std::string& subDir );

  /** Add file to I/O list
   *  @param  ident  [IN]:  String with all the options from the job options file
   *  @param  mode   [IN]:  Flag indicating INPUT or OUTPUT stream
   *  returns StatusCode indicating the success of the operation
   */
  StatusCode connect( const std::string& ident,
                      int mode );

  /// Disconnect all the stores in the I/O list
  StatusCode closeAIDAStores();

public:
  /// DataSvc overrides: Initialize the service.
  virtual StatusCode initialize();
  /// DataSvc overrides: reinitialize service.
  virtual StatusCode reinitialize();
  /// DataSvc overrides: stop the service.
  virtual StatusCode finalize();

  /** Create AIDATuple in the data store
   *  @param  fullPath  [IN]:  Full path of the tuple
   *  @param  title     [IN]:  Title of the tuple
   *  @param  columns   [IN]:  Description of the column names and types
   *  returns a new tuple registered in the transient store
   */
  virtual AIDA::ITuple* book( const std::string& fullPath,
                              const std::string& title,
                              const std::string& columns );


  /** Create AIDATuple in the data store
   *  @param  parentPath  [IN]:  Full path to the parent dir of the tuple
   *  @param  objPath     [IN]:  Name of the tuple
   *  @param  title       [IN]:  Title of the tuple
   *  @param  columns     [IN]:  Description of the column names and types
   *  returns a new tuple registered in the transient store
   */
  virtual AIDA::ITuple* book( const std::string& parentPath,
                              const std::string& objPath,
                              const std::string& title,
                              const std::string& columns );


  /** Create AIDATuple in the data store
   *  @param  parentPath  [IN]:  Full path to the parent dir of the tuple
   *  @param  tupleID     [IN]:  ID of the tuple
   *  @param  title       [IN]:  Title of the tuple
   *  @param  columns     [IN]:  Description of the column names and types
   *  returns a new tuple registered in the transient store
   */
  virtual AIDA::ITuple* book( const std::string& parentPath,
                              int tupleID,
                              const std::string& title,
                              const std::string& columns );


  /** Create AIDATuple in the data store
   *  @param  pParent  [IN]:  Pointer to the parent directory of the tuple
   *  @param  objPath  [IN]:  Name of the tuple
   *  @param  title    [IN]:  Title of the tuple
   *  @param  columns  [IN]:  Description of the column names and types
   *  returns a new tuple registered in the transient store
   */
  virtual AIDA::ITuple* book( DataObject* pParent,
                              const std::string& objPath,
                              const std::string& title,
                              const std::string& columns );


  /** Create AIDATuple in the data store
   *  @param  pParent  [IN]:  Pointer to the parent directory of the tuple
   *  @param  tupleID  [IN]:  ID of the tuple
   *  @param  title    [IN]:  Title of the tuple
   *  @param  columns  [IN]:  Description of the column names and types
   *  returns a new tuple registered in the transient store
   */
  virtual AIDA::ITuple* book( DataObject* pParent,
                              int tupleID,
                              const std::string& title,
                              const std::string& columns );


  /** Register a tuple in the data store
   *  @param  dirPath  [IN]:  Path to the directory where the tuple must be registered
   *  @param  objPath  [IN]:  Name of the tuple
   *  @param  tObj     [IN]:  Pointer to the tuple to be registered
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode registerObject( const std::string& dirPath,
                                     const std::string& objPath,
                                     AIDA::ITuple* tObj );


  /** Register a tuple in the data store
   *  @param  pParent  [IN]:  Pointer to the parent directory of the tuple
   *  @param  objPath  [IN]:  Name of the tuple
   *  @param  tObj     [IN]:  Pointer to the tuple to be registered
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode registerObject( DataObject* pParent,
                                     const std::string& objPath,
                                     AIDA::ITuple* tObj );


  /** Unregister a tuple in the data store
   *  @param  tObj  [IN]:  Pointer to tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode unregisterObject( AIDA::ITuple* tObj );


  /** Unregister a tuple in the data store
   *  @param  tObj     [IN]:  Pointer to tuple
   *  @param  objPath  [IN]:  Name of the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode unregisterObject( AIDA::ITuple* tObj,
                                       const std::string& objPath );


  /** Retrieve a tuple from the data store
   *  @param  fullPath  [IN]:  Full path of the tuple
   *  @param  tObj      [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::ITuple*& tObj );


  /** Retrieve a tuple from the data store
   *  @param  parentPath  [IN]:  Full path to the parent dir of the tuple
   *  @param  objPath     [IN]:  Name of the tuple
   *  @param  tObj        [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::ITuple*& tObj );


  /** Retrieve a tuple from the data store
   *  @param  pParent  [IN]:  Pointer to the parent directory of the tuple
   *  @param  objPath  [IN]:  Name of the tuple
   *  @param  tObj     [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode retrieveObject( DataObject* pParent,
                                     const std::string& objPath,
                                     AIDA::ITuple*& tObj );


  /** Locate a tuple in the data store
   *  @param  fullPath  [IN]:  Full path of the tuple
   *  @param  tObj      [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::ITuple*& tObj );

  /** Locate a tuple in the data store
   *  @param  pDirectory  [IN]:  Pointer to a directory
   *  @param  path        [IN]:  Relative path to the tuple from the pDirectory
   *  @param  tObj        [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::ITuple*& tObj );

  /** Locate a tuple in the data store
   *  @param  parentPath  [IN]:  Full path of the tuple
   *  @param  objPath     [IN]:  Name of the tuple
   *  @param  tObj        [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::ITuple*& tObj );

  /** Locate a tuple in the data store
   *  @param  parentObj  [IN]:  Pointer to the parent directory
   *  @param  objPath    [IN]:  Name of the tuple
   *  @param  tObj       [OUT]: Pointer to the tuple
   *  returns StatusCode indicating the success of the operation
   */
  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::ITuple*& tObj );

  /** Set a selection criteria for reading, as a filter
   *  @param tObj      [IN]:  Pointer to the tuple
   *  @param criteria  [IN]:  Selection criteria
   */
  virtual void setCriteria( AIDA::ITuple*& tObj,
                            const std::string& criteria );

  /// Dummy method to test the service.
  virtual StatusCode myTest();
};

#endif //AIDATUPLESVC_AIDATUPLESVC_H
