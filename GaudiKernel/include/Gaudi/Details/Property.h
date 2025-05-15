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

#include <Gaudi/Details/PropertyBase.h>
#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Parsers/InputData.h>
#include <Gaudi/PropertyFwd.h>
#include <GaudiKernel/TaggedBool.h>
#include <GaudiKernel/ToStream.h>
#include <string>
#include <utility>

namespace Gaudi::Details::Property {
  using ImmediatelyInvokeHandler = Gaudi::tagged_bool<class ImmediatelyInvokeHandler_tag>;

  // ==========================================================================
  // The following code is going to be a bit unpleasant, but as far as its
  // author can tell, it is as simple as the design constraints and C++'s
  // implementation constraints will allow. If you disagree, please submit
  // a patch which simplifies it. Here is the underlying design rationale:
  //
  // - For any given type T used in a Property, we want to have an
  //   associated StringConverter<T> struct which explains how to convert a
  //   value of that type into a string (toString) and parse that string
  //   back (fromString).
  // - There is a default implementation, called DefaultStringConverter<T>,
  //   which is based on the overloadable parse() and toStream() global
  //   methods of Gaudi. Its exact behaviour varies depending on whether T
  //   is default-constructible or only copy-constructible, which requires a
  //   layer of SFINAE indirection.
  // - Some people want to be able to specialize StringConverter as an
  //   alternative to defining parse/toStream overloads. This interferes
  //   with the SFINAE tricks used by DefaultStringConverter, so we cannot
  //   just call a DefaultStringConverter a StringConverter and must add one
  //   more layer to the StringConverter type hierarchy.

  // This class factors out commonalities between DefaultStringConverters
  template <class TYPE>
  struct DefaultStringConverterImpl {
  public:
    virtual ~DefaultStringConverterImpl() = default;
    std::string toString( const TYPE& v ) {
      using Gaudi::Utils::toString;
      return toString( v );
    }

    // Implementation of fromString depends on whether TYPE is default-
    // constructible (fastest, easiest) or only copy-constructible (still
    // doable as long as the caller can provide a valid value of TYPE)
    virtual TYPE fromString( const TYPE& ref_value, const std::string& s ) = 0;

  protected:
    void fromStringImpl( TYPE& buffer, const std::string& s ) {
      using Gaudi::Parsers::InputData;
      if ( !parse( buffer, InputData{ s } ).isSuccess() ) {
        throw std::invalid_argument( "cannot parse '" + s + "' to " + System::typeinfoName( typeid( TYPE ) ) );
      }
    }
  };

  // Specialization of toString for strings (identity function)
  template <>
  inline std::string DefaultStringConverterImpl<std::string>::toString( const std::string& v ) {
    return v;
  }

  // This class provides a default implementation of StringConverter based
  // on the overloadable parse() and toStream() global Gaudi methods.
  //
  // It leverages the fact that TYPE is default-constructible if it can, and
  // falls back fo a requirement of copy-constructibility if it must. So
  // here is the "default" implementation for copy-constructible types...
  //
  template <typename TYPE>
  struct DefaultStringConverter : DefaultStringConverterImpl<TYPE> {
    TYPE fromString( const TYPE& ref_value, const std::string& s ) final override {
      TYPE buffer = ref_value;
      this->fromStringImpl( buffer, s );
      return buffer;
    }
  };
  // ...and here is the preferred impl for default-constructible types:
  template <class TYPE>
    requires( std::is_default_constructible_v<TYPE> )
  struct DefaultStringConverter<TYPE> : DefaultStringConverterImpl<TYPE> {
    TYPE fromString( const TYPE& /* ref_value */, const std::string& s ) final override {
      TYPE buffer{};
      this->fromStringImpl( buffer, s );
      return buffer;
    }
  };

  // Specializable StringConverter struct with a default implementation
  template <typename TYPE>
  struct StringConverter : DefaultStringConverter<TYPE> {};

  struct NullVerifier {
    template <class TYPE>
    void operator()( const TYPE& ) const {}
  };
  template <class TYPE>
  struct BoundedVerifier {
    void operator()( const TYPE& value ) const {
      using Gaudi::Utils::toString;
      // throw the exception if the limit is defined and value is outside
      if ( ( m_hasLowerBound && ( value < m_lowerBound ) ) || ( m_hasUpperBound && ( m_upperBound < value ) ) )
        throw std::out_of_range( "value " + toString( value ) + " outside range" );
    }

