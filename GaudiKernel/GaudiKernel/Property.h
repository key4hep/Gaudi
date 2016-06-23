#ifndef GAUDIKERNEL_PROPERTY_H
#define GAUDIKERNEL_PROPERTY_H
// ============================================================================
// STD & STL
// ============================================================================
#include <stdexcept>
#include <string>
#include <typeinfo>
// ============================================================================
// Application C++ Class Headers
// ============================================================================
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ToStream.h"
// ============================================================================
/// The output operator for friendly printout
// ============================================================================
class Property;
GAUDI_API std::ostream& operator<<( std::ostream& stream, const Property& prop );
// ============================================================================
/** Property base class allowing Property* collections to be "homogeneous"
 *
 * \author Paul Maley
 * \author CTDay
 * \author Vanya BELYAEV ibelyaev@physics.syr.edu
 * \author Marco Clemencic
 */
class GAUDI_API Property
{
private:
  // the default constructor is disabled
  Property() = delete;

public:
  /// property name
  const std::string& name() const { return m_name; }
  /// property documentation
  const std::string& documentation() const { return m_documentation; }
  /// property type-info
  const std::type_info* type_info() const { return m_typeinfo; }
  /// property type
  std::string type() const { return m_typeinfo->name(); }
  ///  export the property value to the destination
  virtual bool load( Property& dest ) const = 0;
  /// import the property value form the source
  virtual bool assign( const Property& source ) = 0;

public:
  /// value  -> string
  virtual std::string toString() const = 0;
  /// value  -> stream
  virtual void toStream( std::ostream& out ) const = 0;
  /// string -> value
  virtual StatusCode fromString( const std::string& value ) = 0;

public:
  /// get a reference to the readCallBack
  const std::function<void( Property& )>& readCallBack() const { return m_readCallBack; }
  /// get a reference to the updateCallBack
  const std::function<void( Property& )>& updateCallBack() const { return m_updateCallBack; }

  /// set new callback for reading
  virtual Property& declareReadHandler( std::function<void( Property& )> fun );
  /// set new callback for update
  virtual Property& declareUpdateHandler( std::function<void( Property& )> fun );

  template <class HT>
  inline Property& declareReadHandler( void ( HT::*MF )( Property& ), HT* instance )
  {
    return declareReadHandler( [=]( Property& p ) { ( instance->*MF )( p ); } );
  }

  template <class HT>
  inline Property& declareUpdateHandler( void ( HT::*MF )( Property& ), HT* instance )
  {
    return declareUpdateHandler( [=]( Property& p ) { ( instance->*MF )( p ); } );
  }

  /// use the call-back function at reading
  virtual void useReadHandler() const;
  /// use the call-back function at update
  virtual bool useUpdateHandler();

public:
  /// virtual destructor
  virtual ~Property() = default;
  /// clone: "virtual constructor"
  // virtual Property*          clone     () const = 0 ;
  /// set the new value for the property name
  void setName( std::string value ) { m_name = std::move( value ); }
  /// set the documentation string
  void setDocumentation( std::string documentation ) { m_documentation = std::move( documentation ); }
  /// the printout of the property value
  virtual std::ostream& fillStream( std::ostream& ) const;

protected:
  /// constructor from the property name and the type
  Property( const std::type_info& type, std::string name = "", std::string doc = "" )
      : m_name( std::move( name ) ), m_documentation( std::move( doc ) ), m_typeinfo( &type )
  {
  }
  /// constructor from the property name and the type
  Property( std::string name, const std::type_info& type );
  /// copy constructor
  Property( const Property& ) = default;
  /// assignment operator
  Property& operator=( const Property& ) = default;

private:
  /// property name
  std::string m_name;
  /// property doc string
  std::string m_documentation;
  /// property type
  const std::type_info* m_typeinfo;

protected:
  // call back functor for reading
  mutable std::function<void( Property& )> m_readCallBack;
  // call back functor for update
  std::function<void( Property& )> m_updateCallBack;
};

