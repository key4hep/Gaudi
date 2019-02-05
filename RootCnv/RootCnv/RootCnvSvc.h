//====================================================================
//	RootCnvSvc definition
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
#ifndef GAUDIROOTCNV_GAUDIROOTCNVSVC_H
#define GAUDIROOTCNV_GAUDIROOTCNVSVC_H

// Framework include files
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiUtils/IIODataManager.h"

// C++ include files
#include <set>

// Forward declarations
struct IDataManagerSvc;
class IIncidentSvc;
class TClass;

// ROOT include files
#include "TBranch.h"
#include "TClass.h"
#include "TROOT.h"
#include "TTree.h"

//#include "RootCnv/RootPerfMonSvc.h"

/*
 * Gaudi namespace declaration
 */
namespace Gaudi {

  // Forward declarations
  class IDataConnection;
  class RootDataConnection;
  class RootConnectionSetup;

  /** @class RootCnvSvc RootCnvSvc.h src/RootCnvSvc.h
   *
   * Description:
   *
   * RootCnvSvc class implementation definition.
   *
   *  @author  Markus Frank
   *  @version 1.0
   *  @date    20/12/2009
   */
  class GAUDI_API RootCnvSvc : public ConversionSvc {
  protected:
    Gaudi::Property<std::string> m_ioPerfStats{this, "IOPerfStats", "",
                                               "Enable TTree IOperfStats if not empty; otherwise perf stat file name"};
    Gaudi::Property<std::string> m_shareFiles{this, "ShareFiles", "NO",
                                              "Share files? If set to YES,  files will not be closed on finalize"};
    Gaudi::Property<bool> m_incidentEnabled{this, "EnableIncident", true, "Flag to enable incidents on FILE_OPEN"};
    Gaudi::Property<std::string> m_recordName{this, "RecordsName", "/FileRecords",
                                              "Records name to fire incident for file records"};

    // ROOT Write parameters
    Gaudi::Property<int> m_autoFlush{this, "AutoFlush", 100,
                                     "AutoFlush parameter for ROOT TTree (Number of events between auto flushes)"};
    Gaudi::Property<int> m_basketSize{this, "BasketSize", 2 * 1024 * 1024 /*MBYTE*/,
                                      "Basket optimization parameter for ROOT TTree (total basket size)"};
    Gaudi::Property<int> m_bufferSize{this, "BufferSize", 2 * 1024 /*kBYTE*/,
                                      "Buffer size optimization parameter for ROOT TTree"};
    Gaudi::Property<int> m_splitLevel{this, "SplitLevel", 0, "Split level optimization parameter for ROOT TTree"};
    Gaudi::Property<std::string> m_compression{this, "GlobalCompression", "",
                                               "Compression-algorithm:compression-level,  empty: do nothing"};

    /// Reference to the I/O data manager
    SmartIF<Gaudi::IIODataManager> m_ioMgr;
    /// Reference to incident service
    SmartIF<IIncidentSvc> m_incidentSvc;
    /// On writing: reference to active output stream
    Gaudi::RootDataConnection* m_current = nullptr;
    /// TClass pointer to reference class
    TClass* m_classRefs = nullptr;
    /// TClass pointer to DataObject class
    TClass* m_classDO = nullptr;
    /// Setup structure (ref-counted) and passed to data connections
    std::shared_ptr<RootConnectionSetup> m_setup;
    /// Property: ROOT section name
    std::string m_currSection;

    /// Set with bad files/tables
    std::set<std::string> m_badFiles;

    /// Message streamer
    std::unique_ptr<MsgStream> m_log;

    /// Helper: Get TClass for a given DataObject pointer
    TClass* getClass( DataObject* pObject );
    /// Helper: Use message streamer
    MsgStream& log() const { return *m_log; }

  public:
    /// Standard constructor
    RootCnvSvc( const std::string& name, ISvcLocator* svc );

    /// Update state of the service
    StatusCode updateServiceState( IOpaqueAddress* /* pAddress */ ) override { return StatusCode::SUCCESS; }

    /** Standard way to print errors. after the printout an exception is thrown.
     * @param      msg      [IN]     Message string to be printed.
     *
     * @return     Status code returning failure.
     */
    StatusCode error( const std::string& msg );

    /** Connect the output file to the service with open mode.
     *  @param      dataset     String containig file name
     *  @param      mode        String containig opening mode
     *
     *  @return     Status code indicating success or failure.
     */
    StatusCode connectDatabase( const std::string& dataset, int mode, RootDataConnection** con );

    /// ConversionSvc overload: initialize Db service
    StatusCode initialize() override;

