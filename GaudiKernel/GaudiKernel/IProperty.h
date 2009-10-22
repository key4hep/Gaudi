// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IProperty.h,v 1.6 2006/06/06 16:16:23 hmd Exp $
#ifndef GAUDIKERNEL_IPROPERTY_H
#define GAUDIKERNEL_IPROPERTY_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include <iostream>
#include <string>
#include <vector>

/** @class IProperty IProperty.h GaudiKernel/IProperty.h

    The IProperty is the basic interface for all components which have
    properties that can be set or get.

    @author Paul Maley
    @author Pere Mato
    @date   29/10/98
*/
class Property;

class GAUDI_API IProperty: virtual public IInterface  {
public:
  /// InterfaceID
  DeclareInterfaceID(IProperty,2,0);

  /// Set the property by property
  virtual StatusCode setProperty( const Property& p // Reference to the input property
                                ) = 0;
  /// Set the property by string
  virtual StatusCode setProperty( const std::string& s ) = 0;
  /// Set the property by std::string
  virtual StatusCode setProperty( const std::string& n, const std::string& v ) = 0;
  /// Get the property by property
  virtual StatusCode getProperty( Property* p       // Pointer to property to be set
                                ) const = 0;
  /// Get the property by name
  virtual const Property& getProperty( const std::string& name  // Property name
                                ) const = 0;
  /// Get the property by std::string
  virtual StatusCode getProperty( const std::string& n, std::string& v ) const = 0;
  /// Get list of properties
  virtual const std::vector<Property*>& getProperties( ) const = 0;
};
#endif  // GAUDIKERNEL_IPROPERTY_H