namespace Gaudi
{
  namespace Details
  {
    namespace Property
    {
      template <class TYPE>
      struct StringConverter {
        inline std::string toString( const TYPE& v ) { return Gaudi::Utils::toString( v ); }
        inline TYPE fromString( const std::string& s )
        {
          TYPE tmp;
          if ( Gaudi::Parsers::parse( tmp, s ).isSuccess() ) {
            return tmp;
          } else {
            throw std::invalid_argument( "cannot parse '" + s + "' to " + System::typeinfoName( typeid( TYPE ) ) );
          }
        }
      };
      template <>
      inline std::string StringConverter<std::string>::toString( const std::string& v )
      {
        return v;
      }
      template <class TYPE>
      struct NullVerifier {
        void operator()( const TYPE& ) const {}
      };
      template <class TYPE>
      struct BoundedVerifier {
        void operator()( const TYPE& value ) const
        {
          // throw the exception if the limit is defined and value is outside
          if ( ( m_hasLowerBound && ( value < m_lowerBound ) ) || ( m_hasUpperBound && ( m_upperBound < value ) ) )
            throw std::out_of_range( "value " + Gaudi::Utils::toString( value ) + " outside range" );
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
        void setLower( const TYPE& value )
        {
          m_hasLowerBound = true;
          m_lowerBound    = value;
        }
        /// Set upper bound value
        void setUpper( const TYPE& value )
        {
          m_hasUpperBound = true;
          m_upperBound    = value;
        }
        /// Clear lower bound value
        void clearLower()
        {
          m_hasLowerBound = false;
          m_lowerBound    = TYPE();
        }
        /// Clear upper bound value
        void clearUpper()
        {
          m_hasUpperBound = false;
          m_upperBound    = TYPE();
        }

        /// Set both bounds (lower and upper) at the same time
        void setBounds( const TYPE& lower, const TYPE& upper )
        {
          setLower( lower );
          setUpper( upper );
        }

        /// Clear both bounds (lower and upper) at the same time
        void clearBounds()
        {
          clearLower();
          clearUpper();
        }

      private:
        /// Data members
        bool m_hasLowerBound{false};
        bool m_hasUpperBound{false};
        TYPE m_lowerBound{};
        TYPE m_upperBound{};
      };
    }
  }
}

// ============================================================================
/** @class PropertyWithValue
 *  Helper intermediate class which
 *  represent partly implemented property
 *  with value of concrete type
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-02-27
 *  @author Marco Clemencic
 *  @date 2016-06-16
 */
// ============================================================================
template <class TYPE, class VERIFIER = Gaudi::Details::Property::NullVerifier<TYPE>>
class PropertyWithValue : public Property
{
public:
  // ==========================================================================
  /// Hosted type
  using StorageType  = TYPE;
  using ValueType    = typename std::remove_reference<StorageType>::type;
  using VerifierType = VERIFIER;

private:
  /// Storage.
  StorageType m_value;
  VerifierType m_verifier;
  /// helper typedefs for SFINAE
  /// @{
  template <class T>
  using is_this_type = std::is_same<PropertyWithValue, typename std::remove_reference<T>::type>;
  template <class T>
  using not_copying = std::enable_if<!is_this_type<T>::value>;
  /// @}
public:
  // ==========================================================================
  /// the constructor with property name, value and documentation.
  template <class T = ValueType>
  inline PropertyWithValue( std::string name, T&& value, std::string doc = "" )
      : Property( typeid( ValueType ), std::move( name ), std::move( doc ) ), m_value( std::forward<T>( value ) )
  {
    m_verifier( m_value );
  }
  /// Autodeclaring constructor with property name, value and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, class T = ValueType,
            typename = typename std::enable_if<std::is_convertible<OWNER*, IProperty*>::value>::type>
  inline PropertyWithValue( OWNER* owner, std::string name, T&& value = ValueType{}, std::string doc = "" )
      : PropertyWithValue( std::move( name ), std::forward<T>( value ), std::move( doc ) )
  {
    owner->declareProperty( *this );
  }

