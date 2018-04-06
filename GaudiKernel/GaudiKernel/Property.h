#ifndef GAUDIKERNEL_PROPERTY_H
#define GAUDIKERNEL_PROPERTY_H
// ============================================================================
// STD & STL
// ============================================================================
#include <boost/utility/string_ref.hpp>
#include <stdexcept>
#include <string>
#include <typeinfo>
// ============================================================================
// Application C++ Class Headers
// ============================================================================
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/PropertyFwd.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TaggedBool.h"
#include "GaudiKernel/ToStream.h"

namespace Gaudi
{
  namespace Details
  {
    // ============================================================================
    /** PropertyBase base class allowing PropertyBase* collections to be "homogeneous"
     *
     * \author Paul Maley
     * \author CTDay
     * \author Vanya BELYAEV ibelyaev@physics.syr.edu
     * \author Marco Clemencic
     */
    class GAUDI_API PropertyBase
    {

    public:
      /// property name
      const std::string name() const { return m_name.to_string(); }
      /// property documentation
      std::string documentation() const { return m_documentation.to_string(); }
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
      PropertyBase& declareReadHandler( void ( HT::*MF )( PropertyBase& ), HT* instance )
      {
        return declareReadHandler( [=]( PropertyBase& p ) { ( instance->*MF )( p ); } );
      }

      template <class HT>
      PropertyBase& declareUpdateHandler( void ( HT::*MF )( PropertyBase& ), HT* instance )
      {
        return declareUpdateHandler( [=]( PropertyBase& p ) { ( instance->*MF )( p ); } );
      }

    public:
      /// virtual destructor
      virtual ~PropertyBase() = default;
      /// set the new value for the property name
      void setName( std::string value ) { m_name = to_view( std::move( value ) ); }
      /// set the documentation string
      void setDocumentation( std::string value ) { m_documentation = to_view( std::move( value ) ); }
      /// the printout of the property value
      virtual std::ostream& fillStream( std::ostream& ) const;
      /// clones the current property
      virtual PropertyBase* clone() const = 0;

      /// set the type of the owner class (used for documentation)
      void setOwnerType( const std::type_info& ownerType ) { m_ownerType = &ownerType; }

      /// set the type of the owner class (used for documentation)
      template <class OWNER>
      void setOwnerType()
      {
        setOwnerType( typeid( OWNER ) );
      }

      /// get the type of the owner class (used for documentation)
      const std::type_info* ownerType() const { return m_ownerType; }

      /// get the string for the type of the owner class (used for documentation)
      std::string ownerTypeName() const
      {
        return m_ownerType ? System::typeinfoName( *m_ownerType ) : std::string( "unknown owner type" );
      }

    protected:
      /// constructor from the property name and the type
      PropertyBase( const std::type_info& type, std::string name = "", std::string doc = "" )
          : m_name( to_view( std::move( name ) ) ), m_documentation( to_view( std::move( doc ) ) ), m_typeinfo( &type )
      {
      }
      /// constructor from the property name and the type
      PropertyBase( std::string name, const std::type_info& type )
          : m_name( to_view( std::move( name ) ) ), m_documentation( m_name ), m_typeinfo( &type )
      {
      }
      /// copy constructor
      PropertyBase( const PropertyBase& ) = default;
      /// assignment operator
      PropertyBase& operator=( const PropertyBase& ) = default;

    private:
      /// helper to map a string to a reliable boost::string_ref
      static boost::string_ref to_view( std::string str );
      /// property name
      boost::string_ref m_name;
      /// property doc string
      boost::string_ref m_documentation;
      /// property type
      const std::type_info* m_typeinfo;
      /// type of owner of the property (if defined)
      const std::type_info* m_ownerType = nullptr;
    };

    inline std::ostream& operator<<( std::ostream& stream, const PropertyBase& prop )
    {
      return prop.fillStream( stream );
    }

    namespace Property
    {
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
        std::string toString( const TYPE& v )
        {
          using Gaudi::Utils::toString;
          return toString( v );
        }

        // Implementation of fromString depends on whether TYPE is default-
        // constructible (fastest, easiest) or only copy-constructible (still
        // doable as long as the caller can provide a valid value of TYPE)
        virtual TYPE fromString( const TYPE& ref_value, const std::string& s ) = 0;

