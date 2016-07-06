#ifndef GAUDIKERNEL_DATAOBJIDPROPERTY_H
#define GAUDIKERNEL_DATAOBJIDPROPERTY_H 1

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/ToStream.h"
#include "GaudiKernel/DataObjID.h"

#include <iostream>

//---------------------------------------------------------------------------

/** DataObjIDProperty.h GaudiKernel/DataObjProperty.h
 *
 * Helper class to manage DataObjIDs and DataObjIDColls as properties
 *
 * TODO: DataObjIDCollProperty implementation
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------


class GAUDI_API DataObjIDProperty :  public PropertyWithHandlers {
 public:

  DataObjIDProperty( const std::string& name, DataObjID& ref );
  DataObjIDProperty& operator=( const DataObjID& value );
  virtual ~DataObjIDProperty();

  virtual DataObjIDProperty* clone() const;
  virtual bool load( Property& destination ) const;
  virtual bool assign( const Property& source );
  virtual std::string toString() const;
  virtual void toStream(std::ostream& out) const;
  virtual StatusCode fromString(const std::string& s);
  const DataObjID& value() const;
  bool setValue( const DataObjID& value );

 private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjID* m_pValue;
};

template<>
class PropertyWithValue<DataObjID&, Gaudi::Details::Property::NullVerifier> :
  public ::DataObjIDProperty
{
public:
  PropertyWithValue(const std::string& name, DataObjID& value) :
    ::DataObjIDProperty(name, value)
  {}

  /// virtual Destructor
  virtual ~PropertyWithValue() {}
};


inline
DataObjIDProperty&
DataObjIDProperty::operator=( const DataObjID& value )
{
  setValue( value );
  return *this;
}

inline
DataObjIDProperty*
DataObjIDProperty::clone() const
{
  return new DataObjIDProperty( *this );
}

inline
bool
DataObjIDProperty::load( Property& destination ) const
{
  return destination.assign( *this );
}

inline
bool
DataObjIDProperty::assign( const Property& source )
{
  return fromString( source.toString() ).isSuccess();
}

inline
const DataObjID&
DataObjIDProperty::value() const
{
  useReadHandler();
  return *m_pValue;
}

//---------------------------------------------------------------------------

class GAUDI_API DataObjIDCollProperty :  public PropertyWithHandlers {
 public:

  DataObjIDCollProperty( const std::string& name, DataObjIDColl& ref );
  DataObjIDCollProperty& operator=( const DataObjIDColl& value );
  virtual ~DataObjIDCollProperty();

  virtual DataObjIDCollProperty* clone() const;
  virtual bool load( Property& destination ) const;
  virtual bool assign( const Property& source );
  virtual std::string toString() const;
  virtual void toStream(std::ostream& out) const;
  virtual StatusCode fromString(const std::string& s);
  const DataObjIDColl& value() const;
  bool setValue( const DataObjIDColl& value );

 private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjIDColl* m_pValue;
};

template<>
class PropertyWithValue<DataObjIDColl&, Gaudi::Details::Property::NullVerifier> :
  public ::DataObjIDCollProperty
{
public:
  PropertyWithValue(const std::string& name, DataObjIDColl& value) :
    ::DataObjIDCollProperty(name, value)
  {}

  /// virtual Destructor
  virtual ~PropertyWithValue() {}
};

template<>
class PropertyWithValue<DataObjIDColl, Gaudi::Details::Property::NullVerifier> :
  public ::DataObjIDCollProperty
{
public:
  PropertyWithValue(const std::string& name, DataObjIDColl& value) :
    ::DataObjIDCollProperty(name, value)
  {}

  /// Autodeclaring constructor with property name, value and documentation.
  template <class OWNER>
  inline PropertyWithValue( OWNER* owner, const std::string& name, const DataObjIDColl& initval = DataObjIDColl{}, std::string doc = "" )
      : ::DataObjIDCollProperty( name, *(new DataObjIDColl{initval}) )
  {
    /// \fixme{DataObjIDCollProperty is a ref property by construction and this
    ///        is a temporary hack to have a "value" property}
    m_data.reset( const_cast<DataObjIDColl*>( &value() ) );
    setDocumentation( std::move( doc ) );
    owner->declareProperty( *this );
  }

  operator const DataObjIDColl& () const { return value(); }

  /// virtual Destructor
  virtual ~PropertyWithValue() {}
private:
  std::unique_ptr<DataObjIDColl> m_data;
};


inline
DataObjIDCollProperty&
DataObjIDCollProperty::operator=( const DataObjIDColl& value )
{
  setValue( value );
  return *this;
}

inline
DataObjIDCollProperty*
DataObjIDCollProperty::clone() const
{
  return new DataObjIDCollProperty( *this );
}

inline
bool
DataObjIDCollProperty::load( Property& destination ) const
{
  return destination.assign( *this );
}

inline
bool
DataObjIDCollProperty::assign( const Property& source )
{
  return fromString( source.toString() ).isSuccess();
}

inline
const DataObjIDColl&
DataObjIDCollProperty::value() const
{
  useReadHandler();
  return *m_pValue;
}




#endif
