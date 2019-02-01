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
namespace Gaudi {

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
  class IODataManager : public extends<Service, IIODataManager> {
  protected:
    typedef const std::string& CSTR;
    struct Entry               final {
      std::string      type;
      IoType           ioType;
      IDataConnection* connection;
      bool             keepOpen;
      Entry( CSTR tech, bool k, IoType iot, IDataConnection* con )
          : type( tech ), ioType( iot ), connection( con ), keepOpen( k ) {}
    };
    typedef std::map<std::string, Entry*>      ConnectionMap;
    typedef std::map<std::string, std::string> FidMap;

    Gaudi::Property<std::string> m_catalogSvcName{this, "CatalogType", "Gaudi::MultiFileCatalog/FileCatalog",
                                                  "name of the file catalog service"};
    Gaudi::Property<bool>        m_useGFAL{this, "UseGFAL", true, "flag for auto gfal data access"};
    Gaudi::Property<bool>        m_quarantine{this, "QuarantineFiles", true,
                                       "if unaccessible files should be quarantines in job"};
    Gaudi::Property<int>         m_ageLimit{this, "AgeLimit", 2, "age limit"};
    Gaudi::Property<bool>        m_disablePFNWarning{
        this, "DisablePFNWarning", false,
        "if set to True,  we will not report when a file is opened by its physical name"};

    /// Map with I/O descriptors
    ConnectionMap m_connectionMap;
    /// Reference to file catalog
    SmartIF<IFileCatalog> m_catalog;
    /// Map of FID to PFN
    FidMap     m_fidMap;
    StatusCode connectDataIO( int typ, IoType rw, CSTR fn, CSTR technology, bool keep, Connection* con );
    StatusCode reconnect( Entry* e );
    StatusCode error( CSTR msg, bool rethrow );
    StatusCode establishConnection( Connection* con );

    SmartIF<IIncidentSvc> m_incSvc; /// the incident service

  public:
    /// Inherited constructor
    using extends::extends;

    /// IService implementation: initialize the service
    StatusCode initialize() override;

    /// IService implementation: finalize the service
    StatusCode finalize() override;

    /// Open data stream in read mode
    StatusCode connectRead( bool keep_open, Connection* ioDesc ) override;
    /// Open data stream in write mode
    StatusCode connectWrite( Connection* con, IoType mode = Connection::CREATE, CSTR doctype = "UNKNOWN" ) override;
    /// Release data stream
    StatusCode disconnect( Connection* ioDesc ) override;
    /// Retrieve known connection
    Connection* connection( const std::string& dsn ) const override;
    /// Get connection by owner instance (0=ALL)
    Connections connections( const IInterface* owner ) const override;
    /// Read raw byte buffer from input stream
    StatusCode read( Connection* ioDesc, void* const data, size_t len ) override;
    /// Write raw byte buffer to output stream
    StatusCode write( Connection* con, const void* data, int len ) override;
    /// Seek on the file described by ioDesc. Arguments as in ::seek()
    long long int seek( Connection* ioDesc, long long int where, int origin ) override;
  };
} // End namespace Gaudi
#endif // GAUDIUTILS_IODATAMANAGER_H
