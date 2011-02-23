#ifndef GAUDIFILECATALOG_IFILECATALOGMGR_H
#define GAUDIFILECATALOG_IFILECATALOGMGR_H
// $ID:$

#include "GaudiKernel/IInterface.h"
#include <string>
#include <vector>

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi {

  // Forward declarations
  class IFileCatalog;

  /** @class IFileCatalogMgr IFileCatalogMgr.h GaudiUtils/IFileCatalogMgr.h
    *
    *  File catalog manager.
    *  Interface to add/remove file catalogs from the multi catalog.
    *
    *  @author  M.Frank
    *  @version 1.0
    *  @date    20/10/2007
    */
  class GAUDI_API IFileCatalogMgr : virtual public IInterface {
  public:
    /// InterfaceID
    DeclareInterfaceID(IFileCatalogMgr,2,0);

    /// Public type definitions
    typedef std::vector<IFileCatalog*> Catalogs;

    /** Catalog management                                                    */
    /// Find catalog by connect string
    virtual IFileCatalog* findCatalog(const std::string& connect, bool must_be_writable) const = 0;
    /// Add new catalog identified by name to the existing ones
    virtual void addCatalog(const std::string& connect) = 0;
    /// Add new catalog identified by reference to the existing ones
    virtual void addCatalog(IFileCatalog* cat) = 0;
    /// Remove catalog identified by name from the existing ones
    virtual void removeCatalog(const std::string& connect) = 0;
    /// Remove catalog identified by reference from the existing ones
    virtual void removeCatalog(const IFileCatalog* cat) = 0;
    /// Access catalog container
    virtual Catalogs& catalogs() = 0;
    /// Access catalog container (CONST)
    virtual const Catalogs& catalogs()  const = 0;
    /// Access to the (first) writable file catalog
    virtual IFileCatalog* writeCatalog(const std::string& fid="") const = 0;
    /// Define the writable catalog identified by reference
    virtual void setWriteCatalog(IFileCatalog* cat) = 0;
    /// Define the writable catalog identified by name
    virtual void setWriteCatalog(const std::string& connect) = 0;
  };
}         /* End namespace Gaudi             */
#endif    /* GAUDIFILECATALOG_IFILECATALOGMGR_H */
