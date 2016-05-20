#ifndef __Algorithm_details_h__
#define __Algorithm_details_h__

#include <type_traits>
#include <utility>
#include <initializer_list>

namespace detail {

    // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

    // helper to check for absentees amongst a list of (named) inputs
    template< template <typename> class Container = std::initializer_list>
    std::vector<int> awol(Container<void*> inputs) {
        std::vector<int> result;
        int i=0;
        for ( const auto& j : inputs ) {
            if (!j) result.push_back(i);
            ++i;
        }
        return result;
    }
}

#endif
