// $Id: TESSerializer.h,v 1.1 2008/10/17 08:56:05 mato Exp $
#ifndef GAUDIPYTHON_TESSERIALIZER_H
#define GAUDIPYTHON_TESSERIALIZER_H

#include "GaudiKernel/IDataStoreAgent.h"

// vector and string
#include <string>
#include <vector>

// forward declarations
class IDataProviderSvc;
class IDataManagerSvc;
class TBufferFile;
class DataStoreItem;
class DataObject;

namespace GaudiPython {
  class GAUDI_API TESSerializer :  virtual public IDataStoreAgent {
    typedef std::vector<DataStoreItem*> Items;
    typedef std::vector<std::string>    ItemNames;
    typedef std::vector<DataObject*>    Objects;
  public:
    TESSerializer(IDataProviderSvc* svc);

    void dumpBuffer(TBufferFile&);
    void loadBuffer(TBufferFile&);

    void addItem(const std::string& path, int level);
    void addOptItem(const std::string& path, int level);

    /// Analysis callback
    virtual bool analyse(IRegistry* dir, int level);

  protected:
    void addItem(Items& itms, const std::string& path, int level);
  private:
     /// TES pointer
    IDataProviderSvc* m_TES;
     /// TES pointer
    IDataManagerSvc*  m_TESMgr;
    /// Vector of item names
    ItemNames         m_itemNames;
    /// Vector of items to be saved to this stream
    Items             m_itemList;
    /// Vector of item names
    ItemNames         m_optItemNames;
    /// Vector of optional items to be saved to this stream
    Items             m_optItemList;
    /// Current item while traversing the TES tree
    DataStoreItem*    m_currentItem;
    /// Selected list of Objects to be serialized
    Objects           m_objects;
  };
}
#endif


