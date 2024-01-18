/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/SerializeSTL.h>
#include <boost/algorithm/string/join.hpp>
#include <initializer_list>
#include <string>

namespace Gaudi::Functional {


  namespace Traits {

    // traits classes used to customize Transformer and FilterPredicate
    // Define the types to to be used as baseclass, and as in- resp. output hanldes.
    // In case a type is not specified in the traits struct, a default is used.
    //
    // The defaults are:
    //
    //      using BaseClass = GaudiAlgorithm
    //      template <typename T> using InputHandle = DataObjectHandle<T>;
    //      template <typename T> using OutputHandle = DataObjectHandle<T>;
    //

    // the best way to 'compose' traits is by inheriting them one-by-one...
    template <typename... Base>
    struct use_ : Base... {};

    // helper classes one can inherit from to specify a specific trait
    template <typename Base>
    struct BaseClass_t {
      using BaseClass = Base;
    };

    template <template <typename> class Handle>
    struct InputHandle_t {
      template <typename T>
      using InputHandle = Handle<T>;
    };

    template <template <typename> class Handle>
    struct OutputHandle_t {
      template <typename T>
      using OutputHandle = Handle<T>;
    };

    template <typename Data, typename View>
    struct writeViewFor {
      template <typename T>
      using OutputHandle = std::enable_if_t<std::is_same_v<T, Data>, DataObjectWriteHandle<View, Data>>;
    };

    // add support for objects that should reside in the TES for lifetime management, but should not
    // be used explicitly and/or directly by downstream code.
    template <typename Data>
    struct WriteOpaqueFor {
      struct OpaqueView {
        OpaqueView() = default;
        template <typename T>
        OpaqueView( T const& ) {}
      };

      template <typename T>
      using OutputHandle = std::enable_if_t<std::is_same_v<T, Data>, DataObjectWriteHandle<OpaqueView, Data>>;
    };

    // this uses the defaults -- and it itself is the default ;-)
    using useDefaults = use_<>;
  } // namespace Traits
} // namespace Gaudi::Functional
