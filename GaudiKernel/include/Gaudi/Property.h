/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Details/Property.h>
#include <Gaudi/Details/PropertyBase.h>
#include <Gaudi/PropertyFwd.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/TaggedBool.h>
#include <GaudiKernel/ToStream.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>

namespace Gaudi {
  // ============================================================================
  /** Implementation of property with value of concrete type.
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2006-02-27
   *  @author Marco Clemencic
   *  @date 2016-06-16
   */
  // ============================================================================
  template <class TYPE, class VERIFIER = Details::Property::NullVerifier,
            class HANDLERS = Details::Property::UpdateHandler>
  class Property : public Details::PropertyBase {
  public:
    // ==========================================================================
    /// Hosted type
    using StorageType  = TYPE;
    using ValueType    = typename std::remove_reference<StorageType>::type;
    using VerifierType = VERIFIER;
    using HandlersType = HANDLERS;
    // ==========================================================================

  private:
    /// Storage.
    StorageType  m_value;
    VerifierType m_verifier;
    HandlersType m_handlers;
    /// helper typedefs for SFINAE
    /// @{
    template <class T>
    static inline constexpr bool is_this_type_v = std::is_same_v<Property, std::remove_reference_t<T>>;
    template <class T>
    using not_copying = std::enable_if_t<!is_this_type_v<T>>;
    /// @}
  public:
    // ==========================================================================
    /// the constructor with property name, value and documentation.
    template <class T = StorageType>
    Property( std::string name, T&& value, std::string doc = "", std::string semantics = "" )
        : Details::PropertyBase( typeid( ValueType ), std::move( name ), std::move( doc ), std::move( semantics ) )
        , m_value( std::forward<T>( value ) ) {
      m_verifier( m_value );
    }
    /// Autodeclaring constructor with property name, value and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <typename OWNER, typename T = ValueType, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>,
              typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    Property( OWNER* owner, std::string name ) : Property( std::move( name ), ValueType{}, "" ) {
      owner->declareProperty( *this );
      setOwnerType<OWNER>();
    }

    /// Autodeclaring constructor with property name, value and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
    Property( OWNER* owner, std::string name, T&& value, std::string doc = "", std::string semantics = "" )
        : Property( std::move( name ), std::forward<T>( value ), std::move( doc ), std::move( semantics ) ) {
      owner->declareProperty( *this );
      setOwnerType<OWNER>();
    }

    /// Autodeclaring constructor with property name, value, updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
    Property( OWNER* owner, std::string name, T&& value, std::function<void( PropertyBase& )> handler,
              std::string doc = "", std::string semantics = "" )
        : Property( owner, std::move( name ), std::forward<T>( value ), std::move( doc ), std::move( semantics ) ) {
      declareUpdateHandler( std::move( handler ) );
    }

    /// Autodeclaring constructor with property name, value, pointer to member function updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
    Property( OWNER* owner, std::string name, T&& value, void ( OWNER::*handler )( PropertyBase& ),
              std::string doc = "", std::string semantics = "" )
        : Property(
              owner, std::move( name ), std::forward<T>( value ),
              [owner, handler]( PropertyBase& p ) { ( owner->*handler )( p ); }, std::move( doc ),
              std::move( semantics ) ) {}
    /// Autodeclaring constructor with property name, value, pointer to member function updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
    Property( OWNER* owner, std::string name, T&& value, void ( OWNER::*handler )(), std::string doc = "",
              std::string semantics = "" )
        : Property(
              owner, std::move( name ), std::forward<T>( value ),
              [owner, handler]( PropertyBase& ) { ( owner->*handler )(); }, std::move( doc ), std::move( semantics ) ) {
    }

    /// Autodeclaring constructor with property name, value, updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
    Property( OWNER* owner, std::string name, T&& value, std::function<void( PropertyBase& )> handler,
              Details::Property::ImmediatelyInvokeHandler invoke, std::string doc = "", std::string semantics = "" )
        : Property( owner, std::move( name ), std::forward<T>( value ), std::move( handler ), std::move( doc ),
                    std::move( semantics ) ) {
      if ( invoke ) useUpdateHandler();
    }

    /// Construct an anonymous property from a value.
    /// This constructor is not generated if T is the current type, so that the
    /// compiler picks up the copy constructor instead of this one.
    template <typename T, typename = not_copying<T>>
    Property( T&& v ) : Details::PropertyBase( typeid( ValueType ), "", "", "" ), m_value( std::forward<T>( v ) ) {}

