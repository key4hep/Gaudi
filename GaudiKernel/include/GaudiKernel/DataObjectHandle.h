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

#include <GaudiKernel/AnyDataWrapper.h>
#include <GaudiKernel/DataObjectHandleBase.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IDataHandleHolder.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/NamedRange.h>
#include <boost/algorithm/string/replace.hpp>
#include <type_traits>

//---------------------------------------------------------------------------
//
namespace details {
  template <typename T>
  using Converter_t = T ( * )( const DataObject* );

  template <typename Range, typename StorageType>
  Range make_range( const DataObject* obj ) {
    auto c = static_cast<const StorageType*>( obj );
    if ( !c ) return Range();
    using std::begin;
    using std::end;
    auto first  = begin( *c );
    auto last   = end( *c );
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

  template <Gaudi::DataHandle::Mode, typename T, typename U>
  struct Payload_helper {
    using type =
        std::conditional_t<std::is_base_of_v<DataObject, T> && std::is_same_v<T, U>, T,
                           std::conditional_t<std::is_same_v<T, U>, AnyDataWrapper<std::remove_const_t<T>>,
                                              AnyDataWithViewWrapper<std::remove_const_t<T>, std::remove_const_t<U>>>>;
  };
  template <typename T, typename U>
  struct Payload_helper<Gaudi::DataHandle::Reader, Gaudi::Range_<T>, U> {
    using type = Gaudi::Range_<T>;
  };
  template <typename T, typename U>
  struct Payload_helper<Gaudi::DataHandle::Reader, Gaudi::NamedRange_<T>, U> {
    using type = Gaudi::NamedRange_<T>;
  };
  template <typename T, typename U>
  struct Payload_helper<Gaudi::DataHandle::Reader, std::optional<Gaudi::NamedRange_<T>>, U> {
    using type = std::optional<Gaudi::NamedRange_<T>>;
  };

  template <Gaudi::DataHandle::Mode mode, typename T, typename U = T>
  using Payload_t = typename Payload_helper<mode, T, U>::type;

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

  std::string pythonRepr() const override {
    auto repr = DataObjectHandleBase::pythonRepr();
    boost::replace_all( repr, default_type, System::typeinfoName( typeid( T ) ) );
    return repr;
  }

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
  if ( !dataObj ) {
    if ( mustExist ) { // Problems in getting from the store
      throw GaudiException( "Cannot retrieve \'" + objKey() + "\' from transient store.",
                            m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
    }
    return nullptr;
  }
  if ( !m_goodType ) m_goodType = ::details::verifyType<T>( dataObj );
  return static_cast<T*>( dataObj );
}

//---------------------------------------------------------------------------
template <typename T>
T* DataObjectHandle<T>::put( std::unique_ptr<T> objectp ) const {
  assert( m_init );
  StatusCode sc = m_EDS->registerObject( objKey(), objectp.get() );
  if ( !sc.isSuccess() ) { throw GaudiException( "Error in put of " + objKey(), "DataObjectHandle<T>::put", sc ); }
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

  std::string pythonRepr() const override {
    auto repr = DataObjectHandleBase::pythonRepr();
    boost::replace_all( repr, default_type, System::typeinfoName( typeid( Gaudi::Range_<T> ) ) );
    return repr;
  }

private:
  mutable ::details::Converter_t<Range> m_converter = nullptr;
};

template <typename ValueType>
auto DataObjectHandle<Gaudi::Range_<ValueType>>::get() const -> Range {
  auto dataObj = fetch();
  if ( !dataObj ) {
    throw GaudiException( "Cannot retrieve \'" + objKey() + "\' from transient store.",
                          m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
  }
  if ( !m_converter ) {
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
/// specialization for NamedRange_

template <typename T>
class DataObjectHandle<Gaudi::NamedRange_<T>> : public DataObjectHandleBase {
public:
  using ValueType = std::remove_cv_t<std::remove_pointer_t<typename T::value_type>>;
  using Range     = Gaudi::NamedRange_<typename ValueType::ConstVector>;

  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  Range get() const;

  std::string pythonRepr() const override {
    auto repr = DataObjectHandleBase::pythonRepr();
    boost::replace_all( repr, default_type, System::typeinfoName( typeid( Gaudi::NamedRange_<T> ) ) );
    return repr;
  }

private:
  mutable ::details::Converter_t<Range> m_converter = nullptr;
};

template <typename ValueType>
auto DataObjectHandle<Gaudi::NamedRange_<ValueType>>::get() const -> Range {
  auto dataObj = fetch();
  if ( !dataObj ) {
    throw GaudiException( "Cannot retrieve \'" + objKey() + "\' from transient store.",
                          m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
  }
  if ( !m_converter ) {
    m_converter = ::details::select_range_converter<ValueType, Range>( dataObj );
    if ( !m_converter ) {
      throw GaudiException( "The type requested for " + objKey() + " (" + System::typeinfoName( typeid( ValueType ) ) +
                                ")" + " cannot be obtained from object in event store" + " (" +
                                System::typeinfoName( typeid( *dataObj ) ) + ").",
                            "Wrong DataObjectType", StatusCode::FAILURE );
    }
  }
  return ( *m_converter )( dataObj );
}

//
//---------------------------------------------------------------------------
/// specialization for optional<NamedRange_>

template <typename T>
class DataObjectHandle<std::optional<Gaudi::NamedRange_<T>>> : public DataObjectHandleBase {
public:
  using ValueType = std::remove_cv_t<std::remove_pointer_t<typename T::value_type>>;
  using Range     = Gaudi::NamedRange_<typename ValueType::ConstVector>;

  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  std::optional<Range> get() const;

  std::string pythonRepr() const override {
    auto repr = DataObjectHandleBase::pythonRepr();
    boost::replace_all( repr, default_type, System::typeinfoName( typeid( std::optional<Gaudi::NamedRange_<T>> ) ) );
    return repr;
  }

private:
  mutable ::details::Converter_t<Range> m_converter = nullptr;
};

template <typename ValueType>
auto DataObjectHandle<std::optional<Gaudi::NamedRange_<ValueType>>>::get() const -> std::optional<Range> {
  auto dataObj = fetch();
  if ( !dataObj ) return std::nullopt;
  if ( !m_converter ) {
    m_converter = ::details::select_range_converter<ValueType, Range>( dataObj );
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
    return data ? &data->getData() : nullptr;
  }

  /**
   * Register object in transient store
   */
  const T* put( T&& obj ) const {
    assert( m_init );
    auto objectp = std::make_unique<AnyDataWrapper<T>>( std::move( obj ) );
    if ( auto sc = m_EDS->registerObject( objKey(), objectp.get() ); sc.isFailure() ) {
      throw GaudiException( "Error in put of " + objKey(), "DataObjectHandle<AnyDataWrapper<T>>::put", sc );
    }
    return &objectp.release()->getData();
  }

  /**
   * Size of boxed item, if boxed item has a 'size' method
   */
  std::optional<std::size_t> size() const { return _get()->size(); }

  std::string pythonRepr() const override {
    auto repr = DataObjectHandleBase::pythonRepr();
    boost::replace_all( repr, default_type, System::typeinfoName( typeid( T ) ) );
    return repr;
  }

private:
  AnyDataWrapper<T>* _get( bool mustExist ) const {
    auto obj = fetch();
    if ( !obj ) {
      if ( mustExist ) {
        throw GaudiException( "Cannot retrieve \'" + objKey() + "\' from transient store.",
                              m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );

      } else {
        return nullptr;
      }
    }
    if ( !m_goodType ) m_goodType = ::details::verifyType<AnyDataWrapper<T>>( obj );
    return static_cast<AnyDataWrapper<T>*>( obj );
  }
  mutable bool m_goodType = false;
};

//---------------------------------------------------------------------------
/// specialization for AnyDataWithViewWrapper
template <typename View, typename Owned>
class DataObjectHandle<AnyDataWithViewWrapper<View, Owned>> : public DataObjectHandleBase {
public:
  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  View* get() const { return &_get( true )->getData(); }
  View* getIfExists() const {
    auto data = _get( false );
    return data ? &data->getData() : nullptr;
  }

  /**
   * Register object in transient store
   */
  const View* put( std::unique_ptr<AnyDataWithViewWrapper<View, Owned>> objectp ) const {
    assert( m_init );
    if ( auto sc = m_EDS->registerObject( objKey(), objectp.get() ); sc.isFailure() ) {
      throw GaudiException( "Error in put of " + objKey(), "DataObjectHandle<AnyDataWithViewWrapper<T>::put", sc );
    }
    return &objectp.release()->getData();
  }
  const View* put( Owned&& obj ) const {
    return put( std::make_unique<AnyDataWithViewWrapper<View, Owned>>( std::move( obj ) ) );
  }

  /**
   * Size of boxed item, if boxed item has a 'size' method
   */
  std::optional<std::size_t> size() const { return _get()->size(); }

  std::string pythonRepr() const override {
    auto repr = DataObjectHandleBase::pythonRepr();
    boost::replace_all( repr, default_type, System::typeinfoName( typeid( View ) ) );
    return repr;
  }

private:
  AnyDataWithViewWrapper<View, Owned>* _get( bool mustExist ) const {
    auto obj = fetch();
    if ( !obj ) {
      if ( mustExist ) {
        throw GaudiException( "Cannot retrieve \'" + objKey() + "\' from transient store.",
                              m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );

      } else {
        return nullptr;
      }
    }
    if ( !m_goodType ) m_goodType = ::details::verifyType<AnyDataWithViewWrapper<View, Owned>>( obj );
    return static_cast<AnyDataWithViewWrapper<View, Owned>*>( obj );
  }
  mutable bool m_goodType = false;
};

//---------------------------- user-facing interface ----------
namespace details {
  template <typename T, typename U = T>
  using WriteHandle = DataObjectHandle<Payload_t<Gaudi::DataHandle::Writer, T, U>>;
  template <typename T, typename U = T>
  using ReadHandle = DataObjectHandle<Payload_t<Gaudi::DataHandle::Reader, T, U>>;
} // namespace details

template <typename T>
class DataObjectReadHandle : public ::details::ReadHandle<T> {
  template <typename... Args, std::size_t... Is>
  DataObjectReadHandle( std::tuple<Args...>&& args, std::index_sequence<Is...> )
      : DataObjectReadHandle( std::get<Is>( std::move( args ) )... ) {}

public:
  DataObjectReadHandle( const DataObjID& k, IDataHandleHolder* owner )
      : ::details::ReadHandle<T>{ k, Gaudi::DataHandle::Reader, owner } {}

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <typename OWNER, typename K, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
  DataObjectReadHandle( OWNER* owner, std::string propertyName, K key = {}, std::string doc = "" )
      : ::details::ReadHandle<T>( owner, Gaudi::DataHandle::Reader, std::move( propertyName ), std::move( key ),
                                  std::move( doc ) ) {}

  template <typename... Args>
  DataObjectReadHandle( std::tuple<Args...>&& args )
      : DataObjectReadHandle( std::move( args ), std::index_sequence_for<Args...>{} ) {}
};

template <typename T, typename U = T>
class DataObjectWriteHandle : public ::details::WriteHandle<T, U> {
  template <typename... Args, std::size_t... Is>
  DataObjectWriteHandle( std::tuple<Args...>&& args, std::index_sequence<Is...> )
      : DataObjectWriteHandle( std::get<Is>( std::move( args ) )... ) {}

public:
  DataObjectWriteHandle( const DataObjID& k, IDataHandleHolder* owner )
      : ::details::WriteHandle<T, U>{ k, Gaudi::DataHandle::Writer, owner } {}

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <typename OWNER, typename K, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
  DataObjectWriteHandle( OWNER* owner, std::string propertyName, K key = {}, std::string doc = "" )
      : ::details::WriteHandle<T, U>( owner, Gaudi::DataHandle::Writer, std::move( propertyName ), std::move( key ),
                                      std::move( doc ) ) {}

  template <typename... Args>
  DataObjectWriteHandle( std::tuple<Args...>&& args )
      : DataObjectWriteHandle( std::move( args ), std::index_sequence_for<Args...>{} ) {}
};
