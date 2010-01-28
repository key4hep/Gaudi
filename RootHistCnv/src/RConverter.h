// $Id: RConverter.h,v 1.8 2006/11/30 20:51:35 mato Exp $
#ifndef ROOTHISTCNV_RCONVERTER_H
#define ROOTHISTCNV_RCONVERTER_H 1

// Include files
#include "GaudiKernel/Converter.h"
#include "GaudiKernel/CnvFactory.h"
#include "TDirectory.h"
#include <string>
#include <map>

// Forward declarations
class ISvcLocator;
class TObject;
class TTree;
class TFile;

class GlobalDirectoryRestore  {
  TDirectory* m_current;
public:
  GlobalDirectoryRestore() { 
    m_current = gDirectory;
  }
  virtual ~GlobalDirectoryRestore()  {
    gDirectory = m_current;
  }
};

namespace RootHistCnv {

  /** @class RootHistCnv::RConverter RConverter.h

    Root Converter
    @author Charles Leggett
  */
  class RConverter : public Converter   {
  public:
    /// Convert the transient object to the requested representation.
    virtual StatusCode createRep(DataObject*      pObj,
                                 IOpaqueAddress*& refpAddr);
    /// Inquire storage type
    static long storageType()    {         return ROOT_StorageType;    }
    long repSvcType() const      {         return i_repSvcType();      }

    StatusCode error(const std::string& msg);
    StatusCode regTFile(const std::string, const TFile*);
    StatusCode findTFile(const std::string, TFile*&);

    std::string diskDirectory(const std::string& loc);
    std::string directory(const std::string& loc);
    void setDirectory(const std::string& loc);
    void setDiskDirectory(const std::string& loc);
    StatusCode createDirectory(const std::string& loc);
    StatusCode createDiskDirectory(const std::string& loc);
    std::string getDirectory();

    std::string convertId(const std::string& ) const ;

  protected:
    /// Standard constructor
    RConverter( const CLID& clid, ISvcLocator* svc )
    : Converter(storageType(), clid, svc)  {    }
    /// Standard destructor
    virtual ~RConverter() {     }
    /// Create the transient representation of an object.
    virtual StatusCode readObject( IOpaqueAddress* pAddr, 
                                   DataObject*& refpObj);
    /// Create the persistent representation of an object.
    virtual TObject* createPersistent(DataObject* pObj);
    /// Create address of the transient object according to the requested representation.
    StatusCode createAddress( DataObject* pObject, 
                              TDirectory* pDir,
                              TObject* pTObject, 
                              IOpaqueAddress*& refpAddr);

    StatusCode createAddress(const std::string& rzdir, 
			     const CLID& clid, 
			     long id, 
			     TObject* pTobj,
			     IOpaqueAddress*& refpAddress);

    StatusCode createAddress(const std::string& rzdir, 
			     const CLID& clid, 
			     const std::string& title, 
			     TObject* pTobj,
			     IOpaqueAddress*& refpAddress);

    /// Switch to object directory (=Parent directory)
    TDirectory* changeDirectory(DataObject* pObject);
  };
} // namespace RootHistCnv

#endif    // RootHistCnv_RConverter_H