    /// Construct an anonymous property with default constructed value.
    /// Can be used only if StorageType is default constructible.
    template <typename T = StorageType, typename = std::enable_if_t<!std::is_reference_v<T>>>
    Property() : Details::PropertyBase( typeid( ValueType ), "", "", "" ), m_value() {}

    using Details::PropertyBase::declareReadHandler;
    using Details::PropertyBase::declareUpdateHandler;

    /// set new callback for reading
    Details::PropertyBase& declareReadHandler( std::function<void( Details::PropertyBase& )> fun ) override {
      m_handlers.setReadHandler( std::move( fun ) );
      return *this;
    }
    /// set new callback for update
    Details::PropertyBase& declareUpdateHandler( std::function<void( Details::PropertyBase& )> fun ) override {
      m_handlers.setUpdateHandler( std::move( fun ) );
      return *this;
    }

    /// get a reference to the readCallBack
    const std::function<void( Details::PropertyBase& )> readCallBack() const override {
      return m_handlers.getReadHandler();
    }
    /// get a reference to the updateCallBack
    const std::function<void( Details::PropertyBase& )> updateCallBack() const override {
      return m_handlers.getUpdateHandler();
    }

    /// manual trigger for callback for update
    bool useUpdateHandler() override {
      m_handlers.useUpdateHandler( *this );
      return true;
    }

    /// Automatic conversion to value (const reference).
    operator const ValueType&() const {
      m_handlers.useReadHandler( *this );
      return m_value;
    }
    // /// Automatic conversion to value (reference).
    // operator ValueType& () {
    //   useReadHandler();
    //   return m_value;
    // }

    template <typename Dummy = TYPE, typename = std::enable_if_t<std::is_constructible_v<std::string_view, Dummy>>>
    operator std::string_view() const {
      m_handlers.useReadHandler( *this );
      return m_value;
    }

    /// Properly quote string properties when printing them
    std::ostream& fillStream( std::ostream& stream ) const override {
      stream << " '" << name() << "':";
      if constexpr ( std::is_same_v<ValueType, std::string> ) {
        using Gaudi::Utils::toStream;
        toStream( value(), stream );
      } else {
        stream << toString();
      }
      return stream;
    }

    operator std::string_view() const {
      m_handlers.useReadHandler( *this );
      return m_value;
    }

    /// equality comparison
    template <class T>
    bool operator==( const T& other ) const {
      return m_value == other;
    }

    /// inequality comparison
    template <class T>
    bool operator!=( const T& other ) const {
      return m_value != other;
    }

    /// "less" comparison
    template <class T>
    bool operator<( const T& other ) const {
      return m_value < other;
    }

    /// allow addition if possible between the property and the other types
    template <class T>
    decltype( auto ) operator+( const T& other ) const {
      return m_value + other;
    }

    /// Assignment from value.
    template <class T = ValueType>
    Property& operator=( T&& v ) {
      m_verifier( v );
      m_value = std::forward<T>( v );
      m_handlers.useUpdateHandler( *this );
      return *this;
    }

    /// Accessor to verifier.
    const VerifierType& verifier() const { return m_verifier; }
    /// Accessor to verifier.
    VerifierType& verifier() { return m_verifier; }

    /// Backward compatibility (\deprecated will be removed)
    /// @{
    const ValueType& value() const { return *this; }
    ValueType&       value() { return const_cast<ValueType&>( static_cast<const ValueType&>( *this ) ); }
    bool             setValue( const ValueType& v ) {
      *this = v;
      return true;
    }
    bool set( const ValueType& v ) {
      *this = v;
      return true;
    }
    Details::PropertyBase* clone() const override { return new Property( *this ); }
    /// @}

