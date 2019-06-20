#ifndef GAUDIHIVE_DATAOBJECTHANDLE_H
#define GAUDIHIVE_DATAOBJECTHANDLE_H

#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/NamedRange.h"

#include <type_traits>

//---------------------------------------------------------------------------
//
namespace details {
  template <typename T>
  using Converter_t = T ( * )( const DataObject* );

  template <typename Range, typename StorageType>
  Range make_range( const DataObject* obj ) {
    auto c = static_cast<const StorageType*>( obj );
    if ( UNLIKELY( !c ) ) return Range();
    using std::begin;
    using std::end;
    auto first = begin( *c );
    auto last  = end( *c );
    // return Range( first, last );
    auto _first = reinterpret_cast<typename Range::const_iterator*>( &first );
    auto _last  = reinterpret_cast<typename Range::const_iterator*>( &last );
    return Range( *_first, *_last );
  }

  template <typename ValueType, typename Range = Gaudi::Range_<typename ValueType::ConstVector>>
  Converter_t<Range> select_range_converter( const DataObject* obj ) {
    using Selection = typename ValueType::Selection;
    auto sel        = dynamic_cast<const Selection*>( obj );
    if ( sel ) return &make_range<Range, typename ValueType::Selection>;
    auto con = dynamic_cast<std::add_const_t<typename ValueType::Container>*>( obj );
    if ( con ) return &make_range<Range, typename ValueType::Container>;
    return nullptr;
  }

  template <typename T>
  bool verifyType( const DataObject* dataObj ) {
    using Type = std::add_const_t<T>;
    assert( dataObj != nullptr );
    auto obj = dynamic_cast<Type*>( dataObj );
    bool ok  = ( obj != nullptr );
    if ( !ok ) {
      const auto* registry = dataObj->registry();
      throw GaudiException( "The type expected for " + registry->identifier() + " is " +
                                System::typeinfoName( typeid( Type ) ) +
                                " and is different from the one of the object in the store which is " +
                                System::typeinfoName( typeid( *dataObj ) ) + ".",
                            "Wrong DataObjectType", StatusCode::FAILURE );
    }
    assert( obj == static_cast<Type*>( dataObj ) );
    return ok;
  }

  template <typename T>
  struct Payload_helper {
    using type = std::conditional_t<std::is_base_of<DataObject, T>::value, T, AnyDataWrapper<T>>;
  };
  template <typename T>
  struct Payload_helper<Gaudi::Range_<T>> {
    using type = Gaudi::Range_<T>;
  };

  template <typename T>
  using Payload_t = typename Payload_helper<T>::type;
} // namespace details

//---------------------------------------------------------------------------

/** DataObjectHandle.h GaudiKernel/DataObjectHandle.h
 *
 * Templated Handle class for objects in the event store
 *
 * Inheritance: DataHandle->DataObjectHandleBase->DataObjectHandle<T>
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

template <typename T>
class DataObjectHandle : public DataObjectHandleBase {
public:
  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  T* get() const { return get( true ); }

  /**
   * Bypass check of existence of object in transient store
   * Only uses main location of the
   */
  T* getIfExists() const { return get( false ); }

  /**
   * Check the existence of the object in the transient store
   */
  bool exist() const { return get( false ) != nullptr; }

  /**
   * Get object from store or create a new one if it doesn't exist
   */
  T* getOrCreate() const;

  /**
   * Register object in transient store
   */
  T* put( std::unique_ptr<T> object ) const;

  // [[deprecated("please pass a std::unique_ptr instead of a raw pointer")]]
  T* put( T* object ) const { return put( std::unique_ptr<T>( object ) ); }

private:
  T*           get( bool mustExist ) const;
  mutable bool m_goodType = false;
};