  /// Construct an anonymous property from a value.
  /// This constructor is not generated if T is the current type, so that the
  /// compiler picks up the copy constructor instead of this one.
  template <class T = ValueType, typename = typename not_copying<T>::type>
  PropertyWithValue( T&& v ) : Property( typeid( ValueType ), "", "" ), m_value( std::forward<T>( v ) )
  {
  }

  /// Construct an anonymous property with default constructed value.
  /// Can be used only if StorageType is default constructible.
  template <typename = void>
  PropertyWithValue() : Property( typeid( ValueType ), "", "" ), m_value()
  {
  }

  /// Automatic conversion to value (const reference).
  operator const ValueType&() const
  {
    useReadHandler();
    return m_value;
  }
  // /// Automatic conversion to value (const reference).
  // operator ValueType& () {
  //   useReadHandler();
  //   return m_value;
  // }

  /// Assignment from value.
  template <class T          = ValueType>
  PropertyWithValue& operator=( T&& v )
  {
    m_verifier( v );
    m_value = std::forward<T>( v );
    if ( !useUpdateHandler() ) throw std::invalid_argument( "failure in update handler of '" + name() + "'" );
    return *this;
  }

  /// Accessor to verifier.
  const VerifierType& verifier() const { return m_verifier; }
  /// Accessor to verifier.
  VerifierType& verifier() { return m_verifier; }

  /// Backward compatibility (deprecated)
  /// @{
  const ValueType& value() const { return *this; }
  ValueType& value()
  {
    useReadHandler();
    return m_value;
  }
  bool setValue( const ValueType& v )
  {
    *this = v;
    return true;
  }
  bool set( const ValueType& v )
  {
    *this = v;
    return true;
  }
  /// @}

