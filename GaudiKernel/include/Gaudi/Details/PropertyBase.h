/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/PropertyFwd.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/System.h>
#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>

namespace Gaudi::Details {
  class WeakPropertyRef;

  /** PropertyBase base class allowing PropertyBase* collections to be "homogeneous"
   *
   * \author Paul Maley
   * \author CTDay
   * \author Vanya BELYAEV ibelyaev@physics.syr.edu
   * \author Marco Clemencic
   */
  class GAUDI_API PropertyBase {

  public:
    /// property name
    const std::string name() const { return std::string{ m_name }; }
    /// property documentation
    std::string documentation() const { return std::string{ m_documentation }; }
    /// property semantics
    std::string semantics() const { return std::string{ m_semantics }; }
    /// property type-info
    const std::type_info* type_info() const { return m_typeinfo; }
    /// property type
    std::string type() const { return m_typeinfo->name(); }
    ///  export the property value to the destination
    virtual bool load( PropertyBase& dest ) const = 0;
    /// import the property value form the source
    virtual bool assign( const PropertyBase& source ) = 0;

  public:
    /// value  -> string
    virtual std::string toString() const = 0;
    /// value  -> stream
    virtual void toStream( std::ostream& out ) const = 0;
    /// string -> value
    virtual StatusCode fromString( const std::string& value ) = 0;

  public:
    /// set new callback for reading
    virtual PropertyBase& declareReadHandler( std::function<void( PropertyBase& )> fun ) = 0;
    /// set new callback for update
    virtual PropertyBase& declareUpdateHandler( std::function<void( PropertyBase& )> fun ) = 0;

    /// get a reference to the readCallBack
    virtual const std::function<void( PropertyBase& )> readCallBack() const = 0;
    /// get a reference to the updateCallBack
    virtual const std::function<void( PropertyBase& )> updateCallBack() const = 0;

    /// manual trigger for callback for update
    virtual bool useUpdateHandler() = 0;

    template <class HT>
    PropertyBase& declareReadHandler( void ( HT::*MF )( PropertyBase& ), HT* instance ) {
      return declareReadHandler( [=]( PropertyBase& p ) { ( instance->*MF )( p ); } );
    }

    template <class HT>
    PropertyBase& declareUpdateHandler( void ( HT::*MF )( PropertyBase& ), HT* instance ) {
      return declareUpdateHandler( [=]( PropertyBase& p ) { ( instance->*MF )( p ); } );
    }

  public:
    /// virtual destructor
    virtual inline ~PropertyBase();
    /// set the new value for the property name
    void setName( std::string value ) { m_name = to_view( std::move( value ) ); }
    /// set the documentation string
    void setDocumentation( std::string value ) { m_documentation = to_view( std::move( value ) ); }
    /// set the semantics string
    void setSemantics( std::string value ) { m_semantics = to_view( std::move( value ) ); }
    /// the printout of the property value
    virtual std::ostream& fillStream( std::ostream& ) const;
    /// clones the current property
    virtual PropertyBase* clone() const = 0;

    /// set the type of the owner class (used for documentation)
    void setOwnerType( const std::type_info& ownerType ) { m_ownerType = &ownerType; }

    /// set the type of the owner class (used for documentation)
    template <class OWNER>
    void setOwnerType() {
      setOwnerType( typeid( OWNER ) );
    }

    /// get the type of the owner class (used for documentation)
    const std::type_info* ownerType() const { return m_ownerType; }

    /// get the string for the type of the owner class (used for documentation)
    std::string ownerTypeName() const {
      return m_ownerType ? System::typeinfoName( *m_ownerType ) : std::string( "unknown owner type" );
    }

  protected:
    /// constructor from the property name and the type
    PropertyBase( const std::type_info& type, std::string name = "", std::string doc = "", std::string semantics = "" )
        : m_name( to_view( std::move( name ) ) )
        , m_documentation( to_view( std::move( doc ) ) )
        , m_semantics( to_view( std::move( semantics ) ) )
        , m_typeinfo( &type ) {}
    /// constructor from the property name and the type
    PropertyBase( std::string name, const std::type_info& type )
        : m_name( to_view( std::move( name ) ) ), m_documentation( m_name ), m_typeinfo( &type ) {}
    /// copy constructor
    PropertyBase( const PropertyBase& ) = default;
    /// assignment operator
    PropertyBase& operator=( const PropertyBase& ) = default;

  private:
    /// helper to map a string to a reliable std::string_view
    static std::string_view to_view( std::string str );
    /// property name
    std::string_view m_name;
    /// property doc string
    std::string_view m_documentation;
    /// property semantics
    std::string_view m_semantics;
    /// property type
    const std::type_info* m_typeinfo;
    /// type of owner of the property (if defined)
    const std::type_info* m_ownerType = nullptr;

    friend WeakPropertyRef;
    std::set<WeakPropertyRef*> m_weakReferences;
    void                       add( WeakPropertyRef* ref ) {
      if ( ref ) m_weakReferences.insert( ref );
    }
    void remove( WeakPropertyRef* ref ) { m_weakReferences.erase( ref ); }
  };

  /// Optional reference to a property that can be used to refer to a sting or
  /// to the string representation of a property instance value
  class GAUDI_API WeakPropertyRef {
    friend PropertyBase;

  public:
    WeakPropertyRef() = default;
    WeakPropertyRef( std::string value ) : m_value{ std::move( value ) }, m_unset{ false } {}
    WeakPropertyRef( PropertyBase& property ) : m_property{ &property } { property.add( this ); }
    WeakPropertyRef( const WeakPropertyRef& other ) = delete;
    WeakPropertyRef( WeakPropertyRef&& other )
        : m_property{ other.m_property }, m_value{ std::move( other.m_value ) }, m_unset{ other.m_unset } {
      if ( m_property ) {
        other.m_property = nullptr;
        m_property->remove( &other );
        m_property->add( this );
      }
    }
    ~WeakPropertyRef() {
      if ( m_property ) m_property->remove( this );
    }
    WeakPropertyRef& operator=( WeakPropertyRef&& other ) {
      if ( this != &other ) {
        if ( m_property ) m_property->remove( this );
        m_property       = other.m_property;
        other.m_property = nullptr;
        if ( m_property ) {
          m_property->remove( &other );
          m_property->add( this );
        }
        m_value = std::move( other.m_value );
        m_unset = other.m_unset;
      }
      return *this;
    }
    WeakPropertyRef& operator=( PropertyBase& value ) {
      if ( m_property != &value ) {
        if ( m_property ) {
          m_property->remove( this );
          if ( !m_unset ) m_value = m_property->toString();
        }
        if ( !m_unset ) value.fromString( m_value ).ignore();
        m_property = &value;
        value.add( this );
      }
      return *this;
    }
    WeakPropertyRef& operator=( const std::string& value ) {
      if ( m_property ) m_property->fromString( value ).ignore();
      m_value = value;
      m_unset = false;
      return *this;
    }
    operator std::string() const;

    inline bool isBound() const { return m_property; }
    inline bool isSet() const { return !m_unset; }

  private:
    PropertyBase* m_property = nullptr;
    std::string   m_value;
    bool          m_unset = true;

    void detach() { m_property = nullptr; }
  };

  inline PropertyBase::~PropertyBase() {
    for ( auto ref : m_weakReferences ) { ref->detach(); }
  }

  inline std::ostream& operator<<( std::ostream& stream, const PropertyBase& prop ) {
    return prop.fillStream( stream );
  }
} // namespace Gaudi::Details
