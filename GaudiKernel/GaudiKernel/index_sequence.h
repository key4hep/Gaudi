#ifndef __index_sequence_h__
#define __index_sequence_h__
#include <cstddef>
// C++14 Compile-time integer sequences -- this can go once we use C++14 only...
// #include <utility> // defines (in C++14) std::make_index_sequence and std::index_sequence
namespace utility {

template<std::size_t... indexes> struct index_sequence {
    static std::size_t size() { return sizeof...(indexes); }
};

namespace detail {
template<std::size_t currentIndex, std::size_t...indexes> struct make_index_sequence_helper;

template<std::size_t...indexes> struct make_index_sequence_helper<0, indexes...> {
    typedef index_sequence<indexes...> type;
};

template<std::size_t currentIndex, std::size_t...indexes> struct make_index_sequence_helper {
    typedef typename make_index_sequence_helper<currentIndex - 1, currentIndex - 1, indexes...>::type type;
};

template<std::size_t N> struct make_index_sequence : public make_index_sequence_helper<N>::type { };

} // namespace detail
using detail::make_index_sequence;
} // namespace utility
#endif
