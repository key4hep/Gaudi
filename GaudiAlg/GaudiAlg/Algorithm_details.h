#ifndef __Algorithm_details_h__
#define __Algorithm_details_h__

#include <type_traits>

namespace detail {

    // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

}

#endif
