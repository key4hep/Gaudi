#ifndef GAUDIKERNEL_COUNTERHOLDER_H
#define GAUDIKERNEL_COUNTERHOLDER_H

#include <algorithm>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>

#include "GaudiKernel/Counters.h"

class INamedInterface;

template <class BASE>
class GAUDI_API CounterHolder : public BASE {
  static_assert( std::is_base_of_v<INamedInterface, BASE>,
                 "CounterHolder template argument must inherit from INamedInterface" );

public:
  using BASE::BASE;

  void declareCounter( const std::string& tag, Gaudi::Accumulators::PrintableCounter& r ) {
    std::lock_guard lock{m_mutex};
    m_counters.emplace( tag, r );
  }

  const Gaudi::Accumulators::PrintableCounter* findCounter( const std::string& tag ) const {
    std::lock_guard lock{m_mutex};
    auto            p = m_counters.find( tag );
    return p != m_counters.end() ? &p->second.get() : nullptr;
  }

  template <typename Callable>
  void forEachCounter( Callable&& f ) const {
    std::lock_guard lock{m_mutex};
    std::for_each( m_counters.begin(), m_counters.end(),
                   [f = std::forward<Callable>( f )]( const auto& p ) { std::invoke( f, p.first, p.second.get() ); } );
  }

  int nCounters() const {
    std::lock_guard lock{m_mutex};
    return m_counters.size();
  }

  std::size_t nOfCountersToBePrinted() {
    std::lock_guard lock{m_mutex};
    return std::accumulate( begin( m_counters ), end( m_counters ), std::size_t{0},
                            []( std::size_t cnt, const auto& c ) -> std::size_t {
                              return cnt + ( c.second.get().toBePrinted() ? 1 : 0 );
                            } );
  }

  void clearCounters() {
    std::lock_guard lock{m_mutex};
    m_counters.clear();
  }

private:
  std::map<std::string, std::reference_wrapper<Gaudi::Accumulators::PrintableCounter>> m_counters;
  mutable std::mutex                                                                   m_mutex;
};
#endif
