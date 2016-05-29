#ifndef GAUDIHIVE_DATAOBJECTHANDLEBASE_H
#define GAUDIHIVE_DATAOBJECTHANDLEBASE_H

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

  DataObjectHandleBase() = default;
  DataObjectHandleBase(const DataObjID& k, Gaudi::DataHandle::Mode a,
		   IDataHandleHolder* o, std::vector<std::string> alternates = {} );
  DataObjectHandleBase(const std::string& k, Gaudi::DataHandle::Mode a,
		   IDataHandleHolder* o);


  std::string toString() const;
  std::string pythonRepr() const;
  void fromString(const std::string& s);

  friend std::ostream& operator<< (std::ostream& str, const DataObjectHandleBase& d);


  /// Check if the data object declared is optional for the algorithm
  bool isOptional() const {return m_optional;}
  void setOptional(bool optional = true) { m_optional = optional; }

  const std::vector<std::string> & alternativeDataProductNames() const
  { return m_altNames; }
  void setAlternativeDataProductNames(const std::vector<std::string> & alternativeAddresses)
  { m_altNames = alternativeAddresses; }

  bool initialized() const { return m_init; }
  bool wasRead() const { return m_wasRead; }
  bool wasWritten() const { return m_wasWritten; }

  bool isValid() const;

protected:
  void setRead(bool wasRead=true) {m_wasRead = wasRead;}
  void setWritten(bool wasWritten=true) {m_wasWritten = wasWritten;}

  void init();

  DataObject* fetch();

protected:

  SmartIF<IDataProviderSvc> m_EDS;
  SmartIF<IMessageSvc> m_MS;

  bool  m_init = false;
  bool  m_goodType = false;
  bool  m_optional = false;
  bool  m_wasRead = false;
  bool  m_wasWritten = false;

  std::vector<std::string> m_altNames;

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
