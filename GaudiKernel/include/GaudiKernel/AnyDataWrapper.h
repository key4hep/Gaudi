/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include "GaudiKernel/DataObject.h"
#include <cstddef>
#include <iterator>
#include <optional>
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
};

template <typename T>
class GAUDI_API AnyDataWrapper : public AnyDataWrapperBase {
protected:
  T m_data;

public:
  AnyDataWrapper( T&& data ) : m_data{ std::move( data ) } {};
  AnyDataWrapper( AnyDataWrapper&& )      = delete;
  AnyDataWrapper( AnyDataWrapper const& ) = delete;
  AnyDataWrapper& operator=( AnyDataWrapper&& ) = delete;
  AnyDataWrapper& operator=( AnyDataWrapper const& ) = delete;

  const T& getData() const { return m_data; }
  T&       getData() { return m_data; }

  std::optional<std::size_t> size() const override {
    using ::details::size;
    return size( getData() );
  }
};

template <typename ViewType, typename OwnedType>
class GAUDI_API AnyDataWithViewWrapper : public AnyDataWrapper<ViewType> {
  OwnedType m_owned;

public:
  AnyDataWithViewWrapper( OwnedType&& data ) : AnyDataWrapper<ViewType>{ {} }, m_owned{ std::move( data ) } {
    AnyDataWrapper<ViewType>::m_data = ViewType{ std::as_const( m_owned ) };
  }
  AnyDataWithViewWrapper( AnyDataWithViewWrapper&& )      = delete;
  AnyDataWithViewWrapper( AnyDataWithViewWrapper const& ) = delete;
  AnyDataWithViewWrapper& operator=( AnyDataWithViewWrapper&& ) = delete;
  AnyDataWithViewWrapper& operator=( AnyDataWithViewWrapper const& ) = delete;
};
