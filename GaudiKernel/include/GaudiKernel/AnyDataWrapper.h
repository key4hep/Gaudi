/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <GaudiKernel/DataObject.h>
#include <cstddef>
#include <iterator>
#include <optional>
#include <typeindex>
#include <utility>

namespace details {
  using std::size;

  template <typename T, typename... Args>
  constexpr auto size( const T&, Args&&... ) noexcept {
    static_assert( sizeof...( Args ) == 0, "No extra args please" );
    return std::nullopt;
  }
} // namespace details

// ugly hack to circumvent the usage of std::any
struct GAUDI_API AnyDataWrapperBase : DataObject {
  virtual std::optional<std::size_t> size() const = 0;

  class Ptr {
    void const*     m_ptr  = nullptr;
    std::type_index m_type = typeid( void );

  public:
    template <typename T>
    Ptr( T const* t ) : m_ptr{ t }, m_type{ typeid( T ) } {}

    operator void const*() const { return m_ptr; }
    std::type_index type() const { return m_type; }

    template <typename T>
    T const* get() const {
      if ( std::is_void_v<T> || m_type == std::type_index( typeid( T ) ) ) return static_cast<T const*>( m_ptr );
      struct bad_AnyDataWrapper_Ptr_cast : std::bad_cast {};
      throw bad_AnyDataWrapper_Ptr_cast{};
    }
  };
  virtual Ptr payload() const = 0;
};

template <typename T>
class GAUDI_API AnyDataWrapper : public AnyDataWrapperBase {
protected:
  T m_data;

public:
  AnyDataWrapper( T&& data ) : m_data{ std::move( data ) } {};
  AnyDataWrapper( AnyDataWrapper&& )                 = delete;
  AnyDataWrapper( AnyDataWrapper const& )            = delete;
  AnyDataWrapper& operator=( AnyDataWrapper&& )      = delete;
  AnyDataWrapper& operator=( AnyDataWrapper const& ) = delete;

  const T& getData() const { return m_data; }
  T&       getData() { return m_data; }

  std::optional<std::size_t> size() const override {
    using ::details::size;
    return size( getData() );
  }

  Ptr payload() const override { return &m_data; }
};

template <typename ViewType, typename OwnedType>
class GAUDI_API AnyDataWithViewWrapper : public AnyDataWrapper<ViewType> {
  OwnedType m_owned;

public:
  AnyDataWithViewWrapper( OwnedType&& data ) : AnyDataWrapper<ViewType>{ {} }, m_owned{ std::move( data ) } {
    AnyDataWrapper<ViewType>::m_data = ViewType{ std::as_const( m_owned ) };
  }
  AnyDataWithViewWrapper( AnyDataWithViewWrapper&& )                 = delete;
  AnyDataWithViewWrapper( AnyDataWithViewWrapper const& )            = delete;
  AnyDataWithViewWrapper& operator=( AnyDataWithViewWrapper&& )      = delete;
  AnyDataWithViewWrapper& operator=( AnyDataWithViewWrapper const& ) = delete;
};