    /// ConversionSvc overload: Finalize Db service
    StatusCode finalize() override;

    /// ConversionSvc overload: Create new Converter using factory
    IConverter* createConverter( long typ, const CLID& wanted, const ICnvFactory* fac ) override;

    /// ConversionSvc overload: Load the class (dictionary) for the converter
    void loadConverter( DataObject* pObj ) override;

    /** Connect the output file to the service with open mode.
     *  @param      outputFile  String containig output file
     *  @param      openMode    String containig opening mode of the output file
     *
     *  @return     Status code indicating success or failure.
     */
    StatusCode connectOutput( const std::string& outputFile, const std::string& openMode ) override;

    /** Connect the output file to the service.
     *  @param      outputFile  String containig output file
     *
     *  @return     Status code indicating success or failure.
     */
    StatusCode connectOutput( const std::string& outputFile ) override;

    /** Commit pending output.
     *  @param      outputFile  String containig output file
     *  @param      do_commit   if true commit the output and flush
     *                          eventually pending items to the database
     *                          if false, discard pending buffers.
     *                          Note: The possibility to commit or rollback
     *                          depends on the database technology used!
     *
     *  @return     Status code indicating success or failure.
     */
    StatusCode commitOutput( const std::string& outputFile, bool do_commit ) override;

    /** Disconnect from an existing data stream.
     * @param      dbName      String containing name of the database
     *
     * @return     Status code indicating success or failure.
     */
    virtual StatusCode disconnect( const std::string& dbName );

    /** IAddressCreator implementation: Address creation.
     * Create an address using the link infotmation together with
     * the triple (database name/container name/object name).
     *
     * @param refLink           Reference to abstract link information
     * @param dbName            Database name
     * @param containerName     Object container name
     * @param refpAddress       Opaque address information to retrieve object
     *
     * @return                  StatusCode indicating SUCCESS or failure
     */
    StatusCode createAddress( long svc_type, const CLID& clid, const std::string* par, const unsigned long* ip,
                              IOpaqueAddress*& refpAddress ) override;

    /** IAddressCreator implementation: Creates an address in string form to object form
     *  @param      svc_type    Technology identifier encapsulated
     *                          in this address.
     *  @param      clid        Class identifier of the DataObject
     *                          represented by the opaque address
     *  @param      address     Input address.
     *  @param      refpAddress Output address in string form.
     *  @return     Status code indicating success or failure.
     */
    StatusCode createAddress( long svc_type, const CLID& clid, const std::string& refAddress,
                              IOpaqueAddress*& refpAddress ) override {
      return this->ConversionSvc::createAddress( svc_type, clid, refAddress, refpAddress );
    }

    /** Insert null marker for not existent transient object
     *
     * @param    path     [IN]   Path to the (null-)object
     *
     * @return Status code indicating success or failure.
     */
    virtual StatusCode createNullRep( const std::string& path );

    /** Insert null marker for not existent transient object
     *
     * @param    path     [IN]   Path to the (null-)object
     *
     * @return Status code indicating success or failure.
     */
    virtual StatusCode createNullRef( const std::string& path );

    /** Convert the transient object to the requested persistent representation.
     *
     * @param    pObj     [IN]   Pointer to data object
     * @param    refpAddr [OUT]  Location to store pointer to object address.
     *
     * @return Status code indicating success or failure.
     */
    virtual StatusCode i__createRep( DataObject* pObj, IOpaqueAddress*& refpAddr );

    /** Resolve the references of the converted object.
     *
     * @param    pAddr    [IN]   Pointer to object address.
     * @param    pObj     [IN]   Pointer to data object
     *
     * @return Status code indicating success or failure.
     */
    virtual StatusCode i__fillRepRefs( IOpaqueAddress* pAddr, DataObject* pObj );

    /** Create transient object from persistent data
     *
     * @param    pAddr       [IN]   Pointer to object address.
     * @param    refpObj     [OUT]  Location to pointer to store data object
     *
     * @return Status code indicating success or failure.
     */
    virtual StatusCode i__createObj( IOpaqueAddress* pAddr, DataObject*& refpObj );

    /** Resolve the references of the created transient object.
     *
     * @param    pAddr    [IN]   Pointer to object address.
     * @param    pObj     [IN]   Pointer to data object
     *
     * @return Status code indicating success or failure.
     */
    virtual StatusCode i__fillObjRefs( IOpaqueAddress* pAddr, DataObject* pObj );
  };
} // namespace Gaudi

#endif // GAUDIROOTCNV_GAUDIROOTCNVSVC_H