  /// @name Helpers for easy use of string and vector properties.
  /// @{
  /// They are instantiated only if they are implemented in the wrapped class.
  template <class T = const ValueType>
  inline decltype( std::declval<T>().size() ) size() const
  {
    return value().size();
  }
  template <class T = const ValueType>
  inline decltype( std::declval<T>().empty() ) empty() const
  {
    return value().empty();
  }
  template <class T = ValueType>
  inline decltype( std::declval<T>().clear() ) clear()
  {
    value().clear();
  }
  template <class T = const ValueType>
  inline decltype( std::declval<T>().begin() ) begin() const
  {
    return value().begin();
  }
  template <class T = const ValueType>
  inline decltype( std::declval<T>().end() ) end() const
  {
    return value().end();
  }
  template <class T = ValueType>
  inline decltype( std::declval<T>().begin() ) begin()
  {
    return value().begin();
  }
  template <class T = ValueType>
  inline decltype( std::declval<T>().end() ) end()
  {
    return value().end();
  }
  template <class T = const ValueType>
  inline decltype( std::declval<T>()[typename T::key_type{}] ) operator[]( const typename T::key_type& key ) const
  {
    return value()[key];
  }
  template <class T = ValueType>
  inline decltype( std::declval<T>()[typename T::key_type{}] ) operator[]( const typename T::key_type& key )
  {
    return value()[key];
  }
  /// @}
  // ==========================================================================
public:
  /// get the value from another property
  bool assign( const Property& source ) override
  {
    // Check if the property of is of "the same" type, except for strings
    const PropertyWithValue* p =
        ( std::is_same<ValueType, std::string>::value ) ? nullptr : dynamic_cast<const PropertyWithValue*>( &source );
    if ( p ) {
      *this = p->value();
    } else {
      this->fromString( source.toString() ).ignore();
    }
    return true;
  }
  /// set value to another property
  bool load( Property& dest ) const override
  {
    // delegate to the 'opposite' method
    return dest.assign( *this );
  }
  /// string -> value
  StatusCode fromString( const std::string& source ) override
  {
    using Converter = Gaudi::Details::Property::StringConverter<ValueType>;
    *this           = Converter().fromString( source );
    return StatusCode::SUCCESS;
  }
  /// value  -> string
  std::string toString() const override
  {
    using Converter = Gaudi::Details::Property::StringConverter<ValueType>;
    return Converter().toString( *this );
  }
  /// value  -> stream
  void toStream( std::ostream& out ) const override
  {
    useReadHandler();
    Gaudi::Utils::toStream( m_value, out );
  }
  // ==========================================================================
  // protected:
  //   // ==========================================================================
  //   /// set the value
  //   inline void  i_set ( const ValueType& value ) {
  //     Traits::assign(*m_value, value);
  //   }
  //   /// get the value
  //   inline PVal  i_get () const {
  //     return m_value;
  //   }
  //   // ==========================================================================
};

template <class TYPE, class VERIFIER = Gaudi::Details::Property::BoundedVerifier<TYPE>>
using SimpleProperty = PropertyWithValue<TYPE, VERIFIER>;

template <class TYPE, class VERIFIER = Gaudi::Details::Property::NullVerifier<TYPE>>
using SimplePropertyRef = PropertyWithValue<TYPE&, VERIFIER>;

// Typedef Properties for built-in types
typedef SimpleProperty<bool> BooleanProperty;
typedef SimpleProperty<char> CharProperty;
typedef SimpleProperty<signed char> SignedCharProperty;
typedef SimpleProperty<unsigned char> UnsignedCharProperty;
typedef SimpleProperty<short> ShortProperty;
typedef SimpleProperty<unsigned short> UnsignedShortProperty;
typedef SimpleProperty<int> IntegerProperty;
typedef SimpleProperty<unsigned int> UnsignedIntegerProperty;
typedef SimpleProperty<long> LongProperty;
typedef SimpleProperty<unsigned long> UnsignedLongProperty;
typedef SimpleProperty<long long> LongLongProperty;
typedef SimpleProperty<unsigned long long> UnsignedLongLongProperty;
typedef SimpleProperty<float> FloatProperty;
typedef SimpleProperty<double> DoubleProperty;
typedef SimpleProperty<long double> LongDoubleProperty;

typedef SimpleProperty<std::string> StringProperty;

// Typedef PropertyRefs for built-in types
typedef SimplePropertyRef<bool> BooleanPropertyRef;
typedef SimplePropertyRef<char> CharPropertyRef;
typedef SimplePropertyRef<signed char> SignedCharPropertyRef;
typedef SimplePropertyRef<unsigned char> UnsignedCharPropertyRef;
typedef SimplePropertyRef<short> ShortPropertyRef;
typedef SimplePropertyRef<unsigned short> UnsignedShortPropertyRef;
typedef SimplePropertyRef<int> IntegerPropertyRef;
typedef SimplePropertyRef<unsigned int> UnsignedIntegerPropertyRef;
typedef SimplePropertyRef<long> LongPropertyRef;
typedef SimplePropertyRef<unsigned long> UnsignedLongPropertyRef;
typedef SimplePropertyRef<long long> LongLongPropertyRef;
typedef SimplePropertyRef<unsigned long long> UnsignedLongLongPropertyRef;
typedef SimplePropertyRef<float> FloatPropertyRef;
typedef SimplePropertyRef<double> DoublePropertyRef;
typedef SimplePropertyRef<long double> LongDoublePropertyRef;

typedef SimplePropertyRef<std::string> StringPropertyRef;

// Typedef "Arrays" of Properties for built-in types
typedef SimpleProperty<std::vector<bool>> BooleanArrayProperty;
typedef SimpleProperty<std::vector<char>> CharArrayProperty;
typedef SimpleProperty<std::vector<signed char>> SignedCharArrayProperty;
typedef SimpleProperty<std::vector<unsigned char>> UnsignedCharArrayProperty;
typedef SimpleProperty<std::vector<short>> ShortArrayProperty;
typedef SimpleProperty<std::vector<unsigned short>> UnsignedShortArrayProperty;
typedef SimpleProperty<std::vector<int>> IntegerArrayProperty;
typedef SimpleProperty<std::vector<unsigned int>> UnsignedIntegerArrayProperty;
typedef SimpleProperty<std::vector<long>> LongArrayProperty;
typedef SimpleProperty<std::vector<unsigned long>> UnsignedLongArrayProperty;
typedef SimpleProperty<std::vector<long long>> LongLongArrayProperty;
typedef SimpleProperty<std::vector<unsigned long long>> UnsignedLongLongArrayProperty;
typedef SimpleProperty<std::vector<float>> FloatArrayProperty;
typedef SimpleProperty<std::vector<double>> DoubleArrayProperty;
typedef SimpleProperty<std::vector<long double>> LongDoubleArrayProperty;

typedef SimpleProperty<std::vector<std::string>> StringArrayProperty;

// Typedef "Arrays" of PropertyRefs for built-in types
typedef SimplePropertyRef<std::vector<bool>> BooleanArrayPropertyRef;
typedef SimplePropertyRef<std::vector<char>> CharArrayPropertyRef;
typedef SimplePropertyRef<std::vector<signed char>> SignedCharArrayPropertyRef;
typedef SimplePropertyRef<std::vector<unsigned char>> UnsignedCharArrayPropertyRef;
typedef SimplePropertyRef<std::vector<short>> ShortArrayPropertyRef;
typedef SimplePropertyRef<std::vector<unsigned short>> UnsignedShortArrayPropertyRef;
typedef SimplePropertyRef<std::vector<int>> IntegerArrayPropertyRef;
typedef SimplePropertyRef<std::vector<unsigned int>> UnsignedIntegerArrayPropertyRef;
typedef SimplePropertyRef<std::vector<long>> LongArrayPropertyRef;
typedef SimplePropertyRef<std::vector<unsigned long>> UnsignedLongArrayPropertyRef;
typedef SimplePropertyRef<std::vector<long long>> LongLongArrayPropertyRef;
typedef SimplePropertyRef<std::vector<unsigned long long>> UnsignedLongLongArrayPropertyRef;
typedef SimplePropertyRef<std::vector<float>> FloatArrayPropertyRef;
typedef SimplePropertyRef<std::vector<double>> DoubleArrayPropertyRef;
typedef SimplePropertyRef<std::vector<long double>> LongDoubleArrayPropertyRef;

typedef SimplePropertyRef<std::vector<std::string>> StringArrayPropertyRef;

// forward-declaration is sufficient here
class GaudiHandleBase;

// implementation in header file only where the GaudiHandleBase class
// definition is not needed. The rest goes into the .cpp file.
// The goal is to decouple the header files, to avoid that the whole
// world depends on GaudiHandle.h
class GAUDI_API GaudiHandleProperty : public Property
{
public:
  GaudiHandleProperty( std::string name, GaudiHandleBase& ref );