//---------------------------------------------------------------------------
//
/**
 * Try to retrieve from the transient store. If the retrieval succeded and
 * this is the first time we retrieve, perform a dynamic cast to the desired
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template <typename T>
T* DataObjectHandle<T>::get( bool mustExist ) const {
  auto dataObj = fetch();
  if ( UNLIKELY( !dataObj ) ) {
    if ( mustExist ) { // Problems in getting from the store
      throw GaudiException( "Cannot retrieve " + objKey() + " from transient store.",
                            m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
    }
    return nullptr;
  }
  if ( UNLIKELY( !m_goodType ) ) m_goodType = ::details::verifyType<T>( dataObj );
  return static_cast<T*>( dataObj );
}

//---------------------------------------------------------------------------
template <typename T>
T* DataObjectHandle<T>::put( std::unique_ptr<T> objectp ) const {
  assert( m_init );
  StatusCode rc = m_EDS->registerObject( objKey(), objectp.get() );
  if ( !rc.isSuccess() ) {
    throw GaudiException( "Error in put of " + objKey(), "DataObjectHandle<T>::put", StatusCode::FAILURE );
  }
  return objectp.release();
}

//---------------------------------------------------------------------------
template <typename T>
T* DataObjectHandle<T>::getOrCreate() const {
  T* obj = get( false );
  return obj ? obj : put( std::make_unique<T>() );
}

//---------------------------------------------------------------------------
/// specialization for Range_

template <typename T>
class DataObjectHandle<Gaudi::Range_<T>> : public DataObjectHandleBase {
public:
  using ValueType = std::remove_cv_t<std::remove_pointer_t<typename T::value_type>>;
  using Range     = Gaudi::Range_<typename ValueType::ConstVector>;

  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  Range get() const;

private:
  mutable ::details::Converter_t<Range> m_converter = nullptr;
};

template <typename ValueType>
auto DataObjectHandle<Gaudi::Range_<ValueType>>::get() const -> Range {
  auto dataObj = fetch();
  if ( UNLIKELY( !dataObj ) ) {
    throw GaudiException( "Cannot retrieve " + objKey() + " from transient store.",
                          m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
  }
  if ( UNLIKELY( !m_converter ) ) {
    m_converter = ::details::select_range_converter<ValueType>( dataObj );
    if ( !m_converter ) {
      throw GaudiException( "The type requested for " + objKey() + " (" + System::typeinfoName( typeid( ValueType ) ) +
                                ")" + " cannot be obtained from object in event store" + " (" +
                                System::typeinfoName( typeid( *dataObj ) ) + ").",
                            "Wrong DataObjectType", StatusCode::FAILURE );
    }
  }
  return ( *m_converter )( dataObj );
}

//---------------------------------------------------------------------------
/// specialization for AnyDataWrapper
template <typename T>
class DataObjectHandle<AnyDataWrapper<T>> : public DataObjectHandleBase {
public:
  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  T* get() const { return &_get( true )->getData(); }
  T* getIfExists() const {
    auto data = _get( false );
    if ( data ) {
      return &data->getData();
    } else {
      return nullptr;
    }
  }

  /**
   * Register object in transient store
   */
  const T* put( T&& object ) const;

  /**
   * Size of boxed item, if boxed item has a 'size' method
   */
  std::optional<std::size_t> size() const { return _get()->size(); }

private:
  AnyDataWrapper<T>* _get( bool mustExist ) const;
  mutable bool       m_goodType = false;
};

//---------------------------------------------------------------------------

template <typename T>
AnyDataWrapper<T>* DataObjectHandle<AnyDataWrapper<T>>::_get( bool mustExist ) const {
  auto obj = fetch();
  if ( UNLIKELY( !obj ) ) {
    if ( mustExist ) {
      throw GaudiException( "Cannot retrieve " + objKey() + " from transient store.",
                            m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );

    } else {
      return nullptr;
    }
  }
  if ( UNLIKELY( !m_goodType ) ) m_goodType = ::details::verifyType<AnyDataWrapper<T>>( obj );
  return static_cast<AnyDataWrapper<T>*>( obj );
}

//---------------------------------------------------------------------------

template <typename T>
const T* DataObjectHandle<AnyDataWrapper<T>>::put( T&& obj ) const {
  assert( m_init );
  auto       objectp = std::make_unique<AnyDataWrapper<T>>( std::move( obj ) );
  StatusCode rc      = m_EDS->registerObject( objKey(), objectp.get() );
  if ( rc.isFailure() ) {
    throw GaudiException( "Error in put of " + objKey(), "DataObjectHandle<T>::put", StatusCode::FAILURE );
  }
  return &objectp.release()->getData();
}

//---------------------------- user-facing interface ----------

template <typename T>
class DataObjectReadHandle : public DataObjectHandle<::details::Payload_t<T>> {
  template <typename... Args, std::size_t... Is>
  DataObjectReadHandle( const std::tuple<Args...>& args, std::index_sequence<Is...> )
      : DataObjectReadHandle( std::get<Is>( args )... ) {}

public:
  DataObjectReadHandle( const DataObjID& k, IDataHandleHolder* owner )
      : DataObjectHandle<::details::Payload_t<T>>{k, Gaudi::DataHandle::Reader, owner} {}

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <typename OWNER, typename K, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
  DataObjectReadHandle( OWNER* owner, std::string propertyName, const K& key = {}, std::string doc = "" )
      : DataObjectHandle<::details::Payload_t<T>>( owner, Gaudi::DataHandle::Reader, std::move( propertyName ), key,
                                                   std::move( doc ) ) {}

  template <typename... Args>
  DataObjectReadHandle( const std::tuple<Args...>& args )
      : DataObjectReadHandle( args, std::index_sequence_for<Args...>{} ) {}
};

template <typename T>
class DataObjectWriteHandle : public DataObjectHandle<::details::Payload_t<T>> {
  template <typename... Args, std::size_t... Is>
  DataObjectWriteHandle( const std::tuple<Args...>& args, std::index_sequence<Is...> )
      : DataObjectWriteHandle( std::get<Is>( args )... ) {}

public:
  DataObjectWriteHandle( const DataObjID& k, IDataHandleHolder* owner )
      : DataObjectHandle<::details::Payload_t<T>>{k, Gaudi::DataHandle::Writer, owner} {}

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <typename OWNER, typename K, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
  DataObjectWriteHandle( OWNER* owner, std::string propertyName, const K& key = {}, std::string doc = "" )
      : DataObjectHandle<::details::Payload_t<T>>( owner, Gaudi::DataHandle::Writer, std::move( propertyName ), key,
                                                   std::move( doc ) ) {}

  template <typename... Args>
  DataObjectWriteHandle( const std::tuple<Args...>& args )
      : DataObjectWriteHandle( args, std::index_sequence_for<Args...>{} ) {}
};

#endif
