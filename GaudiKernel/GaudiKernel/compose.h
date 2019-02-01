#ifndef GAUDIKERNEL_COMPOSE_H
#define GAUDIKERNEL_COMPOSE_H

#include <utility> // std::forward, std::move (objects)

namespace Gaudi {

  namespace details {

#if __cplusplus > 201402L

    // C++17 version: https://godbolt.org/g/YdcvGg
    template <typename... lambda_ts>
    struct overloaded_t : lambda_ts... {
      using lambda_ts::operator()...;
    };
    template <typename... lambda_ts>
    overloaded_t( lambda_ts... )->overloaded_t<lambda_ts...>;

#else

    template <typename... lambda_ts>
    struct overloaded_t;

    template <typename lambda_t>
    struct overloaded_t<lambda_t> : lambda_t {
      overloaded_t( const lambda_t& lambda ) : lambda_t{lambda} {}
      overloaded_t( lambda_t&& lambda ) : lambda_t{std::move( lambda )} {}

      using lambda_t::operator();
    };

    template <typename lambda_t, typename... more_lambda_ts>
    struct overloaded_t<lambda_t, more_lambda_ts...> : lambda_t, overloaded_t<more_lambda_ts...> {
      using super_t = overloaded_t<more_lambda_ts...>;

      template <typename... lambda_ts>
      overloaded_t( const lambda_t& lambda, lambda_ts&&... more_lambdas )
          : lambda_t{lambda}, super_t{std::forward<lambda_ts>( more_lambdas )...} {}
      template <typename... lambda_ts>
      overloaded_t( lambda_t&& lambda, lambda_ts&&... more_lambdas )
          : lambda_t{std::move( lambda )}, super_t{std::forward<lambda_ts>( more_lambdas )...} {}

      using lambda_t::operator();
      using super_t:: operator();
    };
#endif
  } // namespace details

  //
  // Create an object with an overloaded call operator by 'composing'/'joining'
  // a set of callables (such as lambdas)
  //
  // see eg. the example at http://en.cppreference.com/w/cpp/utility/variant/visit
  // for an example of why this is usefull
  //
  template <typename... lambda_ts>
  auto overload( lambda_ts&&... lambdas ) {
#if __cplusplus > 201402L
    return details::overloaded_t{std::forward<lambda_ts>( lambdas )...};
#else
    return details::overloaded_t<std::decay_t<lambda_ts>...>{std::forward<lambda_ts>( lambdas )...};
#endif
  }
} // namespace Gaudi

// for backwards compatibility
// [[deprecated("please use Gaudi::overload instead of compose")]]
template <typename... lambda_ts>
auto compose( lambda_ts&&... lambdas ) {
  return Gaudi::overload( std::forward<lambda_ts>( lambdas )... );
}

#endif
