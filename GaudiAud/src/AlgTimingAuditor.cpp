/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/Auditor.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/SmartIF.h>
#include <chrono>
#include <fmt/format.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

struct AlgTimingAuditor final : extends<Auditor, IIncidentListener> {
  using base_class::base_class;

  using Auditor::after;
  using Auditor::before;

  StatusCode initialize() override {
    return base_class::initialize().andThen( [&]() {
      m_incSvc = service( "IncidentSvc" );
      if ( m_incSvc ) {
        m_incSvc->addListener( this, IncidentType::BeginEvent );
        m_incSvc->addListener( this, IncidentType::EndEvent );
        ++m_currentDepth;
      } else {
        debug() << "no IncidentSvc, I cannot measure overall event processing time" << endmsg;
      }
    } );
  }

  void before( StandardEventType evt, INamedInterface* alg ) override {
    if ( !alg ) return;
    switch ( evt ) {
    case Initialize:
      stats( alg ); // this implicitly adds the algorithm to the list of known ones
      ++m_currentDepth;
      break;

    case Execute:
      stats( alg ).start();
      break;

    default:
      break;
    }
  }

  void after( StandardEventType evt, INamedInterface* alg, const StatusCode& ) override {
    if ( !alg ) return;
    switch ( evt ) {
    case Initialize:
      --m_currentDepth;
      break;

    case Execute:
      stats( alg ).stop();
      break;

    default:
      break;
    }
  }

  void handle( const Incident& i ) override {
    if ( IncidentType::BeginEvent == i.type() ) {
      m_eventLoopStats.start();
    } else if ( IncidentType::EndEvent == i.type() ) {
      m_eventLoopStats.stop();
    }
  }

  StatusCode finalize() override {
    using ms = std::chrono::duration<float, std::milli>;
    using s  = std::chrono::duration<float>;
    info() << "-------------------------------------------------------------------" << endmsg;
    info() << "Algorithm                      | exec (ms) |   count   | total (s)" << endmsg;
    info() << "-------------------------------------------------------------------" << endmsg;
    if ( m_incSvc ) {
      const auto count      = m_eventLoopStats.count;
      const auto total_time = m_eventLoopStats.total_time;
      info() << fmt::format( "{:<30.30} | {:9.4} | {:9} | {:9.4}", "EVENT LOOP",
                             count ? ms( total_time ).count() / count : 0.f, count, s( total_time ).count() )
             << endmsg;
    }
    std::vector<std::pair<std::string, std::size_t>> offsets{ begin( m_offsets ), end( m_offsets ) };
    std::sort( begin( offsets ), end( offsets ), []( auto& a, auto& b ) { return a.second < b.second; } );
    for ( const auto& [name, offset] : offsets ) {
      std::string indented_name = std::string( m_depths[offset], ' ' ) + name;
      const auto  count         = m_stats[offset].count;
      const auto  total_time    = m_stats[offset].total_time;
      info() << fmt::format( "{:<30.30} | {:9.4} | {:9} | {:9.4}", indented_name,
                             count ? ms( total_time ).count() / count : 0.f, count, s( total_time ).count() )
             << endmsg;
    }
    info() << "-------------------------------------------------------------------" << endmsg;

    if ( m_incSvc ) {
      m_incSvc->removeListener( this, IncidentType::BeginEvent );
      m_incSvc->removeListener( this, IncidentType::EndEvent );
      m_incSvc.reset();
    }
    return base_class::finalize();
  }

  using clock_t = std::conditional_t<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock,
                                     std::chrono::steady_clock>;

  struct stats_t {
    clock_t::duration   total_time{};
    std::size_t         count{};
    clock_t::time_point started{};

    void start() { started = clock_t::now(); }
    void stop() {
      total_time += clock_t::now() - started;
      ++count;
    }
  };

  stats_t& stats( INamedInterface* alg ) {
    if ( auto it = m_offsets.find( alg->name() ); it != end( m_offsets ) ) {
      return m_stats[it->second];
    } else {
      m_depths.push_back( m_currentDepth );
      m_stats.emplace_back();
      m_offsets[alg->name()] = m_stats.size() - 1;
      return m_stats.back();
    }
  }

  SmartIF<IIncidentSvc> m_incSvc;

  std::unordered_map<std::string, std::size_t> m_offsets;
  std::vector<std::uint16_t>                   m_depths;
  std::vector<stats_t>                         m_stats;
  stats_t                                      m_eventLoopStats;
  std::uint16_t                                m_currentDepth{ 0 };
};

DECLARE_COMPONENT( AlgTimingAuditor )
