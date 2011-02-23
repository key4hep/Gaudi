#ifndef GAUDIUTILS_IODATAMANAGER_H
#define GAUDIUTILS_IODATAMANAGER_H

// C++ include files
#include <map>
// Framework include files
#include "GaudiKernel/Service.h"
#include "GaudiUtils/IIODataManager.h"

class IIncidentSvc;

/*
 *  LHCb namespace declaration
 */
namespace Gaudi  {

  // Forward declarations
  class IFileCatalog;
  class RawDataConnectionEntry;

  /** @class IIODataManager
    *
    *  @author  M.Frank
    *  @version 1.0
    *  @date    20/10/2007
    *  @author  R. Lambert
    *  @date    03/09/2009
    */
  class IODataManager : public extends1<Service, IIODataManager> {
  protected:
    typedef const std::string& CSTR;
    struct Entry  {
      std::string      type;
      IoType           ioType;
      IDataConnection* connection;
      bool             keepOpen;
      Entry(CSTR tech,bool k, IoType iot,IDataConnection* con)
      : type(tech), ioType(iot), connection(con), keepOpen(k) {
      }
    };
    typedef std::map<std::string,Entry*>       ConnectionMap;
    typedef std::map<std::string, std::string> FidMap;

    /// Property: Name of the file catalog service
    std::string          m_catalogSvcName;
    /// Property: Age limit
    int                  m_ageLimit;
    /// Property: Flag for auto gfal data access
    bool                 m_useGFAL;
    /// Property: Flag if unaccessible files should be quarantines in job
    bool                 m_quarantine;

    /// Map with I/O descriptors
    ConnectionMap        m_connectionMap;
    /// Reference to file catalog
    SmartIF<IFileCatalog> m_catalog;
    /// Map of FID to PFN
    FidMap               m_fidMap;
    StatusCode connectDataIO(int typ, IoType rw, CSTR fn, CSTR technology, bool keep,Connection* con);
    StatusCode reconnect(Entry* e);
    StatusCode error(CSTR msg, bool rethrow);
    StatusCode establishConnection(Connection* con);
    
    SmartIF<IIncidentSvc> m_incSvc; ///the incident service

  public:

    /** Initializing constructor
      *  @param[in]   nam   Name of the service
      *  @param[in]   loc   Pointer to the service locator object
      *  @return Initialized reference to service object
      */
    IODataManager(CSTR nam, ISvcLocator* loc);

    /// Standard destructor
    virtual ~IODataManager()  {}

    /// IService implementation: initialize the service
    virtual StatusCode initialize();

    /// IService implementation: finalize the service
    virtual StatusCode finalize();

    /// Open data stream in read mode
    virtual StatusCode connectRead(bool keep_open, Connection* ioDesc);
    /// Open data stream in write mode
    virtual StatusCode connectWrite(Connection* con,IoType mode=Connection::CREATE,CSTR doctype="UNKNOWN");
    /// Release data stream
    virtual StatusCode disconnect(Connection* ioDesc);
    /// Retrieve known connection
    virtual Connection* connection(const std::string& dsn) const;
    /// Get connection by owner instance (0=ALL)
    virtual Connections connections(const IInterface* owner) const;
    /// Read raw byte buffer from input stream
    virtual StatusCode read(Connection* ioDesc, void* const data, size_t len);
    /// Write raw byte buffer to output stream
    virtual StatusCode write(Connection* con, const void* data, int len);
    /// Seek on the file described by ioDesc. Arguments as in ::seek()
    virtual long long int seek(Connection* ioDesc, long long int where, int origin);
  };
}         // End namespace Gaudi
#endif    // GAUDIUTILS_IODATAMANAGER_H
