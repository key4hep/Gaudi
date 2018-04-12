#ifndef GAUDIUTILS_XMLFILECATALOG_H
#define GAUDIUTILS_XMLFILECATALOG_H

#include "GaudiUtils/IFileCatalog.h"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/sax/ErrorHandler.hpp"

#include <memory>

// Forward declarations
class IMessageSvc;

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi
{

  /** @class XMLFileCatalog
    *
    *  This class constitutes the core of the
    *  XML based FileCatalog API for POOL. It uses the DOM model and
    *  the external XercesC library for parsing.
    *
    */
  class XMLFileCatalog : public implements<IFileCatalog>
  {
  protected:
    typedef const std::string& CSTR;

  public:
    /// Create a catalog file, initialization of XercesC.
    XMLFileCatalog( CSTR url, IMessageSvc* m );

    /** Catalog interface                                               */
    /// Create file identifier using UUID mechanism
    std::string createFID() const override;
    /// Access to connect string
    CSTR connectInfo() const override { return m_file; }
    /// Parse the DOM tree of the XML catalog
    void init() override;
    /// Save DOM catalog to file
    void commit() override;
    /// Save DOM catalog to file
    void rollback() override
    {
      if ( dirty() ) init();
    }
    /// Check if the catalog is read-only
    bool readOnly() const override { return m_rdOnly; }
    /// Check if the catalog should be updated
    bool dirty() const override { return m_update; }
    /// Return the status of a physical file name
    bool existsPFN( CSTR pfn ) const override { return element( pfn, false ) != 0; }
    /// Lookup file identifier by physical file name
    std::string lookupPFN( CSTR fid ) const override { return lookupFID( fid ); }
    /// Return the status of a logical file name
    bool existsLFN( CSTR lfn ) const override { return element( lfn, false ) != 0; }
    /// Lookup file identifier by logical file name
    std::string lookupLFN( CSTR lfn ) const override { return lookupFID( lfn ); }
    /// Return the status of a FileID
    bool existsFID( CSTR fid ) const override { return element( fid, false ) != 0; }
    /// Dump all physical file names of the catalog and their attributes associate to the FileID
    void getPFN( CSTR fid, Files& files ) const override;
    /// Dump all logical file names of the catalog associate to the FileID
    void getLFN( CSTR fid, Files& files ) const override;
    /// Dump all file Identifiers
    void getFID( Strings& fids ) const override;
    /// Delete FileID Node from the catalog
    void deleteFID( CSTR FileID ) const override;
    /// Create a FileID and Node of the physical file name with all the attributes
    void registerPFN( CSTR fid, CSTR pfn, CSTR ftype ) const override;
    /// Create a FileID and Node of the logical file name with all the attributes
    void registerLFN( CSTR fid, CSTR lfn ) const override;
    /// Create a FileID and Node
    void registerFID( CSTR fid ) const override;
    /// rename a PFN
    void renamePFN( CSTR pfn, CSTR new_pfn ) const override;
    /// remove a PFN
    void deletePFN( CSTR pfn ) const override;
    /// Dump all MetaData of the catalog for a given file ID
    void getMetaData( CSTR fid, Attributes& attr ) const override;
    /// Access metadata item
    std::string getMetaDataItem( CSTR fid, CSTR name ) const override;
    /// Insert/update metadata item
    void setMetaData( CSTR fid, CSTR name, CSTR value ) const override;
    /// Drop all metadata of one FID
    void dropMetaData( CSTR fid ) const override { dropMetaData( fid, "*" ); }
    /// Drop specified metadata item
    void dropMetaData( CSTR fid, CSTR attr ) const override;

  private:
    xercesc::DOMDocument* getDoc( bool throw_if_no_exists = true ) const;
    std::string getfile( bool create );
    void printError( CSTR msg, bool throw_exc = true ) const;
    std::string lookupFID( CSTR lfn ) const;
    xercesc::DOMNode* element( CSTR fid, bool print_err = true ) const;
    xercesc::DOMNode* child( xercesc::DOMNode* par, CSTR tag, CSTR attr = "", CSTR val = "" ) const;
    std::pair<xercesc::DOMElement*, xercesc::DOMElement*> i_registerFID( CSTR fid ) const;
    bool                                      m_rdOnly = false;
    mutable bool                              m_update = false;
    xercesc::DOMDocument*                     m_doc    = nullptr;
    std::unique_ptr<xercesc::XercesDOMParser> m_parser;
    std::unique_ptr<xercesc::ErrorHandler>    m_errHdlr;
    std::string                               m_file;
    IMessageSvc*                              m_msgSvc;
  };
  /// Create file identifier using UUID mechanism
  std::string createGuidAsString();
} /* End namespace Gaudi                 */
#endif /* GAUDIUTILS_XMLFILECATALOG_H   */