    /// Return if it has a lower bound
    bool hasLower() const { return m_hasLowerBound; }
    /// Return if it has a lower bound
    bool hasUpper() const { return m_hasUpperBound; }
    /// Return the lower bound value
    const TYPE& lower() const { return m_lowerBound; }
    /// Return the upper bound value
    const TYPE& upper() const { return m_upperBound; }

    /// Set lower bound value
    void setLower( const TYPE& value ) {
      m_hasLowerBound = true;
      m_lowerBound    = value;
    }
    /// Set upper bound value
    void setUpper( const TYPE& value ) {
      m_hasUpperBound = true;
      m_upperBound    = value;
    }
    /// Clear lower bound value
    void clearLower() {
      m_hasLowerBound = false;
      m_lowerBound    = TYPE();
    }
    /// Clear upper bound value
    void clearUpper() {
      m_hasUpperBound = false;
      m_upperBound    = TYPE();
    }

    /// Set both bounds (lower and upper) at the same time
    void setBounds( const TYPE& lower, const TYPE& upper ) {
      setLower( lower );
      setUpper( upper );
    }

    /// Clear both bounds (lower and upper) at the same time
    void clearBounds() {
      clearLower();
      clearUpper();
    }

  private:
    /// Data members
    bool m_hasLowerBound{ false };
    bool m_hasUpperBound{ false };
    TYPE m_lowerBound{};
    TYPE m_upperBound{};
  };

  /// helper to disable a while triggering it, to avoid infinite recursion
  struct SwapCall {
    using callback_t = std::function<void( PropertyBase& )>;
    callback_t tmp, &orig;
    SwapCall( callback_t& input ) : orig( input ) { tmp.swap( orig ); }
    ~SwapCall() { orig.swap( tmp ); }
    void operator()( PropertyBase& p ) const { tmp( p ); }
  };

  struct NoHandler {
    void useReadHandler( const PropertyBase& ) const {}
    void setReadHandler( std::function<void( PropertyBase& )> ) {
      throw std::logic_error( "setReadHandler not implemented for this class" );
    }
    std::function<void( PropertyBase& )> getReadHandler() const { return nullptr; }
    void                                 useUpdateHandler( const PropertyBase& ) const {}
    void                                 setUpdateHandler( std::function<void( PropertyBase& )> ) {
      throw std::logic_error( "setUpdateHandler not implemented for this class" );
    }
    std::function<void( PropertyBase& )> getUpdateHandler() const { return nullptr; }
  };
  struct ReadHandler : NoHandler {
    mutable std::function<void( PropertyBase& )> m_readCallBack;
    void                                         useReadHandler( const PropertyBase& p ) const {
      if ( m_readCallBack ) { SwapCall{ m_readCallBack }( const_cast<PropertyBase&>( p ) ); }
    }
    void setReadHandler( std::function<void( PropertyBase& )> fun ) { m_readCallBack = std::move( fun ); }
    std::function<void( PropertyBase& )> getReadHandler() const { return m_readCallBack; }
  };
  struct UpdateHandler : NoHandler {
    std::function<void( PropertyBase& )> m_updateCallBack;
    void                                 useUpdateHandler( PropertyBase& p ) {
      if ( m_updateCallBack ) {
        try {
          SwapCall{ m_updateCallBack }( p );
        } catch ( const std::exception& x ) {
          throw std::invalid_argument( "failure in update handler of '" + p.name() + "': " + x.what() );
        }
      }
    }
    void setUpdateHandler( std::function<void( PropertyBase& )> fun ) { m_updateCallBack = std::move( fun ); }
    std::function<void( PropertyBase& )> getUpdateHandler() const { return m_updateCallBack; }
  };
  struct ReadUpdateHandler : ReadHandler, UpdateHandler {
    using ReadHandler::getReadHandler;
    using ReadHandler::setReadHandler;
    using ReadHandler::useReadHandler;
    using UpdateHandler::getUpdateHandler;
    using UpdateHandler::setUpdateHandler;
    using UpdateHandler::useUpdateHandler;
  };

  enum class ParsingErrorPolicy { Ignore, Warning, Exception, Abort };
  ParsingErrorPolicy parsingErrorPolicy();
  ParsingErrorPolicy setParsingErrorPolicy( ParsingErrorPolicy p );
} // namespace Gaudi::Details::Property