      protected:
        void fromStringImpl( TYPE& buffer, const std::string& s )
        {
          using Gaudi::Parsers::parse;
          if ( !parse( buffer, s ).isSuccess() ) {
            throw std::invalid_argument( "cannot parse '" + s + "' to " + System::typeinfoName( typeid( TYPE ) ) );
          }
        }
      };
      // Specialization of toString for strings (identity function)
      template <>
      inline std::string DefaultStringConverterImpl<std::string>::toString( const std::string& v )
      {
        return v;
      }

      // This class provides a default implementation of StringConverter based
      // on the overloadable parse() and toStream() global Gaudi methods.
      //
      // It leverages the fact that TYPE is default-constructible if it can, and
      // falls back fo a requirement of copy-constructibility if it must. So
      // here is the "default" implementation for copy-constructible types...
      //
      template <typename TYPE, typename Enable = void>
      struct DefaultStringConverter : DefaultStringConverterImpl<TYPE> {
        TYPE fromString( const TYPE& ref_value, const std::string& s ) final override
        {
          TYPE buffer = ref_value;
          this->fromStringImpl( buffer, s );
          return buffer;
        }
      };
      // ...and here is the preferred impl for default-constructible types:
      template <class TYPE>
      struct DefaultStringConverter<TYPE, std::enable_if_t<std::is_default_constructible<TYPE>::value>>
          : DefaultStringConverterImpl<TYPE> {
        TYPE fromString( const TYPE& /* ref_value */, const std::string& s ) final override
        {
          TYPE buffer{};
          this->fromStringImpl( buffer, s );
          return buffer;
        }
      };

      // Specializable StringConverter struct with a default implementation
      template <typename TYPE>
      struct StringConverter : DefaultStringConverter<TYPE> {
      };

      struct NullVerifier {
        template <class TYPE>
        void operator()( const TYPE& ) const
        {
        }
      };
      template <class TYPE>
      struct BoundedVerifier {
        void operator()( const TYPE& value ) const
        {
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
        void setReadHandler( std::function<void( PropertyBase& )> )
        {
          throw std::logic_error( "setReadHandler not implemented for this class" );
        }
        std::function<void( PropertyBase& )> getReadHandler() const { return nullptr; }
        void                                 useUpdateHandler( const PropertyBase& ) const {}
        void                                 setUpdateHandler( std::function<void( PropertyBase& )> )
        {
          throw std::logic_error( "setUpdateHandler not implemented for this class" );
        }
        std::function<void( PropertyBase& )> getUpdateHandler() const { return nullptr; }
      };
      struct ReadHandler : NoHandler {
        mutable std::function<void( PropertyBase& )> m_readCallBack;
        void useReadHandler( const PropertyBase& p ) const
        {
          if ( m_readCallBack ) {
            SwapCall{m_readCallBack}( const_cast<PropertyBase&>( p ) );
          }
        }
        void setReadHandler( std::function<void( PropertyBase& )> fun ) { m_readCallBack = std::move( fun ); }
        std::function<void( PropertyBase& )>                      getReadHandler() const { return m_readCallBack; }
      };
      struct UpdateHandler : NoHandler {
        std::function<void( PropertyBase& )> m_updateCallBack;
        void useUpdateHandler( PropertyBase& p )
        {
          if ( m_updateCallBack ) {
            try {
              SwapCall{m_updateCallBack}( p );
            } catch ( const std::exception& x ) {
              throw std::invalid_argument( "failure in update handler of '" + p.name() + "': " + x.what() );
            }
          }
        }
        void setUpdateHandler( std::function<void( PropertyBase& )> fun ) { m_updateCallBack = std::move( fun ); }
        std::function<void( PropertyBase& )> getUpdateHandler() const { return m_updateCallBack; }
      };
      struct ReadUpdateHandler : ReadHandler, UpdateHandler {
        using ReadHandler::useReadHandler;
        using ReadHandler::setReadHandler;
        using ReadHandler::getReadHandler;
        using UpdateHandler::useUpdateHandler;
        using UpdateHandler::setUpdateHandler;
        using UpdateHandler::getUpdateHandler;
      };
    }

  } // namespace Details

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
  class Property : public Details::PropertyBase
  {
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
    using is_this_type = std::is_same<Property, std::remove_reference_t<T>>;
    template <class T>
    using not_copying = std::enable_if_t<!is_this_type<T>::value>;
    /// @}
  public:
    // ==========================================================================
    /// the constructor with property name, value and documentation.
    template <class T = StorageType>
    Property( std::string name, T&& value, std::string doc = "" )
        : Details::PropertyBase( typeid( ValueType ), std::move( name ), std::move( doc ) )
        , m_value( std::forward<T>( value ) )
    {
      m_verifier( m_value );
    }
    /// Autodeclaring constructor with property name, value and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <typename OWNER, typename T = ValueType,
              typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>,
              typename = std::enable_if_t<std::is_default_constructible<T>::value>>
    Property( OWNER* owner, std::string name ) : Property( std::move( name ), ValueType{}, "" )
    {
      owner->declareProperty( *this );
      setOwnerType<OWNER>();
    }