    /// @name Helpers for easy use of string and vector properties.
    /// @{
    /// They are instantiated only if they are implemented in the wrapped class.
    template <class T = const ValueType>
    decltype( auto ) size() const {
      return value().size();
    }
    template <class T = const ValueType>
    decltype( auto ) length() const {
      return value().length();
    }
    template <class T = const ValueType>
    decltype( auto ) empty() const {
      return value().empty();
    }
    template <class T = ValueType>
    decltype( auto ) clear() {
      value().clear();
    }
    template <class T = const ValueType>
    decltype( auto ) begin() const {
      return value().begin();
    }
    template <class T = const ValueType>
    decltype( auto ) end() const {
      return value().end();
    }
    template <class T = ValueType>
    decltype( auto ) begin() {
      return value().begin();
    }
    template <class T = ValueType>
    decltype( auto ) end() {
      return value().end();
    }
    template <class ARG>
    decltype( auto ) operator[]( const ARG& arg ) const {
      return value()[arg];
    }
    template <class ARG>
    decltype( auto ) operator[]( const ARG& arg ) {
      return value()[arg];
    }
    template <class T = const ValueType>
    decltype( auto ) find( const typename T::key_type& key ) const {
      return value().find( key );
    }
    template <class T = ValueType>
    decltype( auto ) find( const typename T::key_type& key ) {
      return value().find( key );
    }
    template <class ARG, class T = ValueType>
    decltype( auto ) erase( ARG arg ) {
      return value().erase( arg );
    }
    template <class = ValueType>
    Property& operator++() {
      ++value();
      return *this;
    }
    template <class = ValueType>
    ValueType operator++( int ) {
      return m_value++;
    }
    template <class = ValueType>
    Property& operator--() {
      --value();
      return *this;
    }
    template <class = ValueType>
    ValueType operator--( int ) {
      return m_value--;
    }
    template <class T = ValueType>
    Property& operator+=( const T& other ) {
      m_value += other;
      return *this;
    }
    template <class T = ValueType>
    Property& operator-=( const T& other ) {
      m_value -= other;
      return *this;
    }
    /// Helpers for DataHandles and derived classes
    template <class T = const ValueType>
    decltype( auto ) key() const {
      return value().key();
    }
    template <class T = const ValueType>
    decltype( auto ) objKey() const {
      return value().objKey();
    }
    template <class T = const ValueType>
    decltype( auto ) fullKey() const {
      return value().fullKey();
    }
    template <class T = ValueType>
    decltype( auto ) initialize() {
      return value().initialize();
    }
    template <class T = ValueType>
    decltype( auto ) makeHandles() const {
      return value().makeHandles();
    }
    template <class ARG, class T = ValueType>
    decltype( auto ) makeHandles( const ARG& arg ) const {
      return value().makeHandles( arg );
    }
    /// @}
    // ==========================================================================

    // Delegate operator() to the value
    template <class... Args>
    decltype( std::declval<ValueType>()( std::declval<Args&&>()... ) ) operator()( Args&&... args ) const
        noexcept( noexcept( std::declval<ValueType>()( std::declval<Args&&>()... ) ) ) {
      return value()( std::forward<Args>( args )... );
    }

  public:
    /// get the value from another property
    bool assign( const Details::PropertyBase& source ) override {
      // Check if the property is of "the same" type, except for strings
      const Property* p =
          ( std::is_same_v<ValueType, std::string> ) ? nullptr : dynamic_cast<const Property*>( &source );
      if ( p ) {
        *this = p->value();
      } else {
        return this->fromString( source.toString() ).isSuccess();
      }
      return true;
    }
    /// set value to another property
    bool load( Details::PropertyBase& dest ) const override {
      // delegate to the 'opposite' method
      return dest.assign( *this );
    }
    /// string -> value
    StatusCode fromString( const std::string& source ) override {
      try {
        using Converter = Details::Property::StringConverter<ValueType>;
        *this           = Converter().fromString( m_value, source );
        return StatusCode::SUCCESS;
      } catch ( const std::exception& err ) {
        using Details::Property::parsingErrorPolicy;
        using Details::Property::ParsingErrorPolicy;
        const std::string errMsg =
            "Cannot convert '" + source + "' for property '" + name() + "' in class '" + ownerTypeName() + "'";
        switch ( parsingErrorPolicy() ) {
        case ParsingErrorPolicy::Ignore:
          break;
        case ParsingErrorPolicy::Exception:
          throw GaudiException( errMsg, "Property::fromString", StatusCode::FAILURE, err );
          break;
        case ParsingErrorPolicy::Warning:
          std::cerr << "WARNING: " << errMsg << "': " << err.what() << '\n';
          break;
        case ParsingErrorPolicy::Abort:
          std::cerr << "FATAL: " << errMsg << "': " << err.what() << '\n';
          std::abort();
          break;
        }
        return StatusCode::FAILURE;
      }
    }
    /// value  -> string
    std::string toString() const override {
      using Converter = Details::Property::StringConverter<ValueType>;
      return Converter().toString( *this );
    }
    /// value  -> stream
    void toStream( std::ostream& out ) const override {
      m_handlers.useReadHandler( *this );
      using Utils::toStream;
      toStream( m_value, out );
    }
  }; // namespace Gaudi

#if __cpp_impl_three_way_comparison < 201711
  // Don't want this with c++20 --- it'll just call itself.
  // The default c++20 rules will properly use Property::operator==.
  /// delegate (value == property) to property operator==
  template <class T, class TP, class V, class H>
  bool operator==( const T& v, const Property<TP, V, H>& p ) {
    return p == v;
  }
#endif

