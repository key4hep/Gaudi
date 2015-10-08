#ifndef GAUDIKERNEL_DATAHANDLE
#define GAUDIKERNEL_DATAHANDLE 1

#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/ToStream.h"
#include "GaudiKernel/DataObjID.h"
#include <sstream>

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

  virtual const std::string pythonRepr() const {
    std::ostringstream ost;
    ost << "DataHandle(\""  << fullKey()  <<  "|" 
        << std::to_string(mode()) <<  "\")";
    return ost.str();
  }

protected:
  virtual void setMode(const Mode& mode) { m_mode = mode; }

  DataObjID               m_key;
  IDataHandleHolder*      m_owner;

private:
  Mode                    m_mode;

};

}

namespace Gaudi { 
  namespace Parsers {

    GAUDI_API
    StatusCode parse(DataHandle&, const std::string&);

  } //> ns Parsers

  namespace Utils {

    GAUDI_API
    std::ostream& 
    toStream(const DataHandle& v, std::ostream& o);

  } //> ns Utils
} //> ns Gaudi



class GAUDI_API DataHandleProperty : public Property {
 public:

  /// Constructor with parameters: 
  DataHandleProperty( const std::string& name, Gaudi::DataHandle& ref );

  /// Assignment operator: 
  DataHandleProperty& operator=( const Gaudi::DataHandle& value );

  /// Destructor: 
  virtual ~DataHandleProperty();

  virtual DataHandleProperty* clone() const;
    
  virtual bool load( Property& destination ) const;

  virtual bool assign( const Property& source );

  virtual std::string toString() const;

  virtual void toStream(std::ostream& out) const;

  virtual StatusCode fromString(const std::string& s);

  const Gaudi::DataHandle& value() const;

  bool setValue( const Gaudi::DataHandle& value );

 private:
  /** Pointer to the real property. Reference would be better, 
   *  but Reflex does not support references yet
   */
  Gaudi::DataHandle* m_pValue;
};

inline
DataHandleProperty& 
DataHandleProperty::operator=( const Gaudi::DataHandle& value ) 
{
  setValue( value );
  return *this;
}

inline
DataHandleProperty* 
DataHandleProperty::clone() const 
{
  return new DataHandleProperty( *this );
}

inline
bool
DataHandleProperty::load( Property& destination ) const 
{
  return destination.assign( *this );
}

inline
bool
DataHandleProperty::assign( const Property& source ) 
{
  return fromString( source.toString() ).isSuccess();
}

inline
const Gaudi::DataHandle& 
DataHandleProperty::value() const 
{
  useReadHandler();
  return *m_pValue;
}


template<>
class SimplePropertyRef< Gaudi::DataHandle > : public DataHandleProperty {
public:
  SimplePropertyRef(const std::string& name, Gaudi::DataHandle& value) :
    ::DataHandleProperty(name, value)
  {}

  /// virtual Destructor
  virtual ~SimplePropertyRef() {}
};



#endif
