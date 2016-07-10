#ifndef __Functional_details_h__
#define __Functional_details_h__

#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

namespace Gaudi { namespace Functional { namespace details {

    // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

    //// implementation details:

    //TODO: require that Out2 is convertible to Out1?
    template <typename Out1, typename Out2>
    void put( DataObjectHandle<Out1>& out_handle, Out2&& out ) {
        out_handle.put( new Out1( std::forward<Out2>(out) ) );
    }

    //TODO: require that Out2 is convertible to Out1?
    template <typename Out1, typename Out2>
    void put( AnyDataHandle<Out1>& out_handle, Out2&& out ) {
        out_handle.put( std::forward<Out2>(out) );
    }

    // detect whether a traits class defines the requested type,
    //   if so, use it,
    //   otherwise use the default
    //
    // based on http://en.cppreference.com/w/cpp/experimental/is_detected
    // and the libstdc++ source, libstdc++-v3/include/std/type_traits

    namespace detail2 { 

        /// Implementation of the detection idiom (negative case).
        template<typename Default, typename AlwaysVoid,
                 template<typename...> class Op, typename... Args>
        struct detector {
            using type = Default;
        };

#ifdef HAVE_CPP17
        template<typename...> using void_t = void;

        /// Implementation of the detection idiom (positive case).
        template<typename Default,
                 template<typename...> class Op, typename... Args>
        struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
            using type = Op<Args...>;
        };

#else
        template <typename> struct void_t { typedef void type; };
        template <typename T> using is_valid_t = typename void_t<T>::type;

        /// Implementation of the detection idiom (positive case).
        template<typename Default,
                 template<typename...> class Op, typename... Args>
        struct detector<Default, is_valid_t<Op<Args...>>, Op, Args...> {
            using type = Op<Args...>;
        };

#endif
    }

    // Detect whether Op<_Args...> is a valid type, use Default if not.
    template<typename Default, template<typename...> class Op, typename... Args>
    using detected_or = detail2::detector<Default, void, Op, Args...>;

    // Op<Args...> if that is a valid type, otherwise Default.
    template<typename Default, template<typename...> class Op, typename... Args>
    using detected_or_t = typename detected_or<Default, Op, Args...>::type;

     // Op<Args...> if that is a valid type, otherwise Default<Args...>.
    template<template<typename...> class Default, template<typename...> class Op, typename... Args>
    using detected_or_t_ = detected_or_t<Default<Args...>, Op, Args...>;

    ///////////////

    // check whether Traits::BaseClass is a valid type,
    // if so, define BaseClass_t<Traits> as being Traits::BaseClass
    // else   define BaseClass_t<Traits> as being GaudiAlgorithm
    template <typename Tr> using BaseClass_  = typename Tr::BaseClass;
    template <typename Tr> using BaseClass_t = detected_or_t< GaudiAlgorithm, BaseClass_, Tr >;

    // check whether Traits::{Input,Output}Handle<T> is a valid type,
    // if so, define {Input,Output}Handle_t<Traits,T> as being Traits::{Input,Output}Handle<T>
    // else   define {Input,Output}Handle_t<Traits,T> as being DataHandle<T>
    template <typename Tr, typename T> using DataObjectHandle_ = DataObjectHandle<T>;
    template <typename Tr, typename T> using OutputHandle_  = typename Tr::template OutputHandle<T>;
    template <typename Tr, typename T> using InputHandle_   = typename Tr::template InputHandle<T>;

    template <typename Tr, typename T> using OutputHandle_t = detected_or_t_< DataObjectHandle_, OutputHandle_, Tr, T>;
    template <typename Tr, typename T> using InputHandle_t  = detected_or_t_< DataObjectHandle_, InputHandle_,  Tr, T>;

    /////////

    template <std::size_t N, typename Tuple >
    using element_t = typename std::tuple_element<N, Tuple>::type;

    template <typename Tuple, typename KeyValues, std::size_t... I>
    Tuple make_tuple_of_handles_helper( IDataHandleHolder* o, const KeyValues& initvalue, Gaudi::DataHandle::Mode m, std::index_sequence<I...> ) {
        return std::make_tuple( element_t<I,Tuple>{std::get<I>(initvalue).second, m, o} ... );
    }
    template <typename Tuple, typename KeyValues >
    Tuple make_tuple_of_handles( IDataHandleHolder* owner, const KeyValues& initvalue, Gaudi::DataHandle::Mode mode ) {
        return make_tuple_of_handles_helper<Tuple>( owner, initvalue, mode, std::make_index_sequence<std::tuple_size<Tuple>::value>{} );
    }

    template <typename KeyValues, typename Properties,  std::size_t... I>
    void declare_tuple_of_properties_helper(Algorithm* owner, const KeyValues& inputs, Properties& props,  std::index_sequence<I...>) {
        std::initializer_list<int>{
            (owner->declareProperty( std::get<I>(inputs).first,
                                     std::get<I>(props)         ),0)...
        };
    }

    template <typename KeyValues, typename Properties>
    void declare_tuple_of_properties(Algorithm* owner, const KeyValues& inputs, Properties& props)
    {
        static_assert( std::tuple_size<KeyValues>::value == std::tuple_size<Properties>::value, "Inconsistent lengths" );
        constexpr auto N = std::tuple_size<KeyValues>::value;
        declare_tuple_of_properties_helper( owner, inputs, props, std::make_index_sequence<N>{} );
    }


} } }

#endif
