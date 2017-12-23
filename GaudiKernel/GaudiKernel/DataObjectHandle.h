#ifndef GAUDIHIVE_DATAOBJECTHANDLE_H
#define GAUDIHIVE_DATAOBJECTHANDLE_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/NamedRange.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/ServiceLocatorHelper.h"

#include <type_traits>

//---------------------------------------------------------------------------
//
namespace details
{
  template <typename T>
  using Converter_t = T ( * )( const DataObject* );

  template <typename Range, typename StorageType>
  Range make_range( const DataObject* obj )
  {
    auto c = static_cast<const StorageType*>( obj );
    if ( UNLIKELY( !c ) ) return Range();
    using std::begin;
    using std::end;
    auto first  = begin( *c );
    auto last   = end( *c );
    // return Range( first, last );
    auto _first = reinterpret_cast<typename Range::const_iterator*>( &first );
    auto _last  = reinterpret_cast<typename Range::const_iterator*>( &last );
    return Range( *_first, *_last );
  }
  template <typename ValueType, typename Range = Gaudi::Range_<typename ValueType::ConstVector>>
  Converter_t<Range> select_range_converter( const DataObject* obj )
  {
    using Selection = typename ValueType::Selection;
    auto sel        = dynamic_cast<const Selection*>( obj );
    if ( sel ) return &make_range<Range, typename ValueType::Selection>;
    auto con = dynamic_cast<std::add_const_t<typename ValueType::Container>*>( obj );
    if ( con ) return &make_range<Range, typename ValueType::Container>;
    return nullptr;
  }
}

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
class DataObjectHandle : public DataObjectHandleBase
{
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
  T* getOrCreate();

  /**
   * Register object in transient store
   */
  T* put( T* object ) { return put( std::unique_ptr<T>(object) ); }
  T* put( std::unique_ptr<T> object );

private:
  T* get( bool mustExist ) const;
  mutable bool m_goodType = false;
};

template <typename T>
struct DataObjectReadHandle : public DataObjectHandle<T> {
  DataObjectReadHandle( const DataObjID& k, IDataHandleHolder* owner )
      : DataObjectHandle<T>{k, Gaudi::DataHandle::Reader, owner}
  {
  }
  DataObjectReadHandle( const std::string& k, IDataHandleHolder* owner ) : DataObjectReadHandle{DataObjID{k}, owner} {}

  DataObjectReadHandle( const DataObjectReadHandle& ) = delete;
  DataObjectReadHandle( DataObjectReadHandle&& )      = default;

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, class K, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
  inline DataObjectReadHandle( OWNER* owner, std::string name, const K& key = {}, std::string doc = "" )
      : DataObjectHandle<T>( owner, Gaudi::DataHandle::Reader, std::move( name ), key, std::move( doc ) )
  {
  }
};
template <typename T>
struct DataObjectWriteHandle : public DataObjectHandle<T> {
  DataObjectWriteHandle( const DataObjID& k, IDataHandleHolder* owner )
      : DataObjectHandle<T>{k, Gaudi::DataHandle::Writer, owner}
  {
  }
  DataObjectWriteHandle( const std::string& k, IDataHandleHolder* owner ) : DataObjectWriteHandle{DataObjID{k}, owner}
  {
  }

  DataObjectWriteHandle( const DataObjectWriteHandle& ) = delete;
  DataObjectWriteHandle( DataObjectWriteHandle&& )      = default;

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, class K, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
  inline DataObjectWriteHandle( OWNER* owner, std::string name, const K& key = {}, std::string doc = "" )
      : DataObjectHandle<T>( owner, Gaudi::DataHandle::Writer, std::move( name ), key, std::move( doc ) )
  {
  }
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
T* DataObjectHandle<T>::get( bool mustExist ) const
{

  auto dataObj = fetch();

  if ( UNLIKELY( !dataObj ) ) {
    if ( mustExist ) { // Problems in getting from the store
      throw GaudiException( "Cannot retrieve " + objKey() + " from transient store.",
                            m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
    }
    return nullptr;
  }

  if ( UNLIKELY( !m_goodType ) ) { // Check type compatibility once
    T* obj     = dynamic_cast<T*>( dataObj );
    m_goodType = ( obj != nullptr );
    if ( UNLIKELY( !m_goodType ) ) {

      std::string errorMsg( "The type provided for " + objKey() + " is " + System::typeinfoName( typeid( T ) ) +
                            " and is different from the one of the object in the store." );
      throw GaudiException( errorMsg, "Wrong DataObjectType", StatusCode::FAILURE );
    }
    assert( obj == static_cast<T*>( dataObj ) );
    return obj;
  }

  // From the second read on, this is safe
  return static_cast<T*>( dataObj );
}

//---------------------------------------------------------------------------
template <typename T>
T* DataObjectHandle<T>::put( std::unique_ptr<T> objectp )
{
  assert( m_init );
  StatusCode rc = m_EDS->registerObject( objKey(), objectp.get() );
  if ( !rc.isSuccess() ) {
    throw GaudiException( "Error in put of " + objKey(), "DataObjectHandle<T>::put", StatusCode::FAILURE );
  }
  return objectp.release();
}

//---------------------------------------------------------------------------
template <typename T>
T* DataObjectHandle<T>::getOrCreate()
{
  T* obj = get( false );
  return obj ? obj : put( new T{} );
}

//---------------------------------------------------------------------------

/// specialization for Range_

template <typename T>
class DataObjectHandle<Gaudi::Range_<T>> : public DataObjectHandleBase
{
public:
  using ValueType = std::remove_cv_t<std::remove_pointer_t<typename T::value_type>>;
  using Range     = Gaudi::Range_<typename ValueType::ConstVector>;

  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  Range get() const;

private:
  mutable details::Converter_t<Range> m_converter = nullptr;
};

template <typename ValueType>
auto DataObjectHandle<Gaudi::Range_<ValueType>>::get() const -> Range
{
  auto dataObj = fetch();
  if ( UNLIKELY( !dataObj ) ) {
    throw GaudiException( "Cannot retrieve " + objKey() + " from transient store.",
                          m_owner ? owner()->name() : "no owner", StatusCode::FAILURE );
  }
  if ( UNLIKELY( m_converter == nullptr ) ) {
    m_converter = details::select_range_converter<ValueType>( dataObj );
    if ( !m_converter ) {
      throw GaudiException( "The type requested for " + objKey() + " cannot be obtained from object in event store",
                            "Wrong DataObjectType", StatusCode::FAILURE );
    }
  }
  return ( *m_converter )( dataObj );
}

#endif
