#ifndef GAUDIUTILS_IIODATAMANAGER_H
#define GAUDIUTILS_IIODATAMANAGER_H
// $Id:

// Framework include files
#include "GaudiKernel/IInterface.h"

// C++ include files
#include <string>
#include <vector>

/*
 *  LHCb namespace declaration
 */
namespace Gaudi  {

  /** @class IDataConnection
    *
    *  ABC describing basic data connection
    *
    *  @author  M.Frank
    *  @version 1.0
    *  @date    20/10/2007
    */
  class GAUDI_API IDataConnection  {
  protected:
    /// Connection name/identifier
    std::string              m_name;
    /// File ID of the connection
    std::string              m_fid;
    /// Physical file name of the connection
    std::string              m_pfn;
    /// Age counter
    int                      m_age;
    /// Owner pointer
    const IInterface*        m_owner;
  public:
    /// I/O Connection types
    enum IoType   { READ=1<<1,UPDATE=1<<2,CREATE=1<<3,RECREATE=(1<<4)+(1<<3) };
    /// Status Code on bad file connection
    enum IoStatus { BAD_DATA_CONNECTION=4 };
  public:
    /// Standard constructor
    IDataConnection(const IInterface* own, const std::string& nam)
    : m_name(nam), m_owner(own) {}
    /// Standard destructor
    virtual ~IDataConnection() {}
    /// Connection name
    const std::string& name() const       {     return m_name;       }
    /// Set file ID
    void setFID(const std::string& fid)   {     m_fid = fid;         }
    /// Access file id
    const std::string& fid() const        {     return m_fid;        }
    /// Access physical file name
    const std::string& pfn() const        {     return m_pfn;        }
    /// Set physical file name
    void setPFN(const std::string& fn)    {     m_pfn = fn;          }
    /// Increase age of I/O source
    void ageFile()                        {     ++m_age;             }
    /// Reset age
    void resetAge()                       {     m_age = 0;           }
    /// Access age counter
    int age() const                       {     return m_age;        }
    /// Owner instance
    const IInterface* owner() const       {     return m_owner;      }
    /// Open data stream in read mode
    virtual StatusCode connectRead() = 0;
    /// Open data stream in write mode
    virtual StatusCode connectWrite(IoType type) = 0;
    /// Release data stream
    virtual StatusCode disconnect() = 0;
    /// Check if connected to data source
    virtual bool isConnected() const = 0;
    /// Read raw byte buffer from input stream
    virtual StatusCode read(void* const data, size_t len) = 0;
    /// Write raw byte buffer to output stream
    virtual StatusCode write(const void* data, int len) = 0;
    /// Seek on the file described by ioDesc. Arguments as in ::seek()
    virtual long long int seek(long long int where, int origin) = 0;
  };

  /** @class IIODataManager
    *
    *  @author  M.Frank
    *  @version 1.0
    *  @date    20/10/2007
    */
  class GAUDI_API IIODataManager : virtual public IInterface {
  public:
    /// InterfaceID
    DeclareInterfaceID(IIODataManager,2,0);

    /// Connection type definition
    typedef IDataConnection           Connection;
    typedef std::vector<Connection*>  Connections;
    typedef Connection::IoType        IoType;
    enum FileType { UNKNOWN=1, PFN, LFN, FID };

    /// Open data stream in read mode
    virtual StatusCode connectRead(bool keep_open, IDataConnection* con) = 0;
    /// Open data stream in write mode
    virtual StatusCode connectWrite(IDataConnection* con,IoType mode=Connection::CREATE,const std::string& doctype="UNKNOWN") = 0;
    /// Release data stream
    virtual StatusCode disconnect(IDataConnection* con) = 0;
    /// Retrieve known connection
    virtual Connection* connection(const std::string& dsn) const = 0;
    /// Get connection by owner instance (0=ALL)
    virtual Connections connections(const IInterface* owner) const = 0;
    /// Read raw byte buffer from input stream
    virtual StatusCode read(IDataConnection* con, void* const data, size_t len) = 0;
    /// Write raw byte buffer to output stream
    virtual StatusCode write(IDataConnection* con, const void* data, int len) = 0;
    /// Seek on the file described by ioDesc. Arguments as in ::seek()
    virtual long long int seek(IDataConnection* con, long long int where, int origin) = 0;
  };
}         // End namespace Gaudi
#endif    // GAUDIUTILS_IIODATAMANAGER_H
