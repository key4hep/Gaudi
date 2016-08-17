#ifndef GAUDIHIVE_DATAOBJECTHANDLEBASE_H
#define GAUDIHIVE_DATAOBJECTHANDLEBASE_H

#include <mutex>

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IMessageSvc.h"

//---------------------------------------------------------------------------

/** DataObjectHandleBase  GaudiKernel/DataObjectHandleBase.h
 *
 * Base class for handles to Data Objects in the Event Store, to simplify
 * access via Properties.
 *
 * provides accessors to AlternateNames, and Optional parameters
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class DataObjectHandleBase : public Gaudi::DataHandle {

public:

  DataObjectHandleBase(const DataObjID& k, Gaudi::DataHandle::Mode a,
		   IDataHandleHolder* o, std::vector<std::string> alternates = {} );
  DataObjectHandleBase(const std::string& k, Gaudi::DataHandle::Mode a,
		   IDataHandleHolder* o);
  virtual ~DataObjectHandleBase();
  DataObjectHandleBase(const DataObjectHandleBase&) = delete;
  DataObjectHandleBase(DataObjectHandleBase&&);

  DataObjectHandleBase& operator=(const DataObjectHandleBase&);

  std::string toString() const;
  std::string pythonRepr() const;
  void fromString(const std::string& s);

  friend std::ostream& operator<< (std::ostream& str, const DataObjectHandleBase& d);

  /// Check if the data object declared is optional for the algorithm
  bool isOptional() const {return m_optional;}
  void setOptional(bool optional = true) { m_optional = optional; }

  const std::vector<std::string> & alternativeDataProductNames() const
  { return m_altNames; }
  void setAlternativeDataProductNames(const std::vector<std::string> & alternativeAddresses) const
  { m_altNames = alternativeAddresses; }

  bool initialized() const { return m_init; }
  bool wasRead() const { return m_wasRead; }
  bool wasWritten() const { return m_wasWritten; }

  bool isValid() const;

protected:
  void setRead(bool wasRead=true) {m_wasRead = wasRead;}
  void setWritten(bool wasWritten=true) {m_wasWritten = wasWritten;}

  virtual void init();

  DataObject* fetch() const;

protected:

  SmartIF<IDataProviderSvc> m_EDS;
  SmartIF<IMessageSvc> m_MS;

  bool  m_init = false;
  mutable bool  m_goodType = false;
  bool  m_optional = false;
  bool  m_wasRead = false;
  bool  m_wasWritten = false;

  /**
   * Whether the search part of the fetch method (so dealing with alt names
   * was already executed or not. On subsequent calls (when this is true),
   * it will be skipped.
   */
  mutable bool  m_searchDone = false;

  /**
   * A Mutex protecting the calls to the search part of the fetch method,
   * so that we are sure that we only call it once
   */
  mutable std::mutex m_searchMutex;

  mutable std::vector<std::string> m_altNames;

};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


namespace Gaudi {
  namespace Parsers {
    StatusCode parse(DataObjectHandleBase&, const std::string&);
  }
  namespace Utils {
    GAUDI_API std::ostream& toStream(const DataObjectHandleBase& v, std::ostream& o);
  }
}

#endif
