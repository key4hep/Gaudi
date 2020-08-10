/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_COUNTERHOLDER_H
#define GAUDIKERNEL_COUNTERHOLDER_H

#include <Gaudi/Accumulators.h>
#include <algorithm>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>

#include <Gaudi/MonitoringHub.h>

class INamedInterface;

template <class BASE>
class GAUDI_API CounterHolder : public BASE {
  static_assert( std::is_base_of_v<INamedInterface, BASE>,
                 "CounterHolder template argument must inherit from INamedInterface" );

public:
  using BASE::BASE;

  void declareCounter( std::string tag, Gaudi::Accumulators::PrintableCounter& r ) {
    auto lock = std::scoped_lock{m_mutex};
    m_counters.emplace( tag, r );
    this->serviceLocator()->monitoringHub().registerEntity( {this->name() + '/' + std::move( tag ), r} );
  }

  const Gaudi::Accumulators::PrintableCounter* findCounter( std::string_view tag ) const {
    auto lock = std::scoped_lock{m_mutex};
    auto p    = m_counters.find( tag );
    return p != m_counters.end() ? &p->second.get() : nullptr;
  }

  template <typename Callable>
  void forEachCounter( Callable&& f ) const {
    auto lock = std::scoped_lock{m_mutex};
    std::for_each( m_counters.begin(), m_counters.end(),
                   [f = std::forward<Callable>( f )]( const auto& p ) { std::invoke( f, p.first, p.second.get() ); } );
  }

  int nCounters() const {
    auto lock = std::scoped_lock{m_mutex};
    return m_counters.size();
  }

  std::size_t nOfCountersToBePrinted() {
    auto lock = std::scoped_lock{m_mutex};
    return count_if( begin( m_counters ), end( m_counters ),
                     []( const auto& c ) { return c.second.get().toBePrinted(); } );
  }

  void clearCounters() {
    auto lock = std::scoped_lock{m_mutex};
    m_counters.clear();
  }

private:
  std::map<std::string, std::reference_wrapper<Gaudi::Accumulators::PrintableCounter>, std::less<>> m_counters;
  mutable std::mutex                                                                                m_mutex;
};
#endif
