/***********************************************************************************\
* (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Accumulators.h>
#include <utility>

namespace Gaudi::Accumulators {

  namespace details {

    /**
     * Default formating for counter names, only calling fmt::format
     * on the text given at construction and passing the histo index as argument
     */
    struct FormatCounterDefault {
      std::string_view text;
      FormatCounterDefault( std::string_view t ) : text{ t } {}
      std::string operator()( size_t n );
    };

    /**
     * internal class implementing an array of counters
     * @see CounterArray
     */
    template <typename Counter, std::size_t N>
    struct CounterArrayInternal : std::array<Counter, N> {
      /// constructor with callables for FormatName
      template <typename OWNER, typename FormatName, std::size_t... Ns,
                typename = typename std::enable_if_t<std::is_invocable_v<FormatName, int>>>
      CounterArrayInternal( OWNER* owner, FormatName&& fname, std::integer_sequence<std::size_t, Ns...> )
          : std::array<Counter, N>{ Counter{ owner, fname( Ns ) }... } {
        static_assert( sizeof...( Ns ) < 1000, "Using CounterArray with 1000 arrays or more is prohibited. This "
                                               "would lead to very long compilation times" );
      }
      /// constructor for strings, FormatCounterDefault is used as the default callable
      template <typename OWNER, std::size_t... Ns>
      CounterArrayInternal( OWNER* owner, std::string_view name, std::integer_sequence<std::size_t, Ns...> )
          : std::array<Counter, N>{ Counter{ owner, FormatCounterDefault{ name }( Ns ) }... } {
        static_assert( sizeof...( Ns ) < 1000, "Using CounterArray with 1000 arrays or more is prohibited. This "
                                               "would lead to very long compilation times" );
      }
    };
  } // namespace details

  /**
   * generic class implementing an array of counters
   * The only addition to a raw array is the constructor that allows
   * to build names of the counters automatically from the index of the
   * counter in the array
   * There are 2 possibilities :
   *   - if a string_view is given, it is used in a call to std::format(name, n);
   *   - if a callable is given, it is called on the index
   *     it should take a size_t and return some type convertible to string_view
   * actual implementation is in CounterArrayInternal
   *
   * Typical usage :
   *    // Array of 5 simple counters with simple names. Names will be MyCounter-0, MyCounter-1, ...
   *    CounterArray<Counter<>, 5> counters{ &algo, "MyCounter-{}" };
   *    ++counters[1];
   *    // Array of 5 averaging counters with same simple names
   *    CounterArray<AveragingCounter<>, 5> avgCounters{ &algo, "MyCounter-{}" };
   *    avgCounters[2] += 3.14;
   *    // Array of 5 cimple counters with custom names. Names will be "0^2=0", "1^2=1", "2^2=4", ...
   *    CounterArray<Counter<>, 5> customCounters{
   *      &algo,
   *      []( int n ) { return fmt::format( "{}^2={}", n, n*n ); }
   *    }
   *    ++customCounters[3];
   */
  template <typename Counter, std::size_t N>
  struct CounterArray : details::CounterArrayInternal<Counter, N> {
    template <typename OWNER, typename FormatName>
    CounterArray( OWNER* owner, FormatName&& fname )
        : details::CounterArrayInternal<Counter, N>( owner, fname, std::make_integer_sequence<std::size_t, N>{} ) {}
  };

} // namespace Gaudi::Accumulators
