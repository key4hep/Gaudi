#ifndef GAUDIKERNEL_COUNTERHOLDER_H
#define GAUDIKERNEL_COUNTERHOLDER_H

#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>

#include "GaudiKernel/Counters.h"
#include "GaudiKernel/invoke.h"

template <class BASE>
class GaudiCommon;
class INamedInterface;
class Algorithm;
class AlgTool;
namespace Gaudi {
  class Sequence;
}

template <class BASE>
class GAUDI_API CounterHolder : public BASE {
  static_assert( std::is_base_of<INamedInterface, BASE>::value,
                 "CounterHolder template argument must inherit from INamedInterface" );

public:
  using BASE::BASE;

  void declareCounter( const std::string& tag, Gaudi::Accumulators::PrintableCounter& r ) {
    std::lock_guard<std::mutex> lock( m_countersMutex );
    m_counters.emplace( tag, r );
  }

  const Gaudi::Accumulators::PrintableCounter* findCounter( const std::string& tag ) const {
    std::lock_guard<std::mutex> lock( m_countersMutex );
    auto                        p = this->m_counters.find( tag );
    return p != m_counters.end() ? &p->second.get() : nullptr;
  }

  template <typename Callable>
  void forEachCounter( Callable&& f ) const {
    std::lock_guard<std::mutex> lock( m_countersMutex );
    std::for_each( m_counters.begin(), m_counters.end(), [f = std::forward<Callable>( f )]( const auto& p ) {
      Gaudi::invoke( f, p.first, p.second.get() );
    } );
  }

  int nCounters() const {
    std::lock_guard<std::mutex> lock( m_countersMutex );
    return m_counters.size();
  }

  void clearCounters() {
    std::lock_guard<std::mutex> lock( m_countersMutex );
    m_counters.clear();
  }

private:
  std::map<std::string, std::reference_wrapper<Gaudi::Accumulators::PrintableCounter>> m_counters;
  mutable std::mutex                                                                   m_countersMutex;
};
#endif
