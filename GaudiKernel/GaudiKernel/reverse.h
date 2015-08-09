//
// provide a generic 'reverse' function for use in range-based for loops.
//
// example:
//
//      #include "GaudiKernel/reverse.h"
//      auto l = { 1,2,3,4 };
//      for ( const auto& i : reverse( l ) ) std::cout << i << std::endl;
//
//  Note that it is perfectly fine to reverse a temporary. The temporary
//  will be moved into the reverse_wrapper, and thus kept alive for the
//  duration of the loop. In case reverse is called on an lvalue, the wrapper
//  will take a reference, so no copy will be performed. (if you wonder how
//  the code below (very implicitly) distinguishes between the two cases,
//  google for 'C++11 reference collapsing' -- short version: C++ does not
//  allow references to references, so in cases where this would happen,
//  one gets an lvalue reference)
//
//  Also note that reverse_wrapper does not have a constructor, but this
//  implies that it does meet the requirements for aggregate initializaton,
//  which allows for {} initialization of its member.
//
#if defined __GNUC__ &&  __GNUC__ < 5
// std::rbegin and std::rend require gcc 5.0 or later (and clang 3.5 or later)
template <typename C>
auto rbegin(C& c) { return c.rbegin(); }
template <typename C>
auto rend(C& c) { return c.rend(); }
#else
#include <iterator>
#endif

template <typename Iterable>
struct reverse_wrapper { Iterable iterable; };

template <typename T>
auto begin(reverse_wrapper<T>& w) { using namespace std; return rbegin(w.iterable); }
template <typename T>
auto end(reverse_wrapper<T>& w)   { using namespace std; return rend(w.iterable); }

template <typename T>
reverse_wrapper<T> reverse(T&& iterable) { return { std::forward<T>(iterable) }; }
