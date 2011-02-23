#ifndef GAUDIUTILS_XMLFILECATALOG_H
#define GAUDIUTILS_XMLFILECATALOG_H

#include "GaudiUtils/IFileCatalog.h"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"

// Forward declarations
class IMessageSvc;

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class XMLFileCatalog
    *
    *  This class constitutes the core of the
    *  XML based FileCatalog API for POOL. It uses the DOM model and
    *  the external XercesC library for parsing.
    *
    */
  class XMLFileCatalog : public implements1<IFileCatalog> {
  protected:
    typedef const std::string& CSTR;

  public:
    /// Create a catalog file, initialization of XercesC.
    XMLFileCatalog(CSTR url, IMessageSvc* m);
    /// Destructor,
    virtual ~XMLFileCatalog();

    /** Catalog interface                                               */
    /// Create file identifier using UUID mechanism
    virtual std::string createFID() const;
    /// Access to connect string
    virtual CSTR connectInfo() const               { return m_file;                 }
    /// Parse the DOM tree of the XML catalog
    virtual void init();
    /// Save DOM catalog to file
    virtual void commit();
    /// Save DOM catalog to file
    virtual void rollback()                       { if ( dirty() )  init();        }
    /// Check if the catalog is read-only
    virtual bool readOnly() const                 { return m_rdOnly;                }
    /// Check if the catalog should be updated
    virtual bool dirty() const                    { return m_update;                }
    /// Return the status of a physical file name
    virtual bool existsPFN(CSTR pfn)  const       { return element(pfn,false) != 0; }
    /// Lookup file identifier by physical file name
    virtual std::string lookupPFN(CSTR fid) const { return lookupFID(fid);          }
    /// Return the status of a logical file name
    virtual bool existsLFN(CSTR lfn)  const       { return element(lfn,false) != 0; }
    /// Lookup file identifier by logical file name
    virtual std::string lookupLFN(CSTR lfn) const { return lookupFID(lfn);          }
    /// Return the status of a FileID
    virtual bool existsFID(CSTR fid)  const       { return element(fid,false) != 0; }
    /// Dump all physical file names of the catalog and their attributes associate to the FileID
    virtual void getPFN(CSTR fid, Files& files) const;
    /// Dump all logical file names of the catalog associate to the FileID
    virtual void getLFN(CSTR fid, Files& files) const;
    /// Dump all file Identifiers
    virtual void getFID(Strings& fids)  const;
    /// Delete FileID Node from the catalog
    virtual void deleteFID(CSTR FileID)  const;
    /// Create a FileID and Node of the physical file name with all the attributes
    virtual void registerPFN(CSTR fid, CSTR pfn, CSTR ftype) const;
    /// Create a FileID and Node of the logical file name with all the attributes
    virtual void registerLFN(CSTR fid, CSTR lfn) const;
    /// Create a FileID and Node
    virtual void registerFID(CSTR fid) const;
    /// Dump all MetaData of the catalog for a given file ID
    virtual void getMetaData(CSTR fid, Attributes& attr) const;
    /// Access metadata item
    virtual std::string getMetaDataItem(CSTR fid, CSTR name) const;
    /// Insert/update metadata item
    virtual void setMetaData(CSTR fid, CSTR name, CSTR value) const;
    /// Drop all metadata of one FID
    virtual void dropMetaData(CSTR fid) const  {   dropMetaData(fid,"*");   }
    /// Drop specified metadata item
    virtual void dropMetaData(CSTR fid, CSTR attr) const;
  private:
    xercesc::DOMDocument* getDoc(bool throw_if_no_exists=true)  const;
    std::string getfile(bool create);
    void printError(CSTR msg, bool throw_exc=true) const;
    std::string lookupFID(CSTR lfn)  const;
    xercesc::DOMNode* element(CSTR fid, bool print_err=true)  const;
    xercesc::DOMNode* child(xercesc::DOMNode* par, CSTR tag, CSTR attr="", CSTR val="") const;
    std::pair<xercesc::DOMElement*, xercesc::DOMElement*> i_registerFID(CSTR fid) const;
    bool                      m_rdOnly;
    mutable bool              m_update;
    xercesc::DOMDocument     *m_doc;
    xercesc::XercesDOMParser *m_parser;
    xercesc::ErrorHandler    *m_errHdlr;
    std::string               m_file;
    IMessageSvc*              m_msgSvc;
  };
  /// Create file identifier using UUID mechanism
  std::string createGuidAsString();
}         /* End namespace Gaudi                 */
#endif    /* GAUDIUTILS_XMLFILECATALOG_H   */
