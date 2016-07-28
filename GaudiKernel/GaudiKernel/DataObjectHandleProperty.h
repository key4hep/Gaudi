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

class GAUDI_API DataObjectHandleProperty :  public ::Property {
 public:

  DataObjectHandleProperty( const std::string& name, DataObjectHandleBase& ref );
  DataObjectHandleProperty& operator=( const DataObjectHandleBase& value );
  virtual ~DataObjectHandleProperty();

  DataObjectHandleProperty* clone() const override;
  bool load( Property& destination ) const override;
  bool assign( const Property& source ) override;
  std::string toString() const override;
  void toStream(std::ostream& out) const override;
  StatusCode fromString(const std::string& s) override;
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
class SimplePropertyRef< DataObjectHandleBase > :
  public ::DataObjectHandleProperty
{
public:
  SimplePropertyRef(const std::string& name, DataObjectHandleBase& value) :
    ::DataObjectHandleProperty(name, value)
  {}

  /// virtual Destructor
  virtual ~SimplePropertyRef() {}
};


#endif
