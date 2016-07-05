#ifndef GAUDIKERNEL_DATAOBJECTHANDLEPROPERTY_H
#define GAUDIKERNEL_DATAOBJECTHANDLEPROPERTY_H 1

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Property.h"

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

class GAUDI_API DataObjectHandleProperty :  public PropertyWithHandlers {
 public:

  DataObjectHandleProperty( const std::string& name, DataObjectHandleBase& ref );
  DataObjectHandleProperty& operator=( const DataObjectHandleBase& value );
  virtual ~DataObjectHandleProperty();

  virtual DataObjectHandleProperty* clone() const;
  virtual bool load( Property& destination ) const;
  virtual bool assign( const Property& source );
  virtual std::string toString() const;
  virtual void toStream(std::ostream& out) const;
  virtual StatusCode fromString(const std::string& s);
  const DataObjectHandleBase& value() const;
  bool setValue( const DataObjectHandleBase& value );

  const std::string pythonRepr() const;

 private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjectHandleBase* m_pValue;
};

template<>
class PropertyWithValue<DataObjectHandleBase&, Gaudi::Details::Property::NullVerifier<DataObjectHandleBase>> :
  public ::DataObjectHandleProperty
{
public:
  PropertyWithValue(const std::string& name, DataObjectHandleBase& value) :
    ::DataObjectHandleProperty(name, value)
  {}

  /// virtual Destructor
  virtual ~PropertyWithValue() {}
};


#endif
