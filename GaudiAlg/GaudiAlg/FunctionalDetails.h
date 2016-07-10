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

    // if the traits class defines the relevant type, use it, otherwise pick up
    // the default
    //
    template <typename> struct void_t { typedef void type; };
    template <typename T> using is_valid_t = typename void_t<T>::type;

    // check whether Traits::BaseClass is a valid type, if not, define
    // it as GaudiAlgorithm
    template <typename Tr, typename SFINAE = void> struct has_base                 : std::false_type {};
    template <typename Tr> struct has_base<Tr, is_valid_t<typename Tr::BaseClass>> : std::true_type  {};
    template <typename Tr, bool> struct add_Base;
    template <typename Tr>       struct add_Base<Tr, true> : Tr { };
    template <typename Tr>       struct add_Base<Tr,false> : Tr { using BaseClass = GaudiAlgorithm; };
    template <typename Traits> using BaseClass_t = typename add_Base<Traits,has_base<Traits>::value>::BaseClass;
    // now one can do eg.:
    // BaseClass_t<Traits>
    // which is either Traits::BaseClass in case it is defined, else GaudiAlgorithm


    // check whether Traits::OutputHandle<T> is a valid type, if not, define
    // it as DataObjectHandle<T>
    template <typename Tr, typename T, typename SFINAE = void> struct has_out_handle                 : std::false_type {};
    template <typename Tr, typename T> struct has_out_handle<Tr, T, is_valid_t<typename Tr::template OutputHandle<T>>> : std::true_type  {};
    template <typename Tr, bool> struct add_out_handle;
    template <typename Tr>       struct add_out_handle<Tr, true> : Tr { };
    template <typename Tr>       struct add_out_handle<Tr,false> : Tr { template<typename T> using OutputHandle = DataObjectHandle<T>; };
    template <typename Tr, typename T> using OutputHandle_t = typename add_out_handle<Tr,has_out_handle<Tr,T>::value>::template OutputHandle<T>;
    // now one can do eg.:
    // OutputHandle_t<Traits,T>
    // which is either Traits::OutputHandle<T> in case it is defined, else DataObjectHandle<T>

    // check whether Traits::InputHandle<T> is a valid type, if not, define
    // it as DataObjectHandle<T>
    template <typename Tr, typename T, typename SFINAE = void> struct has_in_handle                 : std::false_type {};
    template <typename Tr, typename T> struct has_in_handle<Tr, T, is_valid_t<typename Tr::template InputHandle<T>>> : std::true_type  {};
    template <typename Tr, bool> struct add_in_handle;
    template <typename Tr >      struct add_in_handle<Tr, true> : Tr { };
    template <typename Tr >      struct add_in_handle<Tr,false> : Tr { template <typename T> using InputHandle = DataObjectHandle<T>; };
    template <typename Tr, typename T> using InputHandle_t = typename add_in_handle<Tr,has_in_handle<Tr,T>::value>::template InputHandle<T>;
    // now one can do eg.:
    // InputHandle_t<Traits,T>
    // which is either Traits::InputHandle<T> in case it is defined, else DataObjectHandle<T>

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