    /// Autodeclaring constructor with property name, value and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
    Property( OWNER* owner, std::string name, T&& value, std::string doc = "" )
        : Property( std::move( name ), std::forward<T>( value ), std::move( doc ) )
    {
      owner->declareProperty( *this );
      setOwnerType<OWNER>();
    }

    /// Autodeclaring constructor with property name, value, updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
    Property( OWNER* owner, std::string name, T&& value, std::function<void( PropertyBase& )> handler,
              std::string doc = "" )
        : Property( owner, std::move( name ), std::forward<T>( value ), std::move( doc ) )
    {
      declareUpdateHandler( std::move( handler ) );
    }

    /// Autodeclaring constructor with property name, value, pointer to member function updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
    Property( OWNER* owner, std::string name, T&& value, void ( OWNER::*handler )( PropertyBase& ),
              std::string doc = "" )
        : Property( owner, std::move( name ), std::forward<T>( value ),
                    [owner, handler]( PropertyBase& p ) { ( owner->*handler )( p ); }, std::move( doc ) )
    {
    }
    /// Autodeclaring constructor with property name, value, pointer to member function updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
    Property( OWNER* owner, std::string name, T&& value, void ( OWNER::*handler )(), std::string doc = "" )
        : Property( owner, std::move( name ), std::forward<T>( value ),
                    [owner, handler]( PropertyBase& ) { ( owner->*handler )(); }, std::move( doc ) )
    {
    }

    /// Autodeclaring constructor with property name, value, updateHandler and documentation.
    /// @note the use std::enable_if is required to avoid ambiguities
    template <class OWNER, class T = StorageType, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
    Property( OWNER* owner, std::string name, T&& value, std::function<void( PropertyBase& )> handler,
              Details::Property::ImmediatelyInvokeHandler invoke, std::string doc = "" )
        : Property( owner, std::move( name ), std::forward<T>( value ), std::move( handler ), std::move( doc ) )
    {
      if ( invoke ) useUpdateHandler();
    }

    /// Construct an anonymous property from a value.
    /// This constructor is not generated if T is the current type, so that the
    /// compiler picks up the copy constructor instead of this one.
    template <typename T, typename = not_copying<T>>
    Property( T&& v ) : Details::PropertyBase( typeid( ValueType ), "", "" ), m_value( std::forward<T>( v ) )
    {
    }

    /// Construct an anonymous property with default constructed value.
    /// Can be used only if StorageType is default constructible.
    template <typename T = StorageType, typename = std::enable_if_t<!std::is_reference<T>::value>>
    Property() : Details::PropertyBase( typeid( ValueType ), "", "" ), m_value()
    {
    }

    using Details::PropertyBase::declareReadHandler;
    using Details::PropertyBase::declareUpdateHandler;

    /// set new callback for reading
    Details::PropertyBase& declareReadHandler( std::function<void( Details::PropertyBase& )> fun ) override
    {
      m_handlers.setReadHandler( std::move( fun ) );
      return *this;
    }
    /// set new callback for update
    Details::PropertyBase& declareUpdateHandler( std::function<void( Details::PropertyBase& )> fun ) override
    {
      m_handlers.setUpdateHandler( std::move( fun ) );
      return *this;
    }

    /// get a reference to the readCallBack
    const std::function<void( Details::PropertyBase& )> readCallBack() const override
    {
      return m_handlers.getReadHandler();
    }
    /// get a reference to the updateCallBack
    const std::function<void( Details::PropertyBase& )> updateCallBack() const override
    {
      return m_handlers.getUpdateHandler();
    }

    /// manual trigger for callback for update
    bool useUpdateHandler() override
    {
      m_handlers.useUpdateHandler( *this );
      return true;
    }

