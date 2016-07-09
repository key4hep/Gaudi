#ifndef __Functional_details_h__
#define __Functional_details_h__

#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"

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

    template <std::size_t N, typename Tuple >
    using Out_t = typename std::tuple_element<N, Tuple>::type;

    template <typename... Handles, typename KeyValues, std::size_t... I>
    auto make_tuple_of_handles_helper( IDataHandleHolder* o, const KeyValues& initvalue, Gaudi::DataHandle::Mode m, std::index_sequence<I...> ) {
        return std::make_tuple( Handles(std::get<I>(initvalue).second, m, o) ... );
    }
    template <typename... Handles, typename KeyValues >
    auto make_tuple_of_handles( IDataHandleHolder* owner, const KeyValues& initvalue, Gaudi::DataHandle::Mode mode ) {
        return make_tuple_of_handles_helper<Handles...>( owner, initvalue, mode, std::make_index_sequence<sizeof...(Handles)>{} );
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