  /// delegate (value != property) to property operator!=
  template <class T, class TP, class V, class H>
  bool operator!=( const T& v, const Property<TP, V, H>& p ) {
    return p != v;
  }

  /// implemantation of (value + property)
  template <class T, class TP, class V, class H>
  decltype( auto ) operator+( const T& v, const Property<TP, V, H>& p ) {
    return v + p.value();
  }

  template <class TYPE, class HANDLERS = Details::Property::UpdateHandler>
  using CheckedProperty = Property<TYPE, Details::Property::BoundedVerifier<TYPE>, HANDLERS>;

  template <class TYPE>
  using PropertyWithReadHandler =
      Property<TYPE, Details::Property::NullVerifier, Gaudi::Details::Property::ReadUpdateHandler>;

} // namespace Gaudi

template <class TYPE>
using SimpleProperty = Gaudi::Property<TYPE>;

template <class TYPE>
using SimplePropertyRef = Gaudi::Property<TYPE&>;

// Typedef Properties for built-in types
typedef Gaudi::Property<bool>               BooleanProperty;
typedef Gaudi::Property<char>               CharProperty;
typedef Gaudi::Property<signed char>        SignedCharProperty;
typedef Gaudi::Property<unsigned char>      UnsignedCharProperty;
typedef Gaudi::Property<short>              ShortProperty;
typedef Gaudi::Property<unsigned short>     UnsignedShortProperty;
typedef Gaudi::Property<int>                IntegerProperty;
typedef Gaudi::Property<unsigned int>       UnsignedIntegerProperty;
typedef Gaudi::Property<long>               LongProperty;
typedef Gaudi::Property<unsigned long>      UnsignedLongProperty;
typedef Gaudi::Property<long long>          LongLongProperty;
typedef Gaudi::Property<unsigned long long> UnsignedLongLongProperty;
typedef Gaudi::Property<float>              FloatProperty;
typedef Gaudi::Property<double>             DoubleProperty;
typedef Gaudi::Property<long double>        LongDoubleProperty;

typedef Gaudi::Property<std::string> StringProperty;

// Typedef PropertyRefs for built-in types
typedef Gaudi::Property<bool&>               BooleanPropertyRef;
typedef Gaudi::Property<char&>               CharPropertyRef;
typedef Gaudi::Property<signed char&>        SignedCharPropertyRef;
typedef Gaudi::Property<unsigned char&>      UnsignedCharPropertyRef;
typedef Gaudi::Property<short&>              ShortPropertyRef;
typedef Gaudi::Property<unsigned short&>     UnsignedShortPropertyRef;
typedef Gaudi::Property<int&>                IntegerPropertyRef;
typedef Gaudi::Property<unsigned int&>       UnsignedIntegerPropertyRef;
typedef Gaudi::Property<long&>               LongPropertyRef;
typedef Gaudi::Property<unsigned long&>      UnsignedLongPropertyRef;
typedef Gaudi::Property<long long&>          LongLongPropertyRef;
typedef Gaudi::Property<unsigned long long&> UnsignedLongLongPropertyRef;
typedef Gaudi::Property<float&>              FloatPropertyRef;
typedef Gaudi::Property<double&>             DoublePropertyRef;
typedef Gaudi::Property<long double&>        LongDoublePropertyRef;

typedef Gaudi::Property<std::string&> StringPropertyRef;