    /// Automatic conversion to value (const reference).
    operator const ValueType&() const
    {
      m_handlers.useReadHandler( *this );
      return m_value;
    }
    // /// Automatic conversion to value (reference).
    // operator ValueType& () {
    //   useReadHandler();
    //   return m_value;
    // }

    /// equality comparison
    template <class T>
    bool operator==( const T& other ) const
    {
      return m_value == other;
    }

    /// inequality comparison
    template <class T>
    bool operator!=( const T& other ) const
    {
      return m_value != other;
    }

    /// "less" comparison
    template <class T>
    bool operator<( const T& other ) const
    {
      return m_value < other;
    }

    /// allow addition if possible between the property and the other types
    template <class T>
    decltype( auto ) operator+( const T& other ) const
    {
      return m_value + other;
    }

    /// Assignment from value.
    template <class T = ValueType>
    Property& operator=( T&& v )
    {
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
    ValueType&       value() { return const_cast<ValueType&>( (const ValueType&)*this ); }
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
    Details::PropertyBase* clone() const override { return new Property( *this ); }
    /// @}

    /// @name Helpers for easy use of string and vector properties.
    /// @{
    /// They are instantiated only if they are implemented in the wrapped class.
    template <class T = const ValueType>
    decltype( auto ) size() const
    {
      return value().size();
    }
    template <class T = const ValueType>
    decltype( auto ) length() const
    {
      return value().length();
    }
    template <class T = const ValueType>
    decltype( auto ) empty() const
    {
      return value().empty();
    }
    template <class T = ValueType>
    decltype( auto ) clear()
    {
      value().clear();
    }
    template <class T = const ValueType>
    decltype( auto ) begin() const
    {
      return value().begin();
    }
    template <class T = const ValueType>
    decltype( auto ) end() const
    {
      return value().end();
    }
    template <class T = ValueType>
    decltype( auto ) begin()
    {
      return value().begin();
    }
    template <class T = ValueType>
    decltype( auto ) end()
    {
      return value().end();
    }
    template <class ARG>
    decltype( auto ) operator[]( const ARG& arg ) const
    {
      return value()[arg];
    }
    template <class ARG>
    decltype( auto ) operator[]( const ARG& arg )
    {
      return value()[arg];
    }
    template <class T = const ValueType>
    decltype( auto ) find( const typename T::key_type& key ) const
    {
      return value().find( key );
    }
    template <class T = ValueType>
    decltype( auto ) find( const typename T::key_type& key )
    {
      return value().find( key );
    }
    template <class ARG, class T = ValueType>
    decltype( auto ) erase( ARG arg )
    {
      return value().erase( arg );
    }
    template <class = ValueType>
    Property& operator++()
    {
      ++value();
      return *this;
    }
    template <class = ValueType>
    ValueType operator++( int )
    {
      return m_value++;
    }
    template <class = ValueType>
    Property& operator--()
    {
      --value();
      return *this;
    }
    template <class = ValueType>
    ValueType operator--( int )
    {
      return m_value--;
    }
    template <class T = ValueType>
    Property& operator+=( const T& other )
    {
      m_value += other;
      return *this;
    }
    template <class T = ValueType>
    Property& operator-=( const T& other )
    {
      m_value -= other;
      return *this;
    }
    /// Helpers for DataHandles and derived classes
    template <class T = const ValueType>
    decltype( auto ) targetKey() const
    {
      return value().targetKey();
    }
    template <class ARG, class T = ValueType>
    decltype( std::declval<T>().setTargetKey( std::declval<ARG>() ) ) setTargetKey( const ARG& arg )
    {
      return value().setTargetKey( arg );
    }
    template <class T = const ValueType>
    decltype( auto ) key() const
    {
      return value().key();
    }
    template <class T = const ValueType>
    decltype( auto ) objKey() const
    {
      return value().objKey();
    }
    template <class T = const ValueType>
    decltype( auto ) fullKey() const
    {
      return value().fullKey();
    }
    template <class T = ValueType>
    decltype( auto ) initialize()
    {
      return value().initialize();
    }
    template <class T = ValueType>
    decltype( auto ) makeHandles() const
    {
      return value().makeHandles();
    }
    template <class ARG, class T = ValueType>
    decltype( auto ) makeHandles( const ARG& arg ) const
    {
      return value().makeHandles( arg );
    }
    /// @}
    // ==========================================================================

    // Delegate operator() to the value
    template <class... Args>
    decltype( std::declval<ValueType>()( std::declval<Args&&>()... ) ) operator()( Args&&... args ) const
        noexcept( noexcept( std::declval<ValueType>()( std::declval<Args&&>()... ) ) )
    {
      return value()( std::forward<Args>( args )... );
    }

  public:
    /// get the value from another property
    bool assign( const Details::PropertyBase& source ) override
    {
      // Check if the property of is of "the same" type, except for strings
      const Property* p =
          ( std::is_same<ValueType, std::string>::value ) ? nullptr : dynamic_cast<const Property*>( &source );
      if ( p ) {
        *this = p->value();
      } else {
        this->fromString( source.toString() ).ignore();
      }
      return true;
    }
    /// set value to another property
    bool load( Details::PropertyBase& dest ) const override
    {
      // delegate to the 'opposite' method
      return dest.assign( *this );
    }
    /// string -> value
    StatusCode fromString( const std::string& source ) override
    {
      using Converter = Details::Property::StringConverter<ValueType>;
      *this           = Converter().fromString( m_value, source );
      return StatusCode::SUCCESS;
    }
    /// value  -> string
    std::string toString() const override
    {
      using Converter = Details::Property::StringConverter<ValueType>;
      return Converter().toString( *this );
    }
    /// value  -> stream
    void toStream( std::ostream& out ) const override
    {
      m_handlers.useReadHandler( *this );
      using Utils::toStream;
      toStream( m_value, out );
    }
  };

