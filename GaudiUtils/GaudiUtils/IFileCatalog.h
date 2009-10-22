#ifndef GAUDIFILECATALOG_IFILECATALOG_H
#define GAUDIFILECATALOG_IFILECATALOG_H
// $ID:$

#include "GaudiKernel/IInterface.h"
#include <string>
#include <vector>
#include <utility>

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class IFileCatalog IFileCatalog.h GaudiUtils/IFileCatalog.h
    *
    *  File catalog interface.
    *  Resolve file ids, physical and logical file names.
    *
    *  @author  M.Frank
    *  @version 1.0
    *  @date    20/10/2007
    */
  class GAUDI_API IFileCatalog : virtual public IInterface {

  public:
    /// InterfaceID
    DeclareInterfaceID(IFileCatalog,2,0);

    /// Public type definitions
    typedef std::pair<std::string,std::string>  NamedItem;
    typedef std::vector<NamedItem>              Attributes;
    typedef std::vector<NamedItem>              Files;
    typedef std::vector<std::string>            Strings;

    /** Catalog interface                                               */
    /// Create file identifier using UUID mechanism
    virtual std::string createFID() const = 0;
    /// Access to connect string
    virtual const std::string& connectInfo() const = 0;
    /// Parse the DOM tree of the XML catalog
    virtual void init() = 0;
    /// Save catalog to file
    virtual void commit() = 0;
    /// Save catalog to file
    virtual void rollback() = 0;
    /// Check if the catalog is read-only
    virtual bool readOnly()  const = 0;
    /// Check if the catalog should be updated
    virtual bool dirty() const = 0;
    /// Dump all PFNames of the catalog and their attributes associate to the FileID
    virtual void getPFN(const std::string& fid, Files& files) const = 0;
    /// Dump all LFNames of the catalog associate to the FileID
    virtual void getLFN(const std::string& fid, Files& files) const = 0;
    /// Dump all file Identifiers
    virtual void getFID(Strings& fids) const = 0;
    /// Return the status of a PFName
    virtual bool existsPFN(const std::string& pfn)  const = 0;
    /// Lookup file identifier by physical file name
    virtual std::string lookupPFN(const std::string& lfn)  const = 0;
    /// Return the status of a LFName
    virtual bool existsLFN(const std::string& lfn) const = 0;
    /// Lookup file identifier by physical file name
    virtual std::string lookupLFN(const std::string& lfn)  const = 0;
    /// Return the status of a FileID
    virtual bool existsFID(const std::string& fid)  const = 0;
    /// Delete FileID Node from the catalog
    virtual void deleteFID(const std::string& FileID) const = 0;
    /// Create a Node for a FileID and DOM Node of the PFN with all the attributes
    virtual void registerPFN(const std::string& fid, const std::string& pfn, const std::string& ftype) const = 0;
    /// Create a Node for a FileID and DOM Node of the LFN with all the attributes
    virtual void registerLFN(const std::string& fid, const std::string& lfn) const = 0;
    /// Create a Node for a FileID and DOM Node
    virtual void registerFID(const std::string& fid) const = 0;
    /// Dump all MetaData of the catalog for a given file ID
    virtual void getMetaData(const std::string& fid, Attributes& attr) const = 0;
    /// Access metadata item
    virtual std::string getMetaDataItem(const std::string& fid, const std::string& name) const = 0;
    /// Insert/update metadata item
    virtual void setMetaData(const std::string& fid, const std::string& name, const std::string& value) const = 0;
    /// Drop all metadata of one FID
    virtual void dropMetaData(const std::string& fid) const = 0;
    /// Drop specified metadata item
    virtual void dropMetaData(const std::string& fid, const std::string& attr) const = 0;
  };
}         /* End namespace Gaudi             */
#endif    /* GAUDIFILECATALOG_IFILECATALOG_H */
