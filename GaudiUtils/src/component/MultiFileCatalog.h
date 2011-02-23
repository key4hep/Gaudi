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
  class MultiFileCatalog: public extends2<Service, IFileCatalog, IFileCatalogMgr>
  {
  protected:
    typedef const std::string&         CSTR;
    typedef std::vector<IFileCatalog*> Catalogs;
    typedef std::vector<std::string>   CatalogNames;

    template <class T> void _exec(T pmf)  const  {
      for(Catalogs::const_iterator i=m_catalogs.begin(); i != m_catalogs.end(); ++i)
        ((*i)->*pmf)();
    }
    template <class A1,class F> std::string _find(A1& arg1,F pmf)  const {
      std::string result;
      for(Catalogs::const_iterator i=m_catalogs.begin(); i != m_catalogs.end(); ++i)
        if ( !(result= ((*i)->*pmf)(arg1)).empty() ) break;
      return result;
    }
    template <class A1,class F> void _collect(A1 arg1,F pmf)  const {
      A1 tmp;
      for(Catalogs::const_iterator i=m_catalogs.begin();i!=m_catalogs.end();++i,tmp.clear()) {
        ((*i)->*pmf)(tmp);
        arg1.insert(arg1.end(),tmp.begin(),tmp.end());
      }
    }
    template <class A1,class A2,class F> void _collect(A1 arg1,A2& arg2,F pmf)  const {
      A2 tmp;
      for(Catalogs::const_iterator i=m_catalogs.begin();i!=m_catalogs.end();++i,tmp.clear()) {
        ((*i)->*pmf)(arg1,tmp);
        arg2.insert(arg2.end(),tmp.begin(),tmp.end());
      }
    }
  public:
    /// Create a catalog file, initialization of XercesC.
    MultiFileCatalog(const std::string& nam, ISvcLocator* svc);
    /// Destructor,
    virtual ~MultiFileCatalog();

    /** IService implementation                                               */
    /// Finalize service object
    virtual StatusCode initialize();
    /// Finalize service object
    virtual StatusCode finalize();

    /** Catalog interface                                                     */
    /// Create file identifier using UUID mechanism
    virtual std::string createFID() const;
    /// Access to connect string
    virtual CSTR connectInfo() const;
    /// Parse the DOM tree of the XML catalog
    virtual void init()
    {  _exec(&IFileCatalog::init); m_started=true;                           }
    /// Save DOM catalog to file
    virtual void commit()                   { _exec(&IFileCatalog::commit);   }
    /// Save DOM catalog to file
    virtual void rollback()                 { _exec(&IFileCatalog::rollback); }
    /// Check if the catalog is read-only
    virtual bool readOnly() const;
    /// Check if the catalog should be updated
    virtual bool dirty() const;
    /// Return the status of physical file name
    virtual bool existsPFN(CSTR pfn)  const
    { return !lookupPFN(pfn).empty();                                         }
    /// Lookup file identifier by physical file name
    virtual std::string lookupPFN(CSTR pfn) const
    {  return _find(pfn,&IFileCatalog::lookupPFN);                            }
    /// Return the status of a logical file name
    virtual bool existsLFN(CSTR lfn)  const
    { return !lookupLFN(lfn).empty();                                         }
    /// Lookup file identifier by logical file name
    virtual std::string lookupLFN(CSTR lfn) const
    {  return _find(lfn,&IFileCatalog::lookupLFN);                            }
    /// Return the status of a FileID
    virtual bool existsFID(CSTR fid)  const
    {  return 0 != getCatalog(fid,false,false,false);                          }
    /// Dump all physical file names of the catalog and their attributes associate to the FileID
    virtual void getPFN(CSTR fid, Files& files) const
    {  _collect(fid,files,&IFileCatalog::getPFN);                             }
    /// Dump all logical file names of the catalog associate to the FileID
    virtual void getLFN(CSTR fid, Files& files) const
    {  _collect(fid,files,&IFileCatalog::getLFN);                             }

    /// Dump all file Identifiers
    virtual void getFID(Strings& fids)  const
    {  _collect(fids,&IFileCatalog::getFID);                                  }
    /// Delete FileID from the catalog
    virtual void deleteFID(CSTR fid)  const
    {  writeCatalog(fid)->deleteFID(fid);                                     }
    /// Create a FileID and DOM Node of the PFN with all the attributes
    virtual void registerPFN(CSTR fid, CSTR pfn, CSTR ftype) const;
    /// Create a FileID and DOM Node of the LFN with all the attributes
    virtual void registerLFN(CSTR fid, CSTR lfn) const;
    /// Create a FileID and DOM Node
    virtual void registerFID(CSTR fid) const
    {  writeCatalog()->registerFID(fid);                                      }
    /// Dump all MetaData of the catalog for a given file ID
    virtual void getMetaData(CSTR fid, Attributes& attr) const
    {  _collect(fid,attr,&IFileCatalog::getMetaData);                         }
    /// Access metadata item
    virtual std::string getMetaDataItem(CSTR fid, CSTR name) const;
    /// Insert/update metadata item
    virtual void setMetaData(CSTR fid, CSTR attr, CSTR val) const
    {  writeCatalog(fid)->setMetaData(fid,attr,val);                          }
    /// Drop all metadata of one FID
    virtual void dropMetaData(CSTR fid) const
    {  writeCatalog(fid)->dropMetaData(fid);                                  }
    /// Drop specified metadata item
    virtual void dropMetaData(CSTR fid, CSTR attr) const
    {  writeCatalog(fid)->dropMetaData(fid,attr);                             }

    /** Catalog management                                                    */
    /// Find catalog by connect string
    virtual IFileCatalog* findCatalog(CSTR connect, bool must_be_writable) const;
    /// Add new catalog identified by name to the existing ones
    virtual void addCatalog(CSTR connect);
    /// Add new catalog identified by reference to the existing ones
    virtual void addCatalog(IFileCatalog* cat);
    /// Remove catalog identified by name from the existing ones
    virtual void removeCatalog(CSTR connect);
    /// Remove catalog identified by reference from the existing ones
    virtual void removeCatalog(const IFileCatalog* cat);
    /// Access catalog container
    virtual Catalogs& catalogs()                    { return m_catalogs;      }
    /// Access catalog container (CONST)
    virtual const Catalogs& catalogs()  const       { return m_catalogs;      }
    /// Access to the (first) writable file catalog
    virtual IFileCatalog* writeCatalog(CSTR fid="") const
    {  return getCatalog(fid,true,true,false);                                }
    /// Define the writable catalog identified by reference
    virtual void setWriteCatalog(IFileCatalog* cat);
    /// Define the writable catalog identified by name
    virtual void setWriteCatalog(CSTR connect);

  private:
    /// Find catalog containing a given file identifier
    IFileCatalog* getCatalog(CSTR fid, bool throw_if_not, bool writable=true, bool prt=true) const;
    /// Find catalog by connect string
    Catalogs::iterator i_findCatalog(CSTR connect, bool must_be_writable);

    /// simple property handle to allow interactive modification of
    /// list of the file catalogs
    void propHandler(Property& /* p */);

    void printError(CSTR msg, bool throw_exc=true) const;
    std::string lookupFID(CSTR lfn) const;

    /// Container with references to known catalogs
    Catalogs        m_catalogs;
    /// Property : Container with catalog names
    CatalogNames    m_catalogNames;
    /// Flag to indicate if catalog is started
    bool            m_started;
    /// BACKUP:: Container with catalog names
    CatalogNames    m_oldNames;
  };
}         /* End namespace Gaudi                 */
#endif    /* GAUDIUTILS_MULTIFILECATALOG_H */
