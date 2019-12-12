/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_IPROPERTY_H
#define GAUDIKERNEL_IPROPERTY_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/PropertyFwd.h"
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
class GAUDI_API IProperty : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IProperty, 2, 1 );

  /// Set the property by property
  virtual StatusCode setProperty( const Gaudi::Details::PropertyBase& p // Reference to the input property
                                  ) = 0;
  /// Set the property by string
  virtual StatusCode setProperty( const std::string& s ) = 0;
  /// Set the property by std::string
  virtual StatusCode setProperty( const std::string& n, const std::string& v ) = 0;
  /// Get the property by property
  virtual StatusCode getProperty( Gaudi::Details::PropertyBase* p // Pointer to property to be set
                                  ) const = 0;
  /// Get the property by name
  virtual const Gaudi::Details::PropertyBase& getProperty( const std::string& name // Property name
                                                           ) const = 0;
  /// Get the property by std::string
  virtual StatusCode getProperty( const std::string& n, std::string& v ) const = 0;
  /// Get list of properties
  virtual const std::vector<Gaudi::Details::PropertyBase*>& getProperties() const = 0;

  /// Return true if we have a property with the given name.
  virtual bool hasProperty( const std::string& name ) const = 0;
};
#endif // GAUDIKERNEL_IPROPERTY_H
