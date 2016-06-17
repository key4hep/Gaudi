#warning deprecated header (will be removed in Gaudi v28r1), think about using std::array<T,N> instead of T[N]

#include "GaudiKernel/Property.h"

// template<class T, size_t N, class VERIFIER>
// class PropertyWithValue<T(&)[N], VERIFIER>: public PropertyWithValue<std::array<T,N>&, VERIFIER>{
//   using PropertyWithValue::PropertyWithValue;
// };
template<class TYPE, size_t N, class VERIFIER>
class PropertyWithValue<TYPE(&)[N], VERIFIER>: public Property
{
public:
  // ==========================================================================
  /// Hosted type
  using StorageType = TYPE(&)[N];
  using ValueType = typename std::remove_reference<StorageType>::type;
  using VerifierType = VERIFIER;
private:
  /// Storage.
  StorageType m_value;
  VerifierType m_verifier;
  /// helper typedefs for SFINAE
  /// @{
  template<class T>
  using is_this_type = std::is_same<Property, typename std::remove_reference<T>::type>;
  template<class T>
  using not_copying = std::enable_if<!is_this_type<T>::value>;
  /// @}
public:

  // ==========================================================================
  /// the constructor with property name, value and documentation.
  template <class T=ValueType>
  inline PropertyWithValue
  ( std::string        name    ,
    T&&                value   ,
    std::string        doc = "")
    : Property(typeid(ValueType), std::move(name), std::move(doc))
    , m_value(std::forward<T>(value))
  {
    m_verifier(m_value);
  }
  /// the constructor with property name, value and documentation.
  template <class T=ValueType>
  inline PropertyWithValue
  ( PropertyMgr*       owner   ,
    std::string        name    ,
    T&&                value = ValueType{},
    std::string        doc = "")
    : PropertyWithValue(std::move(name), std::forward<T>(value), std::move(doc))
  {
    this->declareTo(owner);
  }

  /// Construct an anonymous property from a value.
  /// This constructor is not generated if T is the current type, so that the
  /// compiler picks up the copy constructor instead of this one.
  template <class T=ValueType, typename = typename not_copying<T>::type>
  PropertyWithValue(T&& v): Property(typeid(ValueType), "", ""),
                            m_value(std::forward<T>(v)) {}

  /// Construct an anonymous property with default constructed value.
  /// Can be used only if StorageType is default constructible.
  template <typename = void>
  PropertyWithValue(): Property(typeid(ValueType), "", ""),
                       m_value() {}

  /// Automatic conversion to value (const reference).
  operator const ValueType& () const {
    useReadHandler();
    return m_value;
  }
  // /// Automatic conversion to value (const reference).
  // operator ValueType& () {
  //   useReadHandler();
  //   return m_value;
  // }

  /// Assignment from value.
  PropertyWithValue& operator=(const ValueType& v) {
    m_verifier(v);
    for(size_t i = 0; i != N; ++i) {
      m_value[i] = v[i];
    }
    useUpdateHandler();
    return *this;
  }

  /// Copy constructor.
  // PropertyWithValue(const PropertyWithValue& other):
  //   Property(other), value(other.m_value) {}

  /// Accessor to verifier.
  const VerifierType& verifier() const { return m_verifier; }
  /// Accessor to verifier.
  VerifierType& verifier() { return m_verifier; }

  /// Backward compatibility (deprecated)
  /// @{
  const ValueType& value() const { return *this; }
  ValueType& value() {
    useReadHandler();
    return m_value;
  }
  void setValue(const ValueType& v) {
    *this = v;
  }
  void set(const ValueType& v) {
    *this = v;
  }
  /// @}

  /// @name Helpers for easy use of string and vector properties.
  /// @{
  /// They are instantiated only if they are implemented in the wrapped class.
  inline size_t size() const { return N; }
  inline bool empty() const { return false; }
  template<class T=const ValueType>
  inline decltype(std::declval<T>()[typename T::key_type{}])
    operator[] (const typename T::key_type & key) const { return value()[key]; }
  template<class T=ValueType>
  inline decltype(std::declval<T>()[typename T::key_type{}])
    operator[] (const typename T::key_type & key) { return value()[key]; }
  /// @}
  // ==========================================================================
public:
  /// get the value from another property
  bool assign(const Property& source) override {
    // Is the property of "the same" type?
    const PropertyWithValue* p =
      dynamic_cast<const PropertyWithValue*>(&source) ;
    if (p) {
      *this = p->value();
    } else {
      this->fromString(source.toString());
    }
    return true;
  }
  /// set value to another property
  bool load(Property& dest) const override {
    // delegate to the 'opposite' method
    return dest.assign(*this);
  }
  /// string -> value
  StatusCode fromString(const std::string& source) override {
    ValueType tmp;
    if (Gaudi::Parsers::parse(tmp, source).isSuccess()) {
      *this = tmp;
    } else {
      throw std::invalid_argument("cannot parse '" + source + "' to " + this->type());
    }
    return StatusCode::SUCCESS;
  }
  /// value  -> string
  std::string toString() const override {
    useReadHandler();
    return Gaudi::Utils::toString(m_value) ;
  }
  /// value  -> stream
  void toStream(std::ostream& out) const  override {
    useReadHandler();
    Gaudi::Utils::toStream(m_value, out ) ;
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
