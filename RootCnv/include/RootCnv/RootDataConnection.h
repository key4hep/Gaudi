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
#ifndef GAUDIROOT_ROOTDATACONNECTION_H
#define GAUDIROOT_ROOTDATACONNECTION_H

// Framework include files
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiUtils/IIODataManager.h"
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "RootCnv/RootRefs.h"
#include "TFile.h"
#include "TTreePerfStats.h"

// Forward declarations
class TTree;
class TClass;
class TBranch;

class MsgStream;
class IRegistry;
class DataObject;
class IIncidentSvc;

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class RootConnectionSet RootDataConnection.h GaudiRootCnv/RootDataConnection.h
   *
   *  Class describing the setup parameters of a ROOT data connection.
   *  The parameters are filled by the conversion service (using properties)
   *  and is then passed to all data connections served by this service.
   *
   *  @author  M.Frank
   *  @version 1.0
   *  @date    20/12/2009
   */
  class GAUDI_API RootConnectionSetup final {
  private:
    /// Reference to message service
    std::unique_ptr<MsgStream> m_msgSvc;
    /// Reference to incident service
    SmartIF<IIncidentSvc> m_incidentSvc = nullptr;

  public:
    /// Type definition for string maps
    typedef std::vector<std::string> StringVec;

    /// Vector of strings with branches to be cached for input files
    StringVec cacheBranches;
    /// Vector of strings with branches to NOT be cached for input files
    StringVec vetoBranches;
    /// RootCnvSvc Property: Root data cache size
    std::string loadSection;
    /// RootCnvSvc Property: Root data cache size
    int cacheSize{ 0 };
    /// RootCnvSvc Property: ROOT cache learn entries
    int learnEntries{ 0 };

    /// Standard constructor
    RootConnectionSetup() = default;

    /// Set the global compression level
    static StatusCode setCompression( std::string_view compression );
    /// Access to global compression level
    static int compression();

    /// Set message service reference
    void setMessageSvc( MsgStream* m );
    /// Retrieve message service
    MsgStream& msgSvc() const { return *m_msgSvc; }

    /// Set incident service reference
    void setIncidentSvc( IIncidentSvc* m );
    /// Retrieve incident service
    IIncidentSvc* incidentSvc() const { return m_incidentSvc.get(); }
  };

  /** @class RootDataConnection RootDataConnection.h GaudiRootCnv/RootDataConnection.h
   *
   *  Concrete implementation of the IDataConnection interface to access ROOT files.
   *
   *  @author  M.Frank
   *  @version 1.0
   *  @date    20/12/2009
   */
  class GAUDI_API RootDataConnection : virtual public Gaudi::IDataConnection {
  public:
    enum class Status : StatusCode::code_t { ROOT_READ_ERROR = 0x2, ROOT_OPEN_ERROR = 0x4 };

    /** @class ContainerSection RootDataConnection.h GaudiRootCnv/RootDataConnection.h
     *
     *  Internal helper class, which described a TBranch section in a ROOT file.
     *  TBranch sections (ie. an intervall of events) are used to describe
     *  files using the ROOT fast merge mechanism.
     *
     *  @author  M.Frank
     *  @version 1.0
     *  @date    20/12/2009
     */
    struct ContainerSection {
      /// Default constructor
      ContainerSection() : start( -1 ), length( 0 ) {}
      /// Initializing constructor
      ContainerSection( int s, int l ) : start( s ), length( l ) {}
      /// Copy constructor
      ContainerSection( const ContainerSection& s ) : start( s.start ), length( s.length ) {}
      /// Assignment operator to copy objects
      ContainerSection& operator=( const ContainerSection& s ) {
        if ( this != &s ) {
          start  = s.start;
          length = s.length;
        }
        return *this;
      }
      /// The start entry of the section
      int start;
      /// The length of the section
      int length;
    };

    /// Type definition for string maps
    typedef std::vector<std::string> StringVec;
    /// Type definition for the parameter map
    typedef std::vector<std::pair<std::string, std::string>> ParamMap;
    /// Definition of tree sections
    typedef std::map<std::string, TTree*, std::less<>> Sections;
    /// Definition of container sections to handle merged files
    typedef std::vector<ContainerSection> ContainerSections;
    /// Definition of database section to handle merged files
    typedef std::map<std::string, ContainerSections, std::less<>> MergeSections;
    /// Link sections definition
    typedef std::vector<RootRef> LinkSections;
    /// Client set
    typedef std::set<const IInterface*> Clients;

    /// Allow access to printer service
    MsgStream& msgSvc() const { return m_setup->msgSvc(); }

  protected:
    /// Reference to the setup structure
    std::shared_ptr<RootConnectionSetup> m_setup;
    /// I/O read statistics from TTree
    std::unique_ptr<TTreePerfStats> m_statistics;
    /// Reference to ROOT file
    std::unique_ptr<TFile> m_file;
    /// Pointer to the reference tree
    TTree* m_refs = nullptr;
    /// Tree sections in TFile
    Sections m_sections;
    /// Map containing external database file names (fids)
    StringVec m_dbs;
    /// Map containing external container names
    StringVec m_conts;
    /// Map containing internal links names
    StringVec m_links;
    /// Map containing merge FIDs
    StringVec m_mergeFIDs;
    /// Parameter map for file parameters
    ParamMap m_params;
    /// Database section map for merged files
    MergeSections m_mergeSects;
    /// Database link sections
    LinkSections m_linkSects;
    /// Client list
    Clients m_clients;
    /// Buffer for empty string reference
    std::string m_empty;

    /// Empty string reference
    const std::string& empty() const;

    /// Internal helper to save/update reference tables
    StatusCode saveRefs();

  public:
    /** @class Tool RootDataConnection.h src/RootDataConnection.h
     *
     * Helper class to facilitate an abstraction layer for reading
     * POOL style files with this package.
     * YES: This class obsoletes POOL.
     *
     * @author  M.Frank
     * @version 1.0
     */
    class Tool {
    protected:
      typedef RootDataConnection::StringVec         StringVec;
      typedef RootDataConnection::ParamMap          ParamMap;
      typedef RootDataConnection::Sections          Sections;
      typedef RootDataConnection::MergeSections     MergeSections;
      typedef RootDataConnection::LinkSections      LinkSections;
      typedef RootDataConnection::ContainerSection  ContainerSection;
      typedef RootDataConnection::ContainerSections ContainerSections;

      /// Pointer to containing data connection object
      RootDataConnection* c;

    public:
      TTree*             refs() const { return c->m_refs; }
      StringVec&         dbs() const { return c->m_dbs; }
      StringVec&         conts() const { return c->m_conts; }
      StringVec&         links() const { return c->m_links; }
      ParamMap&          params() const { return c->m_params; }
      MsgStream&         msgSvc() const { return c->msgSvc(); }
      const std::string& name() const { return c->m_name; }
      Sections&          sections() const { return c->m_sections; }
      LinkSections&      linkSections() const { return c->m_linkSects; }
      MergeSections&     mergeSections() const { return c->m_mergeSects; }

      /// Default destructor
      virtual ~Tool() = default;
      /// Access data branch by name: Get existing branch in read only mode
      virtual TBranch* getBranch( std::string_view section, std::string_view n ) = 0;
      /// Internal overload to facilitate the access to POOL files
      virtual RootRef poolRef( size_t /* which */ ) const { return RootRef(); }

      /// Read references section when opening data file
      virtual StatusCode readRefs() = 0;
      /// Save references section when closing data file
      virtual StatusCode saveRefs() = 0;
      /// Load references object
      virtual int loadRefs( std::string_view section, std::string_view cnt, unsigned long entry,
                            RootObjectRefs& refs ) = 0;
    };
    std::unique_ptr<Tool> m_tool;
    friend class Tool;

    /// Create file access tool to encapsulate POOL compatibiliy
    Tool* makeTool();

  public:
    /// Standard constructor
    RootDataConnection( const IInterface* own, std::string_view nam, std::shared_ptr<RootConnectionSetup> setup );

    /// Direct access to TFile structure
    TFile* file() const { return m_file.get(); }
    /// Check if connected to data source
    bool isConnected() const override { return bool( m_file ); }
    /// Is the file writable?
    bool isWritable() const { return m_file && m_file->IsWritable(); }
    /// Access tool
    Tool* tool() const { return m_tool.get(); }
    /// Access merged data section inventory
    const MergeSections& mergeSections() const { return m_mergeSects; }
    /// Access merged FIDs
    const StringVec& mergeFIDs() const { return m_mergeFIDs; }

    /// Add new client to this data source
    void addClient( const IInterface* client );
    /// Remove client from this data source
    size_t removeClient( const IInterface* client );
    /// Lookup client for this data source
    bool lookupClient( const IInterface* client ) const;

    /// Error handler when bad write statements occur
    void badWriteError( std::string_view msg ) const;

    /// Access link section for single container and entry
    std::pair<const RootRef*, const ContainerSection*> getMergeSection( std::string_view container, int entry ) const;

    /// Enable TTreePerStats
    void enableStatistics( std::string_view section );
    /// Save TTree access statistics if required
    void saveStatistics( std::string_view statisticsFile );

    /// Load object
    int loadObj( std::string_view section, std::string_view cnt, unsigned long entry, DataObject*& pObj );

    /// Load references object
    int loadRefs( std::string_view section, std::string_view cnt, unsigned long entry, RootObjectRefs& refs );

    /// Save object of a given class to section and container
    std::pair<int, unsigned long> saveObj( std::string_view section, std::string_view cnt, TClass* cl, DataObject* pObj,
                                           int buff_siz, int split_lvl, bool fill_missing = false );
    /// Save object of a given class to section and container
    std::pair<int, unsigned long> save( std::string_view section, std::string_view cnt, TClass* cl, void* pObj,
                                        int buff_siz, int split_lvl, bool fill_missing = false );

    /// Open data stream in read mode
    StatusCode connectRead() override;
    /// Open data stream in write mode
    StatusCode connectWrite( IoType typ ) override;
    /// Release data stream and release implementation dependent resources
    StatusCode disconnect() override;
    /// Read root byte buffer from input stream
    StatusCode read( void* const, size_t ) override { return StatusCode::FAILURE; }
    /// Write root byte buffer to output stream
    StatusCode write( const void*, int ) override { return StatusCode::FAILURE; }
    /// Seek on the file described by ioDesc. Arguments as in ::seek()
    long long int seek( long long int, int ) override { return -1; }

    /// Access TTree section from section name. The section is created if required.
    TTree* getSection( std::string_view sect, bool create = false );

    /// Access data branch by name: Get existing branch in read only mode
    TBranch* getBranch( std::string_view section, std::string_view branch_name ) {
      return m_tool->getBranch( section, branch_name );
    }
    /// Access data branch by name: Get existing branch in write mode
    TBranch* getBranch( std::string_view section, std::string_view branch_name, TClass* cl, void* ptr, int buff_siz,
                        int split_lvl );

    /// Create reference object from registry entry
    void makeRef( const IRegistry& pA, RootRef& ref );
    /// Create reference object from values
    void makeRef( std::string_view name, long clid, int tech, std::string_view db, std::string_view cnt, int entry,
                  RootRef& ref );

    /// Convert path string to path index
    int makeLink( std::string_view p );

    /// Access database/file name from saved index
    const std::string& getDb( int which ) const;

    /// Access container name from saved index
    const std::string& getCont( int which ) const {
      return ( which >= 0 ) && ( size_t( which ) < m_conts.size() ) ? *( m_conts.begin() + which ) : empty();
    }

    /// Access link name from saved index
    const std::string& getLink( int which ) const {
      return ( which >= 0 ) && ( size_t( which ) < m_links.size() ) ? *( m_links.begin() + which ) : empty();
    }
  };
} // End namespace Gaudi

STATUSCODE_ENUM_DECL( Gaudi::RootDataConnection::Status )

#endif // GAUDIROOT_ROOTDATACONNECTION_H