  GaudiHandleProperty& operator=( const GaudiHandleBase& value )
  {
    setValue( value );
    return *this;
  }

  // GaudiHandleProperty* clone() const override  {
  //   return new GaudiHandleProperty( *this );
  // }

  bool load( Property& destination ) const override { return destination.assign( *this ); }

  bool assign( const Property& source ) override { return fromString( source.toString() ); }

  std::string toString() const override;

  void toStream( std::ostream& out ) const override;

  StatusCode fromString( const std::string& s ) override;

  const GaudiHandleBase& value() const
  {
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

class GAUDI_API GaudiHandleArrayProperty : public Property
{
public:
  GaudiHandleArrayProperty( std::string name, GaudiHandleArrayBase& ref );

  GaudiHandleArrayProperty& operator=( const GaudiHandleArrayBase& value )
  {
    setValue( value );
    return *this;
  }

  // GaudiHandleArrayProperty* clone() const override {
  //   return new GaudiHandleArrayProperty( *this );
  // }

  bool load( Property& destination ) const override { return destination.assign( *this ); }

  bool assign( const Property& source ) override { return fromString( source.toString() ); }

  std::string toString() const override;

  void toStream( std::ostream& out ) const override;

  StatusCode fromString( const std::string& s ) override;

  const GaudiHandleArrayBase& value() const
  {
    useReadHandler();
    return *m_pValue;
  }

  bool setValue( const GaudiHandleArrayBase& value );

private:
  /** Pointer to the real property. Reference would be better, but ROOT does not
      support references yet */
  GaudiHandleArrayBase* m_pValue;
};

namespace Gaudi
{
  namespace Utils
  {
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
    GAUDI_API bool hasProperty( const IProperty* p, const std::string& name );
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
    GAUDI_API bool hasProperty( const IInterface* p, const std::string& name );
    // ========================================================================
    /** simple function which gets the property with given name
     *  from the component
     *
     *  @code
     *
     *  const IProperty* p = ... ;
     *
     *  const Property* pro = getProperty( p , "Context" ) ;
     *
     *  @endcode
     *
     *  @param  p    pointer to IProperty object
     *  @param  name property name (case insensitive)
     *  @return property with the given name (if exists), NULL otherwise
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API Property* getProperty( const IProperty* p, const std::string& name );
    // ========================================================================
    /** simple function which gets the property with given name
     *  from the component
     *
     *  @code
     *
     *  const IInterface* p = ... ;
     *
     *  const Property* pro = getProperty( p , "Context" ) ;
     *
     *  @endcode
     *
     *  @param  p    pointer to IInterface object
     *  @param  name property name (case insensitive)
     *  @return property with the given name (if exists), NULL otherwise
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-09-09
     */
    GAUDI_API Property* getProperty( const IInterface* p, const std::string& name );
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
    GAUDI_API bool hasProperty( const std::vector<const Property*>* p, const std::string& name );
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
     *  const Property* context =
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
    GAUDI_API const Property* getProperty( const std::vector<const Property*>* p, const std::string& name );
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
    StatusCode setProperty( IProperty* component, const std::string& name, const TYPE& value )
    {
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
                            const std::string& doc = "" )
    {
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
    StatusCode setProperty( IProperty* component, const std::string& name, const TYPE& value, const std::string& doc )
    {
      return component && hasProperty( component, name )
                 ? Gaudi::Utils::setProperty( component, name, Gaudi::Utils::toString( value ), doc )
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
     *  const Property* prop = ... ;
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
    GAUDI_API StatusCode setProperty( IProperty* component, const std::string& name, const Property* property,
                                      const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  const Property& prop = ... ;
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
    GAUDI_API StatusCode setProperty( IProperty* component, const std::string& name, const Property& property,
                                      const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IProperty* component = ... ;
     *
     *  SimpleProperty<std::vector<int> > m_data = ... ;
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
    StatusCode setProperty( IProperty* component, const std::string& name, const SimpleProperty<TYPE>& value,
                            const std::string& doc = "" )
    {
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
                            const std::string& doc = "" )
    {
      if ( !component ) {
        return StatusCode::FAILURE;
      }
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
                            const std::string& doc = "" )
    {
      if ( 0 == component ) {
        return StatusCode::FAILURE;
      }
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
     *  const Property* prop = ... ;
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
    GAUDI_API StatusCode setProperty( IInterface* component, const std::string& name, const Property* property,
                                      const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IInterface* component = ... ;
     *
     *  const Property& prop = ... ;
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
    GAUDI_API StatusCode setProperty( IInterface* component, const std::string& name, const Property& property,
                                      const std::string& doc = "" );
    // ========================================================================
    /** simple function to set the property of the given object from another
     *  property
     *
     *  @code
     *
     *  IInterface* component = ... ;
     *
     *  SimpleProperty<std::vector<int> > m_data = ... ;
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
    StatusCode setProperty( IInterface* component, const std::string& name, const SimpleProperty<TYPE>& value,
                            const std::string& doc = "" )
    {
      return setProperty( component, name, &value, doc );
    }
    // ========================================================================
  } // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_PROPERTY_H
// ============================================================================