  /// delegate (value == property) to property operator==
  template <class T, class TP, class V, class H>
  bool operator==( const T& v, const Property<TP, V, H>& p )
  {
    return p == v;
  }

  /// delegate (value != property) to property operator!=
  template <class T, class TP, class V, class H>
  bool operator!=( const T& v, const Property<TP, V, H>& p )
  {
    return p != v;
  }

  /// implemantation of (value + property)
  template <class T, class TP, class V, class H>
  decltype( auto ) operator+( const T& v, const Property<TP, V, H>& p )
  {
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
class PropertyWithHandlers : public Gaudi::Details::PropertyBase
{
  Handler m_handlers;

public:
  using PropertyBase::PropertyBase;

  /// set new callback for reading
  PropertyBase& declareReadHandler( std::function<void( PropertyBase& )> fun ) override
  {
    m_handlers.setReadHandler( std::move( fun ) );
    return *this;
  }
  /// set new callback for update
  PropertyBase& declareUpdateHandler( std::function<void( PropertyBase& )> fun ) override
  {
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
  bool useUpdateHandler() override
  {
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
class GAUDI_API GaudiHandleProperty : public PropertyWithHandlers<>
{
public:
  GaudiHandleProperty( std::string name, GaudiHandleBase& ref );

  GaudiHandleProperty& operator=( const GaudiHandleBase& value )
  {
    setValue( value );
    return *this;
  }

  GaudiHandleProperty* clone() const override { return new GaudiHandleProperty( *this ); }

  bool load( PropertyBase& destination ) const override { return destination.assign( *this ); }

  bool assign( const PropertyBase& source ) override { return fromString( source.toString() ).isSuccess(); }

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

class GAUDI_API GaudiHandleArrayProperty : public PropertyWithHandlers<>
{
public:
  GaudiHandleArrayProperty( std::string name, GaudiHandleArrayBase& ref );

  GaudiHandleArrayProperty& operator=( const GaudiHandleArrayBase& value )
  {
    setValue( value );
    return *this;
  }

  GaudiHandleArrayProperty* clone() const override { return new GaudiHandleArrayProperty( *this ); }

  bool load( PropertyBase& destination ) const override { return destination.assign( *this ); }

  bool assign( const PropertyBase& source ) override { return fromString( source.toString() ).isSuccess(); }

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
    GAUDI_API Gaudi::Details::PropertyBase* getProperty( const IProperty* p, const std::string& name );
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
    GAUDI_API Gaudi::Details::PropertyBase* getProperty( const IInterface* p, const std::string& name );
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
    GAUDI_API bool hasProperty( const std::vector<const Gaudi::Details::PropertyBase*>* p, const std::string& name );
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
    getProperty( const std::vector<const Gaudi::Details::PropertyBase*>* p, const std::string& name );
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
