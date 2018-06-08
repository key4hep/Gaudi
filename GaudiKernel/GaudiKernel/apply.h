#ifndef GAUDIKERNEL_APPLY_H
#define GAUDIKERNEL_APPLY_H
#include <tuple>

#if __cplusplus > 201402L
namespace Gaudi
{
  using std::apply;
}
#else

#include "GaudiKernel/invoke.h"
namespace Gaudi
{
  namespace detail
  {
    template <class F, class Tuple, std::size_t... I>
    constexpr decltype( auto ) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> ) noexcept(
        noexcept( Gaudi::invoke( std::forward<F>( f ), std::get<I>( std::forward<Tuple>( t ) )... ) ) )
    {
      return Gaudi::invoke( std::forward<F>( f ), std::get<I>( std::forward<Tuple>( t ) )... );
    }
  } // namespace detail

  //
  // implementation of std::apply -- see http://en.cppreference.com/w/cpp/utility/apply --
  // Invokes the Callable object f with a tuple of arguments, i.e.
  // returns f( std::get<0>(t), std::get<1>(t), ... )
  //
  template <class F, class Tuple>
  constexpr decltype( auto ) apply( F&& f, Tuple&& t ) noexcept( noexcept(
      detail::apply_impl( std::forward<F>( f ), std::forward<Tuple>( t ),
                          std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{} ) ) )
  {
    return detail::apply_impl( std::forward<F>( f ), std::forward<Tuple>( t ),
                               std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{} );
  }
}
#endif
#endif
