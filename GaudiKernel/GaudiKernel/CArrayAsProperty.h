#ifndef NO_C_ARRAY_AS_PROPERTY_WARNING
#  warning deprecated header (will be removed in Gaudi v29r0), think about using std::array<T,N> instead of T[N]
#endif
#ifndef GAUDIKERNEL_CARRAYASPROPERTY_H
#  define GAUDIKERNEL_CARRAYASPROPERTY_H
#  include "GaudiKernel/Property.h"

namespace Gaudi {
  template <class TYPE, size_t N, class VERIFIER, class HANDLERS>
  class Property<TYPE[N], VERIFIER, HANDLERS> : public Details::PropertyBase {
  public:
    // ==========================================================================
    /// Hosted type
    using StorageType  = TYPE ( & )[N];
    using ValueType    = typename std::remove_reference<StorageType>::type;
    using VerifierType = VERIFIER;
    using HandlersType = HANDLERS;

  private:
    /// Storage.
    StorageType  m_value;
    VerifierType m_verifier;
    HandlersType m_handlers;
    /// helper typedefs for SFINAE
    /// @{
    template <class T>
    using is_this_type = std::is_same<Property, typename std::remove_reference<T>::type>;
    template <class T>
    using not_copying = std::enable_if<!is_this_type<T>::value>;
    /// @}
  public:
    // ==========================================================================
    /// the constructor with property name, value and documentation.
    inline Property( std::string name, StorageType value, std::string doc = "" )
        : PropertyBase( typeid( ValueType ), std::move( name ), std::move( doc ) ), m_value( value ) {
      m_verifier( m_value );
    }

    using PropertyBase::declareReadHandler;
    using PropertyBase::declareUpdateHandler;

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

    /// Assignment from value.
    Property& operator=( const ValueType& v ) {
      m_verifier( v );
      for ( size_t i = 0; i != N; ++i ) { m_value[i] = v[i]; }
      m_handlers.useUpdateHandler( *this );
      return *this;
    }

    /// Copy constructor.
    // Property(const Property& other):
    //   PropertyBase(other), value(other.m_value) {}

    /// Accessor to verifier.
    const VerifierType& verifier() const { return m_verifier; }
    /// Accessor to verifier.
    VerifierType& verifier() { return m_verifier; }

    /// Backward compatibility \deprecated will be removed in v29r0
    /// @{
    const ValueType& value() const { return *this; }
    ValueType&       value() { return const_cast<ValueType&>( (const ValueType&)*this ); }
    bool             setValue( const ValueType& v ) {
      *this = v;
      return true;
    }
    bool set( const ValueType& v ) {
      *this = v;
      return true;
    }
    PropertyBase* clone() const override { return new Property( *this ); }
    /// @}

    /// @name Helpers for easy use of string and vector properties.
    /// @{
    /// They are instantiated only if they are implemented in the wrapped class.
    inline size_t size() const { return N; }
    inline bool   empty() const { return false; }
    template <class T = const ValueType>
    inline decltype( std::declval<T>()[typename T::key_type{}] ) operator[]( const typename T::key_type& key ) const {
      return value()[key];
    }
    template <class T = ValueType>
    inline decltype( std::declval<T>()[typename T::key_type{}] ) operator[]( const typename T::key_type& key ) {
      return value()[key];
    }
    /// @}
    // ==========================================================================
  public:
    /// get the value from another property
    bool assign( const PropertyBase& source ) override {
      // Is the property of "the same" type?
      const Property* p = dynamic_cast<const Property*>( &source );
      if ( p ) {
        *this = p->value();
      } else {
        this->fromString( source.toString() ).ignore();
      }
      return true;
    }
    /// set value to another property
    bool load( PropertyBase& dest ) const override {
      // delegate to the 'opposite' method
      return dest.assign( *this );
    }
    /// string -> value
    StatusCode fromString( const std::string& source ) override {
      ValueType tmp;
      if ( Parsers::parse( tmp, source ).isSuccess() ) {
        *this = tmp;
      } else {
        throw std::invalid_argument( "cannot parse '" + source + "' to " + this->type() );
      }
      return StatusCode::SUCCESS;
    }
    /// value  -> string
    std::string toString() const override {
      m_handlers.useReadHandler( *this );
      return Utils::toString( m_value );
    }
    /// value  -> stream
    void toStream( std::ostream& out ) const override {
      m_handlers.useReadHandler( *this );
      Utils::toStream( m_value, out );
    }
  };
  template <class TYPE, size_t N, class VERIFIER, class HANDLERS>
  class Property<TYPE ( & )[N], VERIFIER, HANDLERS> : public Property<TYPE[N], VERIFIER, HANDLERS> {
  public:
    using Property<TYPE[N], VERIFIER, HANDLERS>::Property;
  };
} // namespace Gaudi
#endif
