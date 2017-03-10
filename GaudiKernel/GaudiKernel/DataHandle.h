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


  DataHandle(const DataObjID& k, Mode a=Reader,
	     IDataHandleHolder* owner=nullptr):
    m_key(k), m_owner(owner), m_mode(a) {};

  DataHandle(const DataObjID& k, const bool& isCond, Mode a=Reader,
	     IDataHandleHolder* owner=nullptr):
    m_key(k), m_owner(owner), m_mode(a), m_isCond(isCond) {};


  virtual ~DataHandle() = default;

  virtual void setOwner(IDataHandleHolder* o) { m_owner = o; }
  virtual IDataHandleHolder* owner() const { return m_owner; }

  virtual Mode mode() const { return m_mode; }

  virtual void setKey(const DataObjID& key) const { m_key = key; }
  virtual void updateKey(const std::string& key) const { m_key.updateKey(key); }

  virtual const std::string& objKey() const { return m_key.key(); }
  virtual const DataObjID& fullKey() const { return m_key; }

  virtual void reset(bool) {};
  virtual StatusCode commit() { return StatusCode::SUCCESS; }

  virtual std::string pythonRepr() const;
  virtual bool init() { return true; }

  // is this a ConditionHandle?
  virtual bool isCondition() const { return m_isCond; }

protected:

  /**
   * The key of the object behind this DataHandle
   * Although it may look strange to have it mutable, this can actually
   * change in case the object had alternative names, and it should not
   * be visible to the end user, for which the Handle is still the same
   */
  mutable DataObjID       m_key = { "NONE" };
  IDataHandleHolder*      m_owner = nullptr;

private:
  Mode                    m_mode = Reader;
  bool                    m_isCond = false;

};

}

#endif
