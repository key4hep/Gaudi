/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Consumer.h>
#include <deque>
#include <fmt/format.h>
#include <mutex>

/// Simple algorithm illustrating the usage of different "counters"
struct CounterAlg : Gaudi::Functional::Consumer<void()> {

  using Gaudi::Functional::Consumer<void()>::Consumer;

  void operator()() const override {
    // update all counters by some fixed values
    ++basic;
    avg += 1.5;
    sig += 2.5;
    stat += 3.5;
    binomial += true;
    ++msg;
    avg_int += 1;
    avg_noAto += 1.5;
    // update counters in the deque, creating new ones for the first 20 events
    // and dropping first 10 in the next 10 events
    {
      // needs to be protected by a mutex in case of multithreaded usage as the
      // deque is not thread safe
      std::scoped_lock lock( counterDequeMutex );
      if ( counterCount < 20 ) {
        counterDeque.emplace_back( this, fmt::format( "DQCounter{}", counterDeque.size() ) );
        ++counterCount;
      }
      if ( counterCount == 20 && counterDeque.size() > 10 ) { counterDeque.pop_front(); }
      for ( auto& c : counterDeque ) ++c;
    }
  }

  // declare all sorts of counters with default options (double values, atomicity full)
  mutable Gaudi::Accumulators::Counter<>             basic{ this, "Basic" };
  mutable Gaudi::Accumulators::AveragingCounter<>    avg{ this, "Average" };
  mutable Gaudi::Accumulators::SigmaCounter<>        sig{ this, "Sigma" };
  mutable Gaudi::Accumulators::StatCounter<>         stat{ this, "Stat" };
  mutable Gaudi::Accumulators::BinomialCounter<>     binomial{ this, "Binomial" };
  mutable Gaudi::Accumulators::MsgCounter<MSG::INFO> msg{ this, "Super nice message, max 5 times", 5 };

  // test change of some template parameters
  mutable Gaudi::Accumulators::AveragingCounter<unsigned int> avg_int{ this, "AverageInteger" };
  mutable Gaudi::Accumulators::AveragingCounter<double, Gaudi::Accumulators::atomicity::none> avg_noAto{
      this, "AverageNonAtomic" };

  // test set of counters stored in a deque
  mutable int                                        counterCount{ 0 };
  mutable std::deque<Gaudi::Accumulators::Counter<>> counterDeque;
  mutable std::mutex                                 counterDequeMutex;
};

DECLARE_COMPONENT( CounterAlg )
