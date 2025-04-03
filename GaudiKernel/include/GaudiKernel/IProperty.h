/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include Files
#include <Gaudi/Details/PropertyBase.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/ToStream.h>
#include <iostream>
#include <string>
#include <string_view>
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
  DeclareInterfaceID( IProperty, 3, 0 );

  /// Set the property from a property
  StatusCode setProperty( const Gaudi::Details::PropertyBase& p ) { return setProperty( p.name(), p ); }
  /// Set the property from a property with a different name
  virtual StatusCode setProperty( const std::string& name, const Gaudi::Details::PropertyBase& p ) = 0;
  /// Set the property by string
  virtual StatusCode setProperty( const std::string& s ) = 0;
  /// Set the property by name and value representation
  virtual StatusCode setPropertyRepr( const std::string& n, const std::string& r ) = 0;
  /// Special case for string literals
  StatusCode setProperty( const std::string& name, const char* v ) { return setProperty( name, std::string{ v } ); }
  /// Special case for std::string
  StatusCode setProperty( const std::string& name, const std::string& v ) {
    if ( !hasProperty( name ) ) return StatusCode::FAILURE;
    return setPropertyRepr( name, v );
  }

  /** set the property form the value
   *
   *  @code
   *
   *  std::vector<double> data = ... ;
   *  setProperty( "Data" , data ) ;
   *
   *  std::map<std::string,double> cuts = ... ;
   *  setProperty( "Cuts" , cuts ) ;
   *
   *  std::map<std::string,std::string> dict = ... ;
   *  setProperty( "Dictionary" , dict ) ;
   *
   *  @endcode
   *
   *  Note: the interface IProperty allows setting of the properties either
   *        directly from other properties or from strings only
   *
   *  This is very convenient in resetting of the default
   *  properties in the derived classes.
   *
   *  @param name      name of the property
   *  @param value     value of the property
   *  @see Gaudi::Utils::setProperty
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2007-05-13
   */
  template <class TYPE>
    requires( !std::is_base_of_v<Gaudi::Details::PropertyBase, TYPE> )
  StatusCode setProperty( const std::string& name, const TYPE& value ) {
    using Gaudi::Utils::toString;
    if ( !hasProperty( name ) ) return StatusCode::FAILURE;
    return setPropertyRepr( name, toString( value ) );
  }
  /// Get the property by property
  virtual StatusCode getProperty( Gaudi::Details::PropertyBase* p // Pointer to property to be set
  ) const = 0;
  /// Get the property by name
  virtual const Gaudi::Details::PropertyBase& getProperty( std::string_view name // Property name
  ) const = 0;
  /// Get the property by std::string
  virtual StatusCode getProperty( std::string_view n, std::string& v ) const = 0;
  /// Get list of properties
  virtual const std::vector<Gaudi::Details::PropertyBase*>& getProperties() const = 0;

  /// Return true if we have a property with the given name.
  virtual bool hasProperty( std::string_view name ) const = 0;
};
