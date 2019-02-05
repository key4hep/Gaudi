#include <utility>
//
//  make it possible to execute an action at the exit of a scope
//
//  auto f = finally( [](){ std::cout << "end of scope!" << std::endl; } );
//
//  the above will execute the provided callable when f goes out of scope,
//  i.e. the 'current' scope ends.

template <typename F>
struct final_action {
  F act;
  final_action( final_action&& ) = default;
  ~final_action() { act(); }
};

template <typename F>
final_action<F> finally( F&& act ) {
  return {std::forward<F>( act )};
}
