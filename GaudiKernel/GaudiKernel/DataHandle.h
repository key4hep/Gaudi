#ifndef GAUDIKERNEL_DATAHANDLE
#define GAUDIKERNEL_DATAHANDLE 1

#include "GaudiKernel/DataObjID.h"

//---------------------------------------------------------------------------

/** DataHandle.h  GaudiKernel/DataHandle.h
 *
 * Base class for Handles to access data in Event Store, shared between
 * Gaudi and Atlas
 *
 * Objects are identified via a DataObjID
 *
 * Once object is created, Mode is not modifiable
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class IDataHandleHolder;

namespace Gaudi {

class DataHandle {
public:

  enum Mode {
    Reader = 1<<2,
    Writer = 1<<4,
    Updater = Reader | Writer
  };


  DataHandle() : m_key("NONE"),  m_owner(0), m_mode(Reader) {};
  DataHandle(const DataObjID& k, Mode a=Reader,
	     IDataHandleHolder* owner=0): 
    m_key(k), m_owner(owner), m_mode(a){};

  virtual ~DataHandle(){}

  virtual void setOwner(IDataHandleHolder* o) { m_owner = o; }
  virtual IDataHandleHolder* owner() const { return m_owner; }

  virtual Mode mode() const { return m_mode; }

  virtual void setKey(const DataObjID& key) { m_key = key; }
  virtual void updateKey(const std::string& key) { m_key.updateKey(key); }

  virtual const std::string& objKey() const { return m_key.key(); }
  virtual const DataObjID& fullKey() const { return m_key; }

  virtual void reset(bool) {};
  virtual StatusCode commit() { return StatusCode::SUCCESS; }

  virtual const std::string pythonRepr() const;

protected:
  virtual void setMode(const Mode& mode) { m_mode = mode; }

  DataObjID               m_key;
  IDataHandleHolder*      m_owner;

private:
  Mode                    m_mode;

};

}

#endif