// Typedef "Arrays" of Properties for built-in types
typedef Gaudi::Property<std::vector<bool>>               BooleanArrayProperty;
typedef Gaudi::Property<std::vector<char>>               CharArrayProperty;
typedef Gaudi::Property<std::vector<signed char>>        SignedCharArrayProperty;
typedef Gaudi::Property<std::vector<unsigned char>>      UnsignedCharArrayProperty;
typedef Gaudi::Property<std::vector<short>>              ShortArrayProperty;
typedef Gaudi::Property<std::vector<unsigned short>>     UnsignedShortArrayProperty;
typedef Gaudi::Property<std::vector<int>>                IntegerArrayProperty;
typedef Gaudi::Property<std::vector<unsigned int>>       UnsignedIntegerArrayProperty;
typedef Gaudi::Property<std::vector<long>>               LongArrayProperty;
typedef Gaudi::Property<std::vector<unsigned long>>      UnsignedLongArrayProperty;
typedef Gaudi::Property<std::vector<long long>>          LongLongArrayProperty;
typedef Gaudi::Property<std::vector<unsigned long long>> UnsignedLongLongArrayProperty;
typedef Gaudi::Property<std::vector<float>>              FloatArrayProperty;
typedef Gaudi::Property<std::vector<double>>             DoubleArrayProperty;
typedef Gaudi::Property<std::vector<long double>>        LongDoubleArrayProperty;

typedef Gaudi::Property<std::vector<std::string>> StringArrayProperty;

// Typedef "Arrays" of PropertyRefs for built-in types
typedef Gaudi::Property<std::vector<bool>&>               BooleanArrayPropertyRef;
typedef Gaudi::Property<std::vector<char>&>               CharArrayPropertyRef;
typedef Gaudi::Property<std::vector<signed char>&>        SignedCharArrayPropertyRef;
typedef Gaudi::Property<std::vector<unsigned char>&>      UnsignedCharArrayPropertyRef;
typedef Gaudi::Property<std::vector<short>&>              ShortArrayPropertyRef;
typedef Gaudi::Property<std::vector<unsigned short>&>     UnsignedShortArrayPropertyRef;
typedef Gaudi::Property<std::vector<int>&>                IntegerArrayPropertyRef;
typedef Gaudi::Property<std::vector<unsigned int>&>       UnsignedIntegerArrayPropertyRef;
typedef Gaudi::Property<std::vector<long>&>               LongArrayPropertyRef;
typedef Gaudi::Property<std::vector<unsigned long>&>      UnsignedLongArrayPropertyRef;
typedef Gaudi::Property<std::vector<long long>&>          LongLongArrayPropertyRef;
typedef Gaudi::Property<std::vector<unsigned long long>&> UnsignedLongLongArrayPropertyRef;
typedef Gaudi::Property<std::vector<float>&>              FloatArrayPropertyRef;
typedef Gaudi::Property<std::vector<double>&>             DoubleArrayPropertyRef;
typedef Gaudi::Property<std::vector<long double>&>        LongDoubleArrayPropertyRef;

typedef Gaudi::Property<std::vector<std::string>&> StringArrayPropertyRef;

/// Helper class to simplify the migration old properties deriving directly from
/// PropertyBase.
template <typename Handler = typename Gaudi::Details::Property::UpdateHandler>
class PropertyWithHandlers : public Gaudi::Details::PropertyBase {
  Handler m_handlers;

public:
  using PropertyBase::PropertyBase;

  /// set new callback for reading
  PropertyBase& declareReadHandler( std::function<void( PropertyBase& )> fun ) override {
    m_handlers.setReadHandler( std::move( fun ) );
    return *this;
  }
  /// set new callback for update
  PropertyBase& declareUpdateHandler( std::function<void( PropertyBase& )> fun ) override {
    m_handlers.setUpdateHandler( std::move( fun ) );
    return *this;
  }

  /// get a reference to the readCallBack
  const std::function<void( PropertyBase& )> readCallBack() const override { return m_handlers.getReadHandler(); }
  /// get a reference to the updateCallBack
  const std::function<void( PropertyBase& )> updateCallBack() const override { return m_handlers.getUpdateHandler(); }

  /// use the call-back function at reading, if available
  void useReadHandler() const { m_handlers.useReadHandler( *this ); }

  /// use the call-back function at update, if available
  bool useUpdateHandler() override {
    m_handlers.useUpdateHandler( *this );
    return true;
  }
};

// forward-declaration is sufficient here
class GaudiHandleBase;

