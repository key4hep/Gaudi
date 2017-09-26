#ifndef GAUDIKERNEL_DATAOBJIDPROPERTY_H
#define GAUDIKERNEL_DATAOBJIDPROPERTY_H 1

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ToStream.h"

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

class GAUDI_API DataObjIDProperty : public PropertyWithHandlers<>
{
public:
  DataObjIDProperty( const std::string& name, DataObjID& ref );
  DataObjIDProperty& operator=( const DataObjID& value );
  ~DataObjIDProperty() override;

  DataObjIDProperty* clone() const override;
  bool load( Property& destination ) const override;
  bool assign( const Property& source ) override;
  std::string toString() const override;
  void toStream( std::ostream& out ) const override;
  StatusCode fromString( const std::string& s ) override;
  const DataObjID& value() const;
  bool setValue( const DataObjID& value );

private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjID* m_pValue;
};

namespace Gaudi
{
  template <>
  class Property<DataObjID&> : public ::DataObjIDProperty
  {
  public:
    Property( const std::string& name, DataObjID& value ) : ::DataObjIDProperty( name, value ) {}

    /// virtual Destructor
    virtual ~Property() {}
  };
} // namespace Gaudi

inline DataObjIDProperty& DataObjIDProperty::operator=( const DataObjID& value )
{
  setValue( value );
  return *this;
}

inline DataObjIDProperty* DataObjIDProperty::clone() const { return new DataObjIDProperty( *this ); }

inline bool DataObjIDProperty::load( Gaudi::Details::PropertyBase& destination ) const
{
  return destination.assign( *this );
}

inline bool DataObjIDProperty::assign( const Gaudi::Details::PropertyBase& source )
{
  return fromString( source.toString() ).isSuccess();
}

inline const DataObjID& DataObjIDProperty::value() const
{
  useReadHandler();
  return *m_pValue;
}

//---------------------------------------------------------------------------

class GAUDI_API DataObjIDCollProperty : public PropertyWithHandlers<>
{
public:
  DataObjIDCollProperty( const std::string& name, DataObjIDColl& ref );
  DataObjIDCollProperty& operator=( const DataObjIDColl& value );
  virtual ~DataObjIDCollProperty();

  DataObjIDCollProperty* clone() const override;
  bool load( Property& destination ) const override;
  bool assign( const Property& source ) override;
  std::string toString() const override;
  void toStream( std::ostream& out ) const override;
  StatusCode fromString( const std::string& s ) override;
  const DataObjIDColl& value() const;
  bool setValue( const DataObjIDColl& value );

private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjIDColl* m_pValue;
};

namespace Gaudi
{
  template <>
  class Property<DataObjIDColl&> : public ::DataObjIDCollProperty
  {
  public:
    Property( const std::string& name, DataObjIDColl& value ) : ::DataObjIDCollProperty( name, value ) {}

    /// virtual Destructor
    virtual ~Property() {}
  };

  template <>
  class Property<DataObjIDColl> : public ::DataObjIDCollProperty
  {
  public:
    Property( const std::string& name, DataObjIDColl& value ) : ::DataObjIDCollProperty( name, value ) {}

    /// Autodeclaring constructor with property name, value and documentation.
    template <class OWNER>
    inline Property( OWNER* owner, const std::string& name, const DataObjIDColl& initval = DataObjIDColl{},
                     std::string doc = "" )
        : ::DataObjIDCollProperty( name, *( new DataObjIDColl{initval} ) )
    {
      /// \fixme{DataObjIDCollProperty is a ref property by construction and this
      ///        is a temporary hack to have a "value" property}
      m_data.reset( const_cast<DataObjIDColl*>( &value() ) );
      setDocumentation( std::move( doc ) );
      owner->declareProperty( *this );
      setOwnerType<OWNER>();
    }

    operator const DataObjIDColl&() const { return value(); }

    /// virtual Destructor
    virtual ~Property() {}

  private:
    std::unique_ptr<DataObjIDColl> m_data;
  };
} // namespace Gaudi

inline DataObjIDCollProperty& DataObjIDCollProperty::operator=( const DataObjIDColl& value )
{
  setValue( value );
  return *this;
}

inline DataObjIDCollProperty* DataObjIDCollProperty::clone() const { return new DataObjIDCollProperty( *this ); }

inline bool DataObjIDCollProperty::load( Gaudi::Details::PropertyBase& destination ) const
{
  return destination.assign( *this );
}

inline bool DataObjIDCollProperty::assign( const Gaudi::Details::PropertyBase& source )
{
  return fromString( source.toString() ).isSuccess();
}

inline const DataObjIDColl& DataObjIDCollProperty::value() const
{
  useReadHandler();
  return *m_pValue;
}

#endif
