#ifndef GAUDIKERNEL_DATAOBJECTHANDLEPROPERTY_H
#define GAUDIKERNEL_DATAOBJECTHANDLEPROPERTY_H 1

#include "GaudiKernel/Property.h"
#include "GaudiKernel/StatusCode.h"

#include <iostream>

class DataObjectHandleBase;

//---------------------------------------------------------------------------

/** DataObjectHandleProperty.h GaudiKernel/DataObjectHandleProperty.h
 *
 * Helper class to manage DataObjectHandles and DataObjectHandleColls as properties
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class GAUDI_API DataObjectHandleProperty : public PropertyWithHandlers
{
public:
  DataObjectHandleProperty( const std::string& name, DataObjectHandleBase& ref );
  DataObjectHandleProperty& operator=( const DataObjectHandleBase& value );
  virtual ~DataObjectHandleProperty();

  virtual DataObjectHandleProperty* clone() const;
  virtual bool load( Gaudi::Details::PropertyBase& destination ) const;
  virtual bool assign( const Gaudi::Details::PropertyBase& source );
  virtual std::string toString() const;
  virtual void toStream( std::ostream& out ) const;
  virtual StatusCode fromString( const std::string& s );
  const DataObjectHandleBase& value() const;
  bool setValue( const DataObjectHandleBase& value );

  const std::string pythonRepr() const;

private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjectHandleBase* m_pValue;
};

namespace Gaudi
{
  template <>
  class Property<DataObjectHandleBase&> : public ::DataObjectHandleProperty
  {
  public:
    Property( const std::string& name, DataObjectHandleBase& value ) : ::DataObjectHandleProperty( name, value ) {}

    /// virtual Destructor
    virtual ~Property() {}
  };
}

#endif
