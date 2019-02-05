#ifndef GAUDIUTILS_MULTIFILECATALOG_H
#define GAUDIUTILS_MULTIFILECATALOG_H

#include "GaudiKernel/Service.h"
#include "GaudiUtils/IFileCatalog.h"
#include "GaudiUtils/IFileCatalogMgr.h"

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class MultiFileCatalog
   *
   *  This class constitutes the core of the
   *  XML based FileCatalog API for using POOL within Gaudi.
   *  This class manages multiple file catalogs.
   *
   */
  class MultiFileCatalog : public extends<Service, IFileCatalog, IFileCatalogMgr> {
  public:
    // disambiguate between Service::Factory and IFileCatalog::Factory
    typedef Service::Factory Factory;

  protected:
    typedef const std::string&         CSTR;
    typedef std::vector<IFileCatalog*> Catalogs;
    typedef std::vector<std::string>   CatalogNames;

    template <class A1, class F>
    std::string _find( A1& arg1, F pmf ) const {
      std::string result;
      for ( const auto& i : m_catalogs ) {
        result = ( i->*pmf )( arg1 );
        if ( !result.empty() ) break;
      }
      return result;
    }
    template <class A1, class F>
    void _collect( A1& arg1, F f ) const {
      A1 tmp;
      for ( const auto& i : m_catalogs ) {
        f( i, tmp ); // tmp = f(i)
        arg1.insert( arg1.end(), tmp.begin(), tmp.end() );
        tmp.clear();
      }
    }
    template <class A2, class A1, class F>
    void _collect( const A2& arg2, A1& arg1, F f ) const {
      A1 tmp;
      for ( const auto& i : m_catalogs ) {
        f( i, arg2, tmp ); // tmp = f(i,arg2)
        arg1.insert( arg1.end(), tmp.begin(), tmp.end() );
        tmp.clear();
      }
    }

  public:
    /// Create a catalog file, initialization of XercesC.
    using extends::extends;

    /** IService implementation                                               */
    /// Finalize service object
    StatusCode initialize() override;
    /// Finalize service object
    StatusCode finalize() override;

    /** Catalog interface                                                     */
    /// Create file identifier using UUID mechanism
    std::string createFID() const override;
    /// Access to connect string
    CSTR connectInfo() const override;
    /// Parse the DOM tree of the XML catalog
    void init() override;
    /// Save DOM catalog to file
    void commit() override;
    /// Save DOM catalog to file
    void rollback() override;
    /// Check if the catalog is read-only
    bool readOnly() const override;
    /// Check if the catalog should be updated
    bool dirty() const override;
    /// Return the status of physical file name
    bool existsPFN( CSTR pfn ) const override { return !lookupPFN( pfn ).empty(); }
    /// Lookup file identifier by physical file name
    std::string lookupPFN( CSTR pfn ) const override { return _find( pfn, &IFileCatalog::lookupPFN ); }
    /// Return the status of a logical file name
    bool existsLFN( CSTR lfn ) const override { return !lookupLFN( lfn ).empty(); }
    /// Lookup file identifier by logical file name
    std::string lookupLFN( CSTR lfn ) const override { return _find( lfn, &IFileCatalog::lookupLFN ); }
    /// Return the status of a FileID
    bool existsFID( CSTR fid ) const override { return 0 != getCatalog( fid, false, false, false ); }
    /// Dump all physical file names of the catalog and their attributes associate to the FileID
    void getPFN( CSTR fid, Files& files ) const override {
      _collect( fid, files, std::mem_fn( &IFileCatalog::getPFN ) );
    }
    /// Dump all logical file names of the catalog associate to the FileID
    void getLFN( CSTR fid, Files& files ) const override {
      _collect( fid, files, std::mem_fn( &IFileCatalog::getLFN ) );
    }

    /// Dump all file Identifiers
    void getFID( Strings& fids ) const override { _collect( fids, std::mem_fn( &IFileCatalog::getFID ) ); }
    /// Delete FileID from the catalog
    void deleteFID( CSTR fid ) const override { writeCatalog( fid )->deleteFID( fid ); }
    /// Create a FileID and DOM Node of the PFN with all the attributes
    void registerPFN( CSTR fid, CSTR pfn, CSTR ftype ) const override;
    /// Create a FileID and DOM Node of the LFN with all the attributes
    void registerLFN( CSTR fid, CSTR lfn ) const override;
    /// Create a FileID and DOM Node
    void registerFID( CSTR fid ) const override { writeCatalog()->registerFID( fid ); }
    /// rename a PFN
    void renamePFN( CSTR pfn, CSTR newpfn ) const override { writeCatalog()->renamePFN( pfn, newpfn ); }
    /// remove a PFN
    void deletePFN( CSTR pfn ) const override { writeCatalog()->deletePFN( pfn ); }
    /// Dump all MetaData of the catalog for a given file ID
    void getMetaData( CSTR fid, Attributes& attr ) const override {
      _collect( fid, attr, std::mem_fn( &IFileCatalog::getMetaData ) );
    }
    /// Access metadata item
    std::string getMetaDataItem( CSTR fid, CSTR name ) const override;
    /// Insert/update metadata item
    void setMetaData( CSTR fid, CSTR attr, CSTR val ) const override {
      writeCatalog( fid )->setMetaData( fid, attr, val );
    }
    /// Drop all metadata of one FID
    void dropMetaData( CSTR fid ) const override { writeCatalog( fid )->dropMetaData( fid ); }
    /// Drop specified metadata item
    void dropMetaData( CSTR fid, CSTR attr ) const override { writeCatalog( fid )->dropMetaData( fid, attr ); }

    /** Catalog management                                                    */
    /// Find catalog by connect string
    IFileCatalog* findCatalog( CSTR connect, bool must_be_writable ) const override;
    /// Add new catalog identified by name to the existing ones
    void addCatalog( CSTR connect ) override;
    /// Add new catalog identified by reference to the existing ones
    void addCatalog( IFileCatalog* cat ) override;
    /// Remove catalog identified by name from the existing ones
    void removeCatalog( CSTR connect ) override;
    /// Remove catalog identified by reference from the existing ones
    void removeCatalog( const IFileCatalog* cat ) override;
    /// Access catalog container
    Catalogs& catalogs() override { return m_catalogs; }
    /// Access catalog container (CONST)
    const Catalogs& catalogs() const override { return m_catalogs; }
    /// Access to the (first) writable file catalog
    IFileCatalog* writeCatalog( CSTR fid = "" ) const override { return getCatalog( fid, true, true, false ); }
    /// Define the writable catalog identified by reference
    void setWriteCatalog( IFileCatalog* cat ) override;
    /// Define the writable catalog identified by name
    void setWriteCatalog( CSTR connect ) override;

  private:
    /// Find catalog containing a given file identifier
    IFileCatalog* getCatalog( CSTR fid, bool throw_if_not, bool writable = true, bool prt = true ) const;
    /// Find catalog by connect string
    Catalogs::iterator i_findCatalog( CSTR connect, bool must_be_writable );

    /// simple property handle to allow interactive modification of
    /// list of the file catalogs
    void propHandler();

    void        printError( CSTR msg, bool throw_exc = true ) const;
    std::string lookupFID( CSTR lfn ) const;

    /// Container with references to known catalogs
    Catalogs m_catalogs;

    Gaudi::Property<CatalogNames> m_catalogNames{
        this, "Catalogs", {{"xmlcatalog_file:test_catalog.xml"}}, &MultiFileCatalog::propHandler, "catalog names"};

    /// Flag to indicate if catalog is started
    bool m_started = false;
    /// BACKUP:: Container with catalog names
    CatalogNames m_oldNames;
  };
} /* End namespace Gaudi                 */
#endif /* GAUDIUTILS_MULTIFILECATALOG_H */