// implementation in header file only where the GaudiHandleBase class
// definition is not needed. The rest goes into the .cpp file.
// The goal is to decouple the header files, to avoid that the whole
// world depends on GaudiHandle.h
class GAUDI_API GaudiHandleProperty : public PropertyWithHandlers<> {
public:
  GaudiHandleProperty( std::string name, GaudiHandleBase& ref );

  GaudiHandleProperty& operator=( const GaudiHandleBase& value ) {
    setValue( value );
    return *this;
  }

  GaudiHandleProperty* clone() const override { return new GaudiHandleProperty( *this ); }

  bool load( PropertyBase& destination ) const override { return destination.assign( *this ); }

  bool assign( const PropertyBase& source ) override { return fromString( source.toString() ).isSuccess(); }

  std::string toString() const override;

  void toStream( std::ostream& out ) const override;

  StatusCode fromString( const std::string& s ) override;

  const GaudiHandleBase& value() const {
    useReadHandler();
    return *m_pValue;
  }

  bool setValue( const GaudiHandleBase& value );

private:
  /** Pointer to the real property. Reference would be better, but ROOT does not
      support references yet */
  GaudiHandleBase* m_pValue;
};

// forward-declaration is sufficient here
class GaudiHandleArrayBase;

class GAUDI_API GaudiHandleArrayProperty : public PropertyWithHandlers<> {
public:
  GaudiHandleArrayProperty( std::string name, GaudiHandleArrayBase& ref );

  GaudiHandleArrayProperty& operator=( const GaudiHandleArrayBase& value ) {
    setValue( value );
    return *this;
  }

  GaudiHandleArrayProperty* clone() const override { return new GaudiHandleArrayProperty( *this ); }

  bool load( PropertyBase& destination ) const override { return destination.assign( *this ); }

  bool assign( const PropertyBase& source ) override { return fromString( source.toString() ).isSuccess(); }

  std::string toString() const override;

  void toStream( std::ostream& out ) const override;

  StatusCode fromString( const std::string& s ) override;

  const GaudiHandleArrayBase& value() const {
    useReadHandler();
    return *m_pValue;
  }

  bool setValue( const GaudiHandleArrayBase& value );

private:
  /** Pointer to the real property. Reference would be better, but ROOT does not
      support references yet */
  GaudiHandleArrayBase* m_pValue;
};

namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** simple function which check the existence of the property with
     *  the given name.
     *
     *  @code
     *
     *  const IProperty* p = ... ;
     *
     *  const bool = hasProperty( p , "Context" ) ;
     *
     *  @endcode
     *
     *  @param  p    pointer to IProperty object
     *  @param  name property name (case insensitive)
     *  @return true if "p" has a property with such name
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API bool hasProperty( const IProperty* p, std::string_view name );
    // ========================================================================
    /** simple function which check the existence of the property with
     *  the given name.
     *
     *  @code
     *
     *  IInterface* p = .
     *
     *  const bool = hasProperty( p , "Context" ) ;
     *
     *  @endcode
     *
     *  @param  p    pointer to IInterface   object (any component)
     *  @param  name property name (case insensitive)
     *  @return true if "p" has a property with such name
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API bool hasProperty( const IInterface* p, std::string_view name );
    // ========================================================================
    /** simple function which gets the property with given name
     *  from the component
     *
     *  @code
     *
     *  const IProperty* p = ... ;
     *
     *  const Gaudi::Details::PropertyBase* pro = getProperty( p , "Context" ) ;
     *
     *  @endcode
     *
     *  @param  p    pointer to IProperty object
     *  @param  name property name (case insensitive)
     *  @return property with the given name (if exists), NULL otherwise
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API Gaudi::Details::PropertyBase* getProperty( const IProperty* p, std::string_view name );
    // ========================================================================
    /** simple function which gets the property with given name
     *  from the component
     *
     *  @code
     *
     *  const IInterface* p = ... ;
     *
     *  const Gaudi::Details::PropertyBase* pro = getProperty( p , "Context" ) ;
     *
     *  @endcode
     *
     *  @param  p    pointer to IInterface object
     *  @param  name property name (case insensitive)
     *  @return property with the given name (if exists), NULL otherwise
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API Gaudi::Details::PropertyBase* getProperty( const IInterface* p, std::string_view name );
    // ========================================================================
    /** check  the property by name from  the list of the properties
     *
     *  @code
     *
     *   IJobOptionsSvc* svc = ... ;
     *
     *   const std::string client = ... ;
     *
     *  // get the property:
     *  bool context =
     *      hasProperty ( svc->getProperties( client ) , "Context" )
     *
     *  @endcode
     *
     *  @see IJobOptionsSvc
     *
     *  @param  p    list of properties
     *  @param  name property name (case insensitive)
     *  @return true if the property exists
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API bool hasProperty( const std::vector<const Gaudi::Details::PropertyBase*>* p, std::string_view name );
    // ========================================================================
    /** get the property by name from  the list of the properties
     *
     *  @code
     *
     *   IJobOptionsSvc* svc = ... ;
     *
     *   const std::string client = ... ;
     *
     *  // get the property:
     *  const Gaudi::Details::PropertyBase* context =
     *      getProperty ( svc->getProperties( client ) , "Context" )
     *
     *  @endcode
     *
     *  @see IJobOptionsSvc
     *
     *  @param  p    list of properties
     *  @param  name property name (case insensitive)
     *  @return property with the given name (if exists), NULL otherwise
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API const Gaudi::Details::PropertyBase*
                    getProperty( const std::vector<const Gaudi::Details::PropertyBase*>* p, std::string_view name );
    // ========================================================================
    /** simple function to set the property of the given object from the value
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  std::vector<double> data = ... ;
     *  StatusCode sc = setProperty ( componet , "Data" ,  data ) ;
     *
     *  @endcode
     *
     *  Note: the interface IProperty allows setting of the properties either
     *        directly from other properties or from strings only
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param value     value of the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    template <class TYPE>
    StatusCode setProperty( IProperty* component, const std::string& name, const TYPE& value, const std::string& doc );
    // ========================================================================
    /** simple function to set the property of the given object from the value
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  std::vector<double> data = ... ;
     *  StatusCode sc = setProperty ( componet , "Data" ,  data ) ;
     *
     *  @endcode
     *
     *  Note: the interface IProperty allows setting of the properties either
     *        directly from other properties or from strings only
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param value     value of the property
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    template <class TYPE>
    StatusCode setProperty( IProperty* component, const std::string& name, const TYPE& value ) {
      return setProperty( component, name, value, std::string() );
    }
    // ========================================================================
    /** the full specialization of the
     *  previous method setProperty( IProperty, std::string, const TYPE&)
     *  for standard strings
     *
     *  @param component component which needs to be configured
     *  @param name      name of the property
     *  @param value     value of the property
     *  @param doc       the new documentation string
     *
     *  @see IProperty
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IProperty* component, const std::string& name, const std::string& value,
                                      const std::string& doc = "" );
    // ========================================================================
    /** the full specialization of the
     *  method setProperty( IProperty, std::string, const TYPE&)
     *  for C-strings
     *
     *  @param component component which needs to be configured
     *  @param name      name of the property
     *  @param value     value of the property
     *  @param doc       the new documentation string
     *
     *  @see IProperty
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IProperty* component, const std::string& name, const char* value,
                                      const std::string& doc = "" );
    // ========================================================================
    /** the full specialization of the
     *  method setProperty( IProperty, std::string, const TYPE&)
     *  for C-arrays
     *
     *  @param component component which needs to be configured
     *  @param name      name of the property
     *  @param value     value of the property
     *  @param doc       the new documentation string
     *
     *  @see IProperty
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-05-13
     */
    template <unsigned N>
    StatusCode setProperty( IProperty* component, const std::string& name, const char ( &value )[N],
                            const std::string& doc = "" ) {
      return component ? setProperty( component, name, std::string( value, value + N ), doc ) : StatusCode::FAILURE;
    }
    // ========================================================================
    /** simple function to set the property of the given object from the value
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  std::vector<double> data = ... ;
     *  StatusCode sc = setProperty ( component , "Data" ,  data ) ;
     *
     *  std::map<std::string,double> cuts = ... ;
     *  sc = setProperty ( component , "Cuts" , cuts ) ;
     *
     *  std::map<std::string,std::string> dict = ... ;
     *  sc = setProperty ( component , "Dictionary" , dict ) ;
     *
     *  @endcode
     *
     *  Note: the native interface IProperty allows setting of the
     *        properties either directly from other properties or
     *        from strings only
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param value     value of the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    template <class TYPE>
    StatusCode setProperty( IProperty* component, const std::string& name, const TYPE& value, const std::string& doc ) {
      using Gaudi::Utils::toString;
      return component && hasProperty( component, name )
                 ? Gaudi::Utils::setProperty( component, name, toString( value ), doc )
                 : StatusCode::FAILURE;
    }
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  const Gaudi::Details::PropertyBase* prop = ... ;
     *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param property  the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IProperty* component, const std::string& name,
                                      const Gaudi::Details::PropertyBase* property, const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  const Gaudi::Details::PropertyBase& prop = ... ;
     *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param property  the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IProperty* component, const std::string& name,
                                      const Gaudi::Details::PropertyBase& property, const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  Gaudi::Property<std::vector<int> > m_data = ... ;
     *
     *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param value     the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    template <class TYPE>
    StatusCode setProperty( IProperty* component, const std::string& name, const Gaudi::Property<TYPE>& value,
                            const std::string& doc = "" ) {
      return setProperty( component, name, &value, doc );
    }
    // ========================================================================
    /** simple function to set the property of the given object from the value
     *
     *  @code
     *
     *  IInterface* component = ... ;
     *
     *  std::vector<double> data = ... ;
     *  StatusCode sc = setProperty ( component , "Data" ,  data ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param value     value of the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    template <class TYPE>
    StatusCode setProperty( IInterface* component, const std::string& name, const TYPE& value,
                            const std::string& doc = "" ) {
      if ( !component ) { return StatusCode::FAILURE; }
      auto property = SmartIF<IProperty>{component};
      return property ? setProperty( property, name, value, doc ) : StatusCode::FAILURE;
    }
    // ========================================================================
    /** the full specialization of the
     *  method setProperty( IInterface , std::string, const TYPE&)
     *  for standard strings
     *
     *  @param component component which needs to be configured
     *  @param name      name of the property
     *  @param value     value of the property
     *  @param doc       the new documentation string
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IInterface* component, const std::string& name, const std::string& value,
                                      const std::string& doc = "" );
    // ========================================================================
    /** the full specialization of the
     *  method setProperty( IInterface , std::string, const TYPE&)
     *  for C-strings
     *
     *  @param component component which needs to be configured
     *  @param name      name of the property
     *  @param value     value of the property
     *  @param doc       the new documentation string
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IInterface* component, const std::string& name, const char* value,
                                      const std::string& doc = "" );
    // ========================================================================
    /** the full specialization of the
     *  method setProperty( IInterface, std::string, const TYPE&)
     *  for C-arrays
     *
     *  @param component component which needs to be configured
     *  @param name      name of the property
     *  @param value     value of the property
     *  @param doc       the new documentation string
     *
     *  @see IProperty
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-05-13
     */
    template <unsigned N>
    StatusCode setProperty( IInterface* component, const std::string& name, const char ( &value )[N],
                            const std::string& doc = "" ) {
      if ( 0 == component ) { return StatusCode::FAILURE; }
      return setProperty( component, name, std::string{value, value + N}, doc );
    }
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IInterface* component = ... ;
     *
     *  const Gaudi::Details::PropertyBase* prop = ... ;
     *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param property  the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IInterface* component, const std::string& name,
                                      const Gaudi::Details::PropertyBase* property, const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IInterface* component = ... ;
     *
     *  const Gaudi::Details::PropertyBase& prop = ... ;
     *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param property  the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    GAUDI_API StatusCode setProperty( IInterface* component, const std::string& name,
                                      const Gaudi::Details::PropertyBase& property, const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IInterface* component = ... ;
     *
     *  Gaudi::Property<std::vector<int> > m_data = ... ;
     *
     *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
     *
     *  @endcode
     *
     * @param component component which needs to be configured
     * @param name      name of the property
     * @param value     the property
     * @param doc       the new documentation string
     *
     * @see IProperty
     * @author Vanya BELYAEV ibelyaev@physics.syr.edu
     * @date 2007-05-13
     */
    template <class TYPE>
    StatusCode setProperty( IInterface* component, const std::string& name, const Gaudi::Property<TYPE>& value,
                            const std::string& doc = "" ) {
      return setProperty( component, name, &value, doc );
    }
    // ========================================================================
  } // namespace Utils
} // end of namespace Gaudi
