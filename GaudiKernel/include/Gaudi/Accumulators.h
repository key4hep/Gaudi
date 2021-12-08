/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#pragma once

#include <Gaudi/MonitoringHub.h>

#include <chrono>

namespace Gaudi::Accumulators {
  template <class Rep, class Period>
  auto sqrt( std::chrono::duration<Rep, Period> d );
  template <class Rep1, class Rep2, class Period>
  auto operator*( const std::chrono::duration<Rep1, Period>& lhs, const std::chrono::duration<Rep2, Period>& rhs );
} // namespace Gaudi::Accumulators

/**
 * @namespace Gaudi::Accumulators
 *
 * Efficient counter implementations for Gaudi.
 *
 * A number of concepts and templated classes are defined:
 *
 * Concepts
 * --------
 *
 *   - Accumulator : object accumulating some data in some way.
 *       examples : counters, sum of squares accumulator,
 *                  minimum accumulator (keeps minimum of all values)
 *   - Atomicity : the atomicity of an accumulator. Can be none or full.
 *       "none" means that the accumulator is not thread safe and should
 *       only be used locally within a thread. "full" means that the
 *       accumulator is thread safe, but that you pay the price of atomics
 *       inside. Note that this price may be reduced by the usage of the
 *       Buffer class, see below.
 *   - InputTransform : a transformation to be applied to the input of an Accumulator.
 *       for example "elevation to power 2" for a sum of squares Accumulator,
 *       "identity" for a sum Accumulator or a minimum Accumulator,
 *       "constant 1" for a pure counter
 *   - OutputTransform : a transformation to be applied to the value of an
 *       Accumulator when reading it. Usually identity. You can think of
 *       square root for an RMS Accumulator (accumulating squares internally)
 *   - ValueHandler : the class handling the internal value of an Accumulator,
 *       and responsible for updating it with a new input. It defines the initial
 *       default value, the storage type used (depending on atomicity) and the
 *       actual update operation (sum, keep min or max typically).
 *   - AccumulatorSet : an Accumulator defined as a set of Accumulators with
 *       same InputType and Atomicity. Updating it means updating all Accumulators
 *       of the set with the same value. E.g. a set with Counter and Sum will
 *       be able to compute some average
 *   - Buffer : a wrapper around a thread safe Accumulator that allows to accumulate
 *       locally (in a non thread safe Accumulator) data before merging into the original one
 *       when Buffer is destructed. To be use when accumulators are updated in tight loops
 *   - Counter : a higher level object that is an Accumulator and provides extra methods,
 *       most notably a way to print itself via operator<<, a print method and a
 *       buffer method to retrieve a Buffer on top of it.
 *
 *
 * Classes and helper functions
 * ----------------------------
 *
 *   - many classes are directly linked to concepts :
 *     + Constant, Identity, Square  are InputTransforms / OutputTransforms
 *     + BaseValueHandler : a base class for ValueHandlers Adder, Minimum and Maximum
 *     + Adder, Extremum, Minimum, Maximum, Binomial are ValueHandlers
 *     + GenericAccumulator : implements a generic Accumulator. See class definition for details
 *     + AccumulatorSet binds together a set of Accumulators into a new accumulator
 *     + Buffer implements the local buffering of an Accumulator
 *
 *   - some classes implements the most common Accumulators :
 *     + MaxAccumulator : keeps the max, has a max() method
 *     + MinAccumulator : keeps the min, has a min() method
 *     + CountAccumulator : keeps count of number of values, has a nEntries() method
 *     + SumAccumulator : keeps the sum of all values, has a sum() method
 *     + IntegralAccumulator : similar to CountAccumulator and SumAccumulator for integers, has a nEntries() and a sum()
 * method
 *     + SquareAccumulator : keeps the sum of all values squared, has a sum2() method
 *     + TrueAccumulator : keeps a count of the number of true values, has a nTrueEntries() method
 *     + FalseAccumulator : keeps a count of the number of false values, has a nFalseEntries() method
 *     + BinomialAccumulator : set of TrueAccumulator and FalseAccumulator. Has extra
 *       nEntries(), efficiency() and efficiencyErr() methods
 *     + AveragingAccumulator : set of CountAccumulator and SumAccumulator. Has an extra mean() method
 *     + SigmaAccumulator : set of AveragingAccumulator and SquareAccumulator. Has extra
 *       methods variance(), standard_deviation() and meanErr()
 *     + MsgCounter: keeps a count of number of values, and prints the first N times it
 *       is incremented at a specified logging level
 *     + StatAccumulator : set of SigmaAccumulator, MinAccumulator and MaxAccumulator
 *
 *   - some classes implement the most common Counters :
 *     + PrintableCounter is the interface to be used for abstract counters that can
 *       be printed
 *     + BufferableCounter is the base class for counters that can be buffered. It
 *       provides a buffer method returning a Buffer on the current counter
 *     + Counter, AveragingCounter, SigmaCounter, StatCounter, BinomialCounter : standard
 *       counters based on the corresponding accumulators
 *     + StatEntity : a counter meant to be backward compatible with the old one
 *       and actually being a set of StatAccumulator and BinomialAccumulator.
 *       StatEntity should not be used and should be dropped when ancient code has been
 *       ported to the other counters
 *     + MsgCounter : a counter logging a given message up to the given number of occurrences,
 *       then loggin that it stops logging but still counting occurences
 *
 * (De)Serialization
 * -----------------
 *
 * All counters are serialized into JSON strings using nlohmann's json library (pure header)
 * See https://github.com/nlohmann/json for details, but here are few notes :
 *   - all Arithmetic types used in Counters need to be serializable with this library
 *   - standard types (STL included) already are
 *   - for custom types, one needs to implement a to_json method along these lines :
 * @code
 *   void to_json(json& j, const Person& p) {
 *       j = nlohmann::json{{"name", p.name}, {"type", "person:basic"}, {"address", p.address}, {"age", p.age}};
 *   }
 * @endcode
 * This method must be declared in your type namespace
 *   - for third-party type, it's slightly more complicated and one has to specialize
 *     a class called adl_serializer for the given type. Just follow the example given in
 *     this file for std::chrono::duration
 *
 * Symetrically to serialization, all counters objects can be constructed from json objects
 * using the same nlohmann's json library via a the fromJSON static method;
 *
 * The serialized json needs to respect a given format, described in the documentation of
 * Gaudi::Monitoring::Hub, but essentially boiling down to having a 'type' entry in the
 * generated json dictionnary. This type defines the rest of the json structure.
 *
 * Here is a list of types defined in this header and their corresponding fields.
 * Note that there are 2 kinds of fields : raw ones and derived ones, built on top of raw ones.
 * Note also that <prec> when present is a string defining the precision of the data when numerical. It
 * is described at the end of the list
 * for each dependent field, the computation formula is given
 *   - counter:Counter:<prec> : empty(bool), nEntries(integer)
 *   - counter:AveragingCounter:<prec> : same as counter:Counter, sum(number), mean(number) = sum/nEntries
 *   - counter:SigmaCounter:<prec> : same as counter:AveragingCounter, sum2(number),
 *                                   standard_deviation(number) = sqrt((sum2-sum*sum/nEntries)/(nEntries-1))
 *   - counter:StatCounter:<prec> : same as counter:SigmaCounter, min(number), max(number)
 *   - counter:BinomialCounter:<prec> : nTrueEntries(integer), nFalseEntries(integer),
 *                                      nEntries(integer) = nTrueEntries+nFalseEntries
 *                                      efficiency(number) = nTrueEntries/nEntries,
 *                                      efficiencyErr(number) = (nTrueEntries*nFalseEntries)/(nEntries*nEntries)
 *   - counter:MsgCounter : same as counter:Counter, level(number), max(number), msg(string)
 *   - statentity : union of fields of counter:StatCounter and counter:BinomialCounter. DEPRECATED
 *   - timer:<prec> : same fields as counter:StatCounter
 *   - <prec> is a string defining the type of data in the counter. It is the typeid representation
 *     of the type that should be used by a customer to use the data and not lose precision.
 *     In other terms, it's usually one char of chstijlmxyfde for respectively char, unsigned char,
 *     short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long,
 *     float, double, long double
 *     Note that it does not have to match the original type, just to be the same precision.
 *     E.g. an std::chrono::nanoseconds type can and should be mapped to its internal representation
 *     that happens to be long so <prec> will be l.
 *
 * Notes
 * -----
 *
 * All Accumulators and Counters defined above are provided in their atomic and non atomic versions
 *
 * Here is an example of the typical usage of these classes :
 *
 * @code
 *  AveragingCounter<> avg;
 *  avg += 3;
 *  avg += 5;
 *  avg += 6;
 *  std::cout << avg << std::endl;
 *
 *  SigmaCounter<> sig;
 *  sig += 3;
 *  sig += 5;
 *  sig += 6;
 *  std::cout << sig << std::endl;
 *
 *  AveragingCounter<float, atomicity::full> avg2;
 *  {
 *    auto buf = avg2.buffer();
 *    for ( int i = 0; i < 1000; i++ ) buf += i;
 *    // single update of original counter when buf is destroyed
 *  }
 *  std::cout << avg2 << std::endl;
 *
 *  BinomialCounter<> bin;
 *  bin += false;
 *  bin += true;
 *  bin += true;
 *  bin += false;
 *  bin += false;
 *  std::cout << bin << std::endl;
 *
 *  StatEntity se;
 *  se += 3;
 *  se += 5;
 *  se += 6;
 *  std::cout << se << std::endl;
 *
 * @endcode
 */

#include "boost/algorithm/string/predicate.hpp"
#include "boost/format.hpp"
#include <atomic>
#include <cmath>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/detected.h"

// Json serialization for std::chrono::duration
namespace nlohmann {
  template <class Rep, class Period>
  struct adl_serializer<std::chrono::duration<Rep, Period>> {
    static void to_json( json& j, const std::chrono::duration<Rep, Period>& d ) { j = d.count(); }
    static void from_json( const json& j, std::chrono::duration<Rep, Period>& d ) {
      d = std::chrono::duration<Rep, Period>{ j.get<Rep>() };
    }
  };
} // namespace nlohmann

namespace Gaudi::Accumulators {

  /// Defines atomicity of the accumulators
  enum class atomicity { none, full };

  /// forward declaration of sqrt for custom types
  template <class T>
  auto sqrt( T d );

  /**
   * A functor always returning the value N
   */
  template <typename T, T N>
  struct Constant {
    template <typename U>
    constexpr T operator()( U&& ) const noexcept {
      return N;
    }
  };

  /**
   * An Identity functor
   */
  struct Identity {
    template <typename U>
    constexpr decltype( auto ) operator()( U&& v ) const noexcept {
      return std::forward<U>( v );
    }
  };

  /**
   * A Square functor
   */
  struct Square {
    template <typename U>
    constexpr decltype( auto ) operator()( U&& v ) const noexcept {
      return v * v;
    }
  };

  /**
   * type_traits for checking the presence of fetch_add
   */
  template <typename T, typename = int>
  using has_fetch_add_ = decltype( std::declval<T&>().fetch_add( 0 ) );
  template <typename T>
  inline constexpr bool has_fetch_add_v = Gaudi::cpp17::is_detected_v<has_fetch_add_, T>;

  /**
   * type_trait for the result type of a floating point operation on the type Arithmetic
   */
  template <typename Arithmetic, typename Result = double>
  using fp_result_type = std::conditional_t<std::is_integral_v<Arithmetic>, Result, Arithmetic>;

  /**
   * Base type for all functors used as ValuesHandler. The base takes care of storing the value
   */
  template <typename Arithmetic, atomicity Atomicity>
  struct BaseValueHandler;

  /**
   * BaseValueHandler specialization in the case of atomicity none
   */
  template <typename Arithmetic>
  struct BaseValueHandler<Arithmetic, atomicity::none> {
    using OutputType   = Arithmetic;
    using InternalType = Arithmetic;
    static constexpr OutputType getValue( const InternalType& v ) noexcept { return v; };
    static Arithmetic exchange( InternalType& v, Arithmetic newv ) noexcept { return std::exchange( v, newv ); }
  };

  /**
   * BaseValueHandler specialization in the case of atomicity full
   */
  template <typename Arithmetic>
  struct BaseValueHandler<Arithmetic, atomicity::full> {
    using OutputType   = Arithmetic;
    using InternalType = std::atomic<Arithmetic>;
    static constexpr OutputType getValue( const InternalType& v ) noexcept {
      return v.load( std::memory_order_relaxed );
    };
    static Arithmetic exchange( InternalType& v, Arithmetic newv ) noexcept { return v.exchange( newv ); }
  };

  /**
   * An Adder ValueHandler
   * operator(a, b) means a += b. In case of full atomicity, fetch_add or compare_exchange_weak are used.
   */
  template <typename Arithmetic, atomicity Atomicity>
  struct Adder;

  /**
   * Adder specialization in the case of atomicity none
   */
  template <typename Arithmetic>
  struct Adder<Arithmetic, atomicity::none> : BaseValueHandler<Arithmetic, atomicity::none> {
    using typename BaseValueHandler<Arithmetic, atomicity::none>::OutputType;
    using typename BaseValueHandler<Arithmetic, atomicity::none>::InternalType;
    static constexpr OutputType DefaultValue() { return Arithmetic{}; }
    static void                 merge( InternalType& a, Arithmetic b ) noexcept { a += b; };
  };

  /**
   * Adder specialization in the case of atomicity full
   */
  template <typename Arithmetic>
  struct Adder<Arithmetic, atomicity::full> : BaseValueHandler<Arithmetic, atomicity::full> {
    using typename BaseValueHandler<Arithmetic, atomicity::full>::OutputType;
    using typename BaseValueHandler<Arithmetic, atomicity::full>::InternalType;
    static constexpr OutputType DefaultValue() { return Arithmetic{}; }
    static void                 merge( InternalType& a, Arithmetic b ) noexcept {
      if ( DefaultValue() == b ) return; // avoid atomic operation if b is "0"
      if constexpr ( has_fetch_add_v<InternalType> ) {
        a.fetch_add( b, std::memory_order_relaxed );
      } else {
        auto current = BaseValueHandler<Arithmetic, atomicity::full>::getValue( a );
        while ( !a.compare_exchange_weak( current, current + b ) )
          ;
      }
    };
  };

  /**
   * An Extremum ValueHandler, to be reused for Minimum and Maximum
   * operator(a, b) means if (Compare(b,a)) a = b In case of full atomicity, compare_exchange_weak is used.
   */
  template <typename Arithmetic, atomicity Atomicity, typename Compare, Arithmetic ( *Initial )()>
  struct Extremum;

  /**
   * Extremum specialization in the case of atomicity none
   */
  template <typename Arithmetic, typename Compare, Arithmetic ( *Initial )()>
  struct Extremum<Arithmetic, atomicity::none, Compare, Initial> : BaseValueHandler<Arithmetic, atomicity::none> {
    using typename BaseValueHandler<Arithmetic, atomicity::none>::OutputType;
    using typename BaseValueHandler<Arithmetic, atomicity::none>::InternalType;
    static constexpr OutputType DefaultValue() { return Initial(); }
    static void                 merge( InternalType& a, Arithmetic b ) noexcept {
      if ( Compare{}( b, a ) ) a = b;
    };
  };

  /**
   * Extremum specialization in the case of atomicity full
   */
  template <typename Arithmetic, typename Compare, Arithmetic ( *Initial )()>
  struct Extremum<Arithmetic, atomicity::full, Compare, Initial> : BaseValueHandler<Arithmetic, atomicity::full> {
    using typename BaseValueHandler<Arithmetic, atomicity::full>::OutputType;
    using typename BaseValueHandler<Arithmetic, atomicity::full>::InternalType;
    static constexpr OutputType DefaultValue() { return Initial(); }
    static void                 merge( InternalType& a, Arithmetic b ) noexcept {
      Arithmetic prev_value = BaseValueHandler<Arithmetic, atomicity::full>::getValue( a );
      while ( Compare{}( b, prev_value ) && !a.compare_exchange_weak( prev_value, b ) )
        ;
    };
  };

  /**
   * A Minimun ValueHandler
   * operator(a, b) means a = min(a, b) In case of full atomicity, compare_exchange_weak is used.
   */
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
  using Minimum = Extremum<Arithmetic, Atomicity, std::less<Arithmetic>, std::numeric_limits<Arithmetic>::max>;

  /**
   * An Maximum ValueHandler
   * operator(a, b) means a = max(a, b) In case of full atomicity, compare_exchange_weak is used.
   */
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
  using Maximum = Extremum<Arithmetic, Atomicity, std::greater<Arithmetic>, std::numeric_limits<Arithmetic>::lowest>;

  /**
   * constant used to disambiguate construction of an empty Accumulator
   * versus the copy constructor. Indeed, construction of an empty Accumulator
   * still takes another Accumulator and copies configuration
   */
  struct construct_empty_t {
    explicit construct_empty_t() = default;
  };
  constexpr construct_empty_t construct_empty{};

  /**
   * Generic Accumulator, templated by
   *   - InputType : the type of input data coming to this Accumulator
   *   - InnerType : the type of the accumulated value. May be different from InputType,
   *        e.g. for a pure counter where InputType is double and InnerType is unsigned long
   *   - Atomicity : the desired atomicity : none or full
   *   - InputTransform : a function to be applied to the values pushed in
   *        + default to identity
   *        + other typical functions : square for a sumn of squares, constant 1 for a count
   *   - OutputTransform : a function to be applied to returned value
   *        + default to identity
   *        + one could think of sqrt for and RMS accumulator, InputTransform begin square
   *   - ValueHandler : the handler of the internal value, that is a functor providing :
   *        + the way to increment the internal value (e.g. +, min, max) as the operator()
   *        + the type of the internal value (typically Arithmetic or std::atomic<Arithmetic>)
   *        + the default value for the internal type
   */
  template <typename InputTypeT, typename InnerType, atomicity Atomicity = atomicity::full,
            typename InputTransform = Identity, typename OutputTransform = Identity,
            typename ValueHandler = Adder<InnerType, Atomicity>>
  class GenericAccumulator {
    template <typename, typename, atomicity, typename, typename, typename>
    friend class GenericAccumulator;

  public:
    using InputType             = InputTypeT;
    using OutputType            = std::decay_t<std::result_of_t<OutputTransform( InnerType )>>;
    using InternalType          = InnerType;
    using JSONStringEntriesType = std::string;
    GenericAccumulator operator+=( const InputType by ) {
      ValueHandler::merge( m_value, InputTransform{}( by ) );
      return *this;
    }
    GenericAccumulator() = default;
    /// constructor of an empty GenericAccumulator, copying the (non existent) config from another GenericAccumulator
    template <atomicity ato, typename VH>
    GenericAccumulator( construct_empty_t,
                        const GenericAccumulator<InputType, InnerType, ato, InputTransform, OutputTransform, VH>& )
        : GenericAccumulator() {}
    template <typename... Args>
    GenericAccumulator( std::in_place_t, Args&&... args ) : m_value( std::forward<Args>( args )... ) {}
    GenericAccumulator( const GenericAccumulator& other ) : m_value( ValueHandler::getValue( other.m_value ) ) {}
    GenericAccumulator& operator=( const GenericAccumulator& other ) {
      m_value = ValueHandler::getValue( other.m_value );
      return *this;
    }
    OutputType value() const { return OutputTransform{}( ValueHandler::getValue( m_value ) ); }
    void       reset() { reset( ValueHandler::DefaultValue() ); }
    template <atomicity ato, typename VH>
    void mergeAndReset( GenericAccumulator<InputType, InnerType, ato, InputTransform, OutputTransform, VH>&& other ) {
      ValueHandler::merge( m_value, VH::exchange( other.m_value, VH::DefaultValue() ) );
    }
    template <atomicity ato, typename VH>
    void operator+( GenericAccumulator<InputType, InnerType, ato, InputTransform, OutputTransform, VH>&& other ) {
      ValueHandler::merge( m_value, other.m_value );
    }

  protected:
    auto             rawValue() const { return ValueHandler::getValue( m_value ); }
    void             reset( InnerType in ) { m_value = std::move( in ); }
    static InnerType extractJSONData( const nlohmann::json& j, const JSONStringEntriesType& entries ) {
      return j.at( entries ).get<InnerType>();
    }

  private:
    typename ValueHandler::InternalType m_value{ ValueHandler::DefaultValue() };
  };

  /**
   * AccumulatorSet is an Accumulator that holds a set of Accumulators templated by same Arithmetic and Atomicity
   * and increase them altogether
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity, typename InputTypeT = Arithmetic,
            template <atomicity, typename> class... Bases>
  class AccumulatorSet : public Bases<Atomicity, Arithmetic>... {
  public:
    using InputType             = InputTypeT;
    using OutputType            = std::tuple<typename Bases<Atomicity, Arithmetic>::OutputType...>;
    using InternalType          = std::tuple<typename Bases<Atomicity, Arithmetic>::InternalType...>;
    using JSONStringEntriesType = std::tuple<typename Bases<Atomicity, Arithmetic>::JSONStringEntriesType...>;
    constexpr AccumulatorSet()  = default;
    /// constructor of an empty AccumulatorSet, copying the (non existent) config from another GenericAccumulator
    template <atomicity ato>
    AccumulatorSet( construct_empty_t, const AccumulatorSet<Arithmetic, ato, InputType, Bases...>& )
        : AccumulatorSet() {}
    AccumulatorSet& operator+=( const InputType by ) {
      ( Bases<Atomicity, Arithmetic>::operator+=( by ), ... );
      return *this;
    }
    OutputType value() const { return std::make_tuple( Bases<Atomicity, Arithmetic>::value()... ); }
    void       reset() { ( Bases<Atomicity, Arithmetic>::reset(), ... ); }
    template <atomicity Ato>
    void mergeAndReset( AccumulatorSet<Arithmetic, Ato, InputType, Bases...>&& other ) {
      ( Bases<Atomicity, Arithmetic>::mergeAndReset( static_cast<Bases<Ato, Arithmetic>&&>( other ) ), ... );
    }
    template <atomicity Ato>
    void operator+( AccumulatorSet<Arithmetic, Ato, InputType, Bases...>&& other ) {
      ( Bases<Atomicity, Arithmetic>::operator+( static_cast<Bases<Ato, Arithmetic>&&>( other ) ), ... );
    }

  protected:
    void reset( const InternalType& t ) {
      std::apply( [this]( const auto&... i ) { ( this->Bases<Atomicity, Arithmetic>::reset( i ), ... ); }, t );
    }
    static InternalType extractJSONData( const nlohmann::json& j, const JSONStringEntriesType& entries ) {
      return extractJSONDataHelper( j, entries, std::index_sequence_for<Bases<Atomicity, Arithmetic>...>{} );
    }

  private:
    template <size_t... Is>
    static InternalType extractJSONDataHelper( const nlohmann::json& j, const JSONStringEntriesType& entries,
                                               std::index_sequence<Is...> ) {
      return extractJSONDataHelper( j, std::get<Is>( entries )... );
    }
    static InternalType
    extractJSONDataHelper( const nlohmann::json& j,
                           typename Bases<Atomicity, Arithmetic>::JSONStringEntriesType... entries ) {
      return { Bases<Atomicity, Arithmetic>::extractJSONData( j, entries )... };
    }
  };

  /**
   * MaxAccumulator. A MaxAccumulator is an Accumulator storing the max value of the provided arguments
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct MaxAccumulator
      : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity, Maximum<Arithmetic, Atomicity>> {
    using GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity,
                             Maximum<Arithmetic, Atomicity>>::GenericAccumulator;
    Arithmetic max() const { return this->value(); }
  };

  /**
   * MinAccumulator. A MinAccumulator is an Accumulator storing the min value of the provided arguments
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct MinAccumulator
      : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity, Minimum<Arithmetic, Atomicity>> {
    using GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity,
                             Minimum<Arithmetic, Atomicity>>::GenericAccumulator;
    Arithmetic min() const { return this->value(); }
  };

  /**
   * CountAccumulator. A CountAccumulator is an Accumulator storing the number of provided values
   * @see Gaudi::Accumulators for detailed documentation
   * Note that the Arithmetic type is actually irrelevant here, it's only there
   * for compatibility with other accumulators when aggregating them
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct CountAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, Constant<unsigned long, 1UL>> {
    using GenericAccumulator<Arithmetic, unsigned long, Atomicity, Constant<unsigned long, 1UL>>::GenericAccumulator;
    CountAccumulator& operator++() {
      ( *this ) += Arithmetic{};
      return *this;
    }
    CountAccumulator operator++( int ) {
      auto copy = *this;
      ++( *this );
      return copy;
    }
    unsigned long nEntries() const { return this->value(); }
  };

  /**
   * SumAccumulator. A SumAccumulator is an Accumulator storing the sum of the provided values
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct SumAccumulator : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity> {
    using GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity>::GenericAccumulator;
    Arithmetic sum() const { return this->value(); }
  };

  /**
   * IntegralAccumulator. An IntegralAccumulator is an Accumulator with an integral storage
   * (unsigned long, int, ...) that can be incremented via operator++ and operator+=.
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = unsigned long>
  struct IntegralAccumulator : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity> {
    static_assert( std::is_integral_v<Arithmetic>,
                   "Invalid Arithmetic type for IntegralAccumulator. It must be an integral type" );

    using GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity>::GenericAccumulator;
    IntegralAccumulator& operator++() {
      ( *this ) += 1;
      return *this;
    }
    IntegralAccumulator operator++( int ) {
      auto copy = *this;
      ++( *this );
      return copy;
    }
    Arithmetic nEntries() const { return this->value(); }
    Arithmetic sum() const { return this->value(); }
  };

  /**
   * SquareAccumulator. A SquareAccumulator is an Accumulator storing the sum of squares of the provided values
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct SquareAccumulator : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Square> {
    using GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Square>::GenericAccumulator;
    Arithmetic sum2() const { return this->value(); };
  };

  /// helper functor for the TrueAccumulator
  struct TrueTo1 {
    unsigned int operator()( bool v ) const { return v; }
  };

  /**
   * TrueAccumulator. A TrueAccumulator is an Accumulator counting the number of True values in the data
   * data
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct TrueAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, TrueTo1> {
    using GenericAccumulator<Arithmetic, unsigned long, Atomicity, TrueTo1>::GenericAccumulator;
    unsigned long nTrueEntries() const { return this->value(); };
  };

  /// helper functor for the FalseAccumulator
  struct FalseTo1 {
    unsigned int operator()( bool v ) const { return !v; }
  };

  /**
   * FalseAccumulator. A FalseAccumulator is an Accumulator counting the number of False values in the data
   * data
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct FalseAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, FalseTo1> {
    using GenericAccumulator<Arithmetic, unsigned long, Atomicity, FalseTo1>::GenericAccumulator;
    unsigned long nFalseEntries() const { return this->value(); };
  };

  /**
   * BinomialAccumulator. A BinomialAccumulator is an Accumulator able to compute the efficiency of a process
   * data
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct BinomialAccumulator : AccumulatorSet<bool, Atomicity, bool, TrueAccumulator, FalseAccumulator> {
    using AccumulatorSet<bool, Atomicity, bool, TrueAccumulator, FalseAccumulator>::AccumulatorSet;
    unsigned long nEntries() const { return this->nTrueEntries() + this->nFalseEntries(); };

    template <typename Result = fp_result_type<Arithmetic>>
    auto efficiency() const {
      auto nbEntries = nEntries();
      if ( 1 > nbEntries ) return Result{ -1 };
      return static_cast<Result>( this->nTrueEntries() ) / nbEntries;
    }
    auto eff() const { return efficiency(); }

    template <typename Result = fp_result_type<Arithmetic>>
    auto efficiencyErr() const {
      // Note the usage of using, aiming at using the std version of sqrt by default, without preventing
      // more specialized versions to be used via ADL (see http://en.cppreference.com/w/cpp/language/adl)
      using Gaudi::Accumulators::sqrt;
      using std::sqrt;
      auto nbEntries = nEntries();
      if ( 1 > nbEntries ) return Result{ -1 };
      return sqrt( static_cast<Result>( this->nTrueEntries() * this->nFalseEntries() ) / nbEntries ) / nbEntries;
    }
    auto effErr() const { return efficiencyErr(); }
    using AccumulatorSet<bool, Atomicity, bool, TrueAccumulator, FalseAccumulator>::operator+=;
    struct binomial_t {
      unsigned long nPass;
      unsigned long nTotal;
    };
    BinomialAccumulator& operator+=( binomial_t b ) {
      assert( b.nPass <= b.nTotal );
      TrueAccumulator<Atomicity, bool>::mergeAndReset(
          TrueAccumulator<atomicity::none, bool>{ std::in_place, b.nPass } );
      FalseAccumulator<Atomicity, bool>::mergeAndReset(
          FalseAccumulator<atomicity::none, bool>{ std::in_place, b.nTotal - b.nPass } );
      return *this;
    }
  };

  /**
   * AveragingAccumulatorBase. An AveragingAccumulator is an Accumulator able to compute an average
   * This Base class is still templated on the counting and summing accumulators
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic, template <atomicity, typename> typename CountAcc,
            template <atomicity, typename> typename SumAcc>
  struct AveragingAccumulatorBase
      : AccumulatorSet<Arithmetic, Atomicity, typename CountAcc<Atomicity, Arithmetic>::InputType, CountAcc, SumAcc> {
    static_assert( std::is_same_v<typename CountAcc<Atomicity, Arithmetic>::InputType,
                                  typename SumAcc<Atomicity, Arithmetic>::InputType>,
                   "Incompatible Counters in definition of AveragingAccumulator. Both should have identical Input" );
    using AccumulatorSet<Arithmetic, Atomicity, typename CountAcc<Atomicity, Arithmetic>::InputType, CountAcc,
                         SumAcc>::AccumulatorSet;
    template <typename Result = fp_result_type<Arithmetic>>
    auto mean() const {
      auto   n   = this->nEntries();
      Result sum = this->sum();
      return ( n > 0 ) ? static_cast<Result>( sum / n ) : Result{};
    }
  };

  /**
   * AveragingAccumulator. An AveragingAccumulator is an Accumulator able to compute an average
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using AveragingAccumulator = AveragingAccumulatorBase<Atomicity, Arithmetic, CountAccumulator, SumAccumulator>;

  /**
   * SigmaAccumulatorBase. A SigmaAccumulator is an Accumulator able to compute an average and variance/rms
   * This Base class is still templated on the averaging and square accumulators
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic, template <atomicity, typename> typename AvgAcc,
            template <atomicity, typename> typename SquareAcc>
  struct SigmaAccumulatorBase
      : AccumulatorSet<Arithmetic, Atomicity, typename AvgAcc<Atomicity, Arithmetic>::InputType, AvgAcc, SquareAcc> {
    static_assert( std::is_same_v<typename AvgAcc<Atomicity, Arithmetic>::InputType,
                                  typename SquareAcc<Atomicity, Arithmetic>::InputType>,
                   "Incompatible Counters in definition of SigmaAccumulator. Both should have identical Input" );
    using AccumulatorSet<Arithmetic, Atomicity, typename SquareAcc<Atomicity, Arithmetic>::InputType, AvgAcc,
                         SquareAcc>::AccumulatorSet;
    template <typename Result = fp_result_type<Arithmetic>>
    auto biased_sample_variance() const {
      auto   n   = this->nEntries();
      Result sum = this->sum();
      return ( n > 0 ) ? static_cast<Result>( ( this->sum2() - sum * ( sum / n ) ) / n ) : Result{};
    }

    template <typename Result = fp_result_type<Arithmetic>>
    auto unbiased_sample_variance() const {
      auto   n   = this->nEntries();
      Result sum = this->sum();
      return ( n > 1 ) ? static_cast<Result>( ( this->sum2() - sum * ( sum / n ) ) / ( n - 1 ) ) : Result{};
    }

    template <typename Result = fp_result_type<Arithmetic>>
    auto standard_deviation() const {
      // Note the usage of using, aiming at using the std version of sqrt by default, without preventing
      // more specialized versions to be used via ADL (see http://en.cppreference.com/w/cpp/language/adl)
      using Gaudi::Accumulators::sqrt;
      using std::sqrt;
      Result v = biased_sample_variance();
      return ( Result{ 0 } > v ) ? Result{} : static_cast<Result>( sqrt( v ) );
    }
    [[deprecated( "The name 'rms' has changed to standard_deviation" )]] Arithmetic rms() const {
      return standard_deviation();
    }

    template <typename Result = fp_result_type<Arithmetic>>
    auto meanErr() const {
      auto n = this->nEntries();
      if ( 0 == n ) return Result{};
      // Note the usage of using, aiming at using the std version of sqrt by default, without preventing
      // more specialized versions to be used via ADL (see http://en.cppreference.com/w/cpp/language/adl)
      using Gaudi::Accumulators::sqrt;
      using std::sqrt;
      Result v = biased_sample_variance();
      return ( Result{ 0 } > v ) ? Result{} : static_cast<Result>( sqrt( v / n ) );
    }
  };

  /**
   * SigmaAccumulator. A SigmaAccumulator is an Accumulator able to compute an average and variance/rms
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using SigmaAccumulator = SigmaAccumulatorBase<Atomicity, Arithmetic, AveragingAccumulator, SquareAccumulator>;

  /**
   * StatAccumulator. A StatAccumulator is an Accumulator able to compute an average, variance/rms and min/max
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using StatAccumulator =
      AccumulatorSet<Arithmetic, Atomicity, Arithmetic, SigmaAccumulator, MinAccumulator, MaxAccumulator>;

  /**
   * Buffer is a non atomic Accumulator which, when it goes out-of-scope,
   * updates the underlying thread-safe Accumulator for all previous updates in one go.
   * It is templated by the basic accumulator type and has same interface
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <template <atomicity Ato, typename... Int> class ContainedAccumulator, atomicity Atomicity, typename... Args>
  class Buffer : public ContainedAccumulator<atomicity::none, Args...> {
    using prime_type = ContainedAccumulator<Atomicity, Args...>;
    using base_type  = ContainedAccumulator<atomicity::none, Args...>;

  public:
    Buffer() = delete;
    Buffer( prime_type& p ) : base_type( construct_empty, p ), m_prime( p ) {}
    Buffer( const Buffer& ) = delete;
    void operator=( const Buffer& ) = delete;
    Buffer( Buffer&& other ) : base_type( other ), m_prime( other.m_prime ) { other.reset(); }
    void push() { m_prime.mergeAndReset( static_cast<base_type&&>( *this ) ); }
    ~Buffer() { push(); }

  private:
    prime_type& m_prime;
  };

  /**
   * An empty ancester of all counters that knows how to print themselves
   * @see Gaudi::Accumulators for detailed documentation
   */
  struct PrintableCounter {
    PrintableCounter() = default;
    /// destructor
    virtual ~PrintableCounter() = default;
    // add tag to printout
    template <typename stream>
    stream& printImpl( stream& s, std::string_view tag ) const {
      s << boost::format{ " | %|-48.48s|%|50t|" } % ( std::string{ '\"' }.append( tag ).append( "\"" ) );
      return print( s, true );
    }
    /// prints the counter to a stream
    virtual std::ostream& print( std::ostream&, bool tableFormat = false ) const = 0;
    virtual MsgStream&    print( MsgStream&, bool tableFormat = true ) const     = 0;
    /// prints the counter to a stream in table format, with the given tag
    virtual std::ostream& print( std::ostream& o, std::string_view tag ) const { return printImpl( o, tag ); }
    virtual MsgStream&    print( MsgStream& o, std::string_view tag ) const { return printImpl( o, tag ); }
    /** hint whether we should print that counter or not.
        Typically empty counters may not be printed */
    virtual bool toBePrinted() const { return true; }
    /// get a string representation
    std::string toString() const {
      std::ostringstream ost;
      print( ost );
      return ost.str();
    }
    /// Basic JSON export for Gaudi::Monitoring::Hub support.
    virtual nlohmann::json toJSON() const = 0;
  };

  /**
   * external printout operator to a stream type
   */
  inline std::ostream& operator<<( std::ostream& s, const PrintableCounter& counter ) { return counter.print( s ); }
  inline MsgStream&    operator<<( MsgStream& s, const PrintableCounter& counter ) { return counter.print( s ); }
  /**
   * An empty ancester of all counters that provides a buffer method that returns a buffer on itself
   * Also registers the counter to its owner, with default type "counter"
   * Due to this registration, move semantic is disabled. But copy semantic remains.
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, template <atomicity Ato, typename... Int> class Accumulator, typename... Args>
  class BufferableCounter : public PrintableCounter, public Accumulator<Atomicity, Args...> {
  public:
    BufferableCounter() = default;
    template <typename OWNER, typename... CARGS>
    BufferableCounter( OWNER* o, std::string const& name, const std::string counterType, CARGS... args )
        : Accumulator<Atomicity, Args...>( args... ), m_monitoringHub( &o->serviceLocator()->monitoringHub() ) {
      m_monitoringHub->registerEntity( o->name(), name, counterType, *this );
    }
    template <typename OWNER>
    BufferableCounter( OWNER* o, std::string const& name ) : BufferableCounter( o, name, "counter" ) {}
    Buffer<Accumulator, Atomicity, Args...> buffer() { return { *this }; }
    BufferableCounter( BufferableCounter const& ) = delete;
    BufferableCounter& operator=( BufferableCounter const& ) = delete;
    ~BufferableCounter() {
      if ( m_monitoringHub ) { m_monitoringHub->removeEntity( *this ); }
    }

  private:
    Monitoring::Hub* m_monitoringHub{ nullptr };
  };

  /**
   * A basic integral counter;
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity = atomicity::full, typename Arithmetic = unsigned long>
  struct Counter : BufferableCounter<Atomicity, IntegralAccumulator, Arithmetic> {
    inline static const std::string typeString{ std::string{ "counter:Counter:" } + typeid( unsigned long ).name() };
    using BufferableCounter<Atomicity, IntegralAccumulator, Arithmetic>::BufferableCounter;
    template <typename OWNER>
    Counter( OWNER* o, std::string const& name )
        : BufferableCounter<Atomicity, IntegralAccumulator, Arithmetic>( o, name, typeString ) {}
    Counter& operator++() { return ( *this ) += 1; }
    Counter& operator+=( const Arithmetic v ) {
      BufferableCounter<Atomicity, IntegralAccumulator, Arithmetic>::operator+=( v );
      return *this;
    }
    using BufferableCounter<Atomicity, IntegralAccumulator, Arithmetic>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      // Avoid printing empty counters in non DEBUG mode
      auto fmt = ( tableFormat ? "|%|10d| |" : "#=%|-7lu|" );
      return o << boost::format{ fmt } % this->nEntries();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    bool       toBePrinted() const override { return this->nEntries() > 0; }
    virtual nlohmann::json toJSON() const override {
      return { { "type", typeString }, { "empty", this->nEntries() == 0 }, { "nEntries", this->nEntries() } };
    }
    static Counter fromJSON( const nlohmann::json& j ) {
      return CountAccumulator<Atomicity, int>::extractJSONData( j, { "nEntries" } );
    }
  };

  /**
   * A counter aiming at computing sum and average
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct AveragingCounter : BufferableCounter<Atomicity, AveragingAccumulator, Arithmetic> {
    inline static const std::string typeString{ std::string{ "counter:AveragingCounter:" } +
                                                typeid( Arithmetic ).name() };
    using BufferableCounter<Atomicity, AveragingAccumulator, Arithmetic>::BufferableCounter;
    template <typename OWNER>
    AveragingCounter( OWNER* o, std::string const& name )
        : BufferableCounter<Atomicity, AveragingAccumulator, Arithmetic>( o, name, typeString ) {}
    using BufferableCounter<Atomicity, AveragingAccumulator, Arithmetic>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.7g| |%|#11.5g| |" : "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g|" );
      return o << boost::format{ fmt } % this->nEntries() % this->sum() % this->mean();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }

    bool                   toBePrinted() const override { return this->nEntries() > 0; }
    virtual nlohmann::json toJSON() const override {
      return { { "type", typeString },
               { "empty", this->nEntries() == 0 },
               { "nEntries", this->nEntries() },
               { "sum", this->sum() },
               { "mean", this->mean() } };
    }
    static AveragingCounter fromJSON( const nlohmann::json& j ) {
      return AveragingAccumulator<Atomicity, Arithmetic>::extractJSONData( j, { "nEntries", "sum" } );
    }
  };
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  using SummingCounter = AveragingCounter<Arithmetic, Atomicity>;

  /**
   * A counter aiming at computing average and sum2 / variance / standard deviation
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct SigmaCounter : BufferableCounter<Atomicity, SigmaAccumulator, Arithmetic> {
    inline static const std::string typeString{ std::string{ "counter:SigmaCounter:" } + typeid( Arithmetic ).name() };
    using BufferableCounter<Atomicity, SigmaAccumulator, Arithmetic>::BufferableCounter;
    template <typename OWNER>
    SigmaCounter( OWNER* o, std::string const& name )
        : BufferableCounter<Atomicity, SigmaAccumulator, Arithmetic>( o, name, typeString ) {}
    using BufferableCounter<Atomicity, SigmaAccumulator, Arithmetic>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |"
                               : "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g|" );
      return o << boost::format{ fmt } % this->nEntries() % this->sum() % this->mean() % this->standard_deviation();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    bool       toBePrinted() const override { return this->nEntries() > 0; }
    virtual nlohmann::json toJSON() const override {
      return { { "type", typeString },
               { "empty", this->nEntries() == 0 },
               { "nEntries", this->nEntries() },
               { "sum", this->sum() },
               { "mean", this->mean() },
               { "sum2", this->sum2() },
               { "standard_deviation", this->standard_deviation() } };
    }
    static SigmaCounter fromJSON( const nlohmann::json& j ) {
      return SigmaAccumulator<Atomicity, Arithmetic>::extractJSONData( j, { { "nEntries", "sum" }, "sum2" } );
    }
  };

  /**
   * A counter aiming at computing average and sum2 / variance / standard deviation
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct StatCounter : BufferableCounter<Atomicity, StatAccumulator, Arithmetic> {
    inline static const std::string typeString{ std::string{ "counter:StatCounter:" } + typeid( Arithmetic ).name() };
    using BufferableCounter<Atomicity, StatAccumulator, Arithmetic>::BufferableCounter;
    template <typename OWNER>
    StatCounter( OWNER* o, std::string const& name )
        : BufferableCounter<Atomicity, StatAccumulator, Arithmetic>( o, name, typeString ) {}
    using BufferableCounter<Atomicity, StatAccumulator, Arithmetic>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |"
                               : "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g| Min/Max=%|#10.4g|/%|-#10.4g|" );
      return o << boost::format{ fmt } % this->nEntries() % this->sum() % this->mean() % this->standard_deviation() %
                      this->min() % this->max();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    bool       toBePrinted() const override { return this->nEntries() > 0; }
    virtual nlohmann::json toJSON() const override {
      return { { "type", typeString },
               { "empty", this->nEntries() == 0 },
               { "nEntries", this->nEntries() },
               { "sum", this->sum() },
               { "mean", this->mean() },
               { "sum2", this->sum2() },
               { "standard_deviation", this->standard_deviation() },
               { "min", this->min() },
               { "max", this->max() } };
    }
    static StatCounter fromJSON( const nlohmann::json& j ) {
      return StatAccumulator<Atomicity, Arithmetic>::extractJSONData(
          j, { { { "nEntries", "sum" }, "sum2" }, "min", "max" } );
    }
  };

  /**
   * A counter dealing with binomial data
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct BinomialCounter : BufferableCounter<Atomicity, BinomialAccumulator, Arithmetic> {
    inline static const std::string typeString{ std::string{ "counter:BinomialCounter:" } +
                                                typeid( Arithmetic ).name() };
    using BufferableCounter<Atomicity, BinomialAccumulator, Arithmetic>::BufferableCounter;
    template <typename OWNER>
    BinomialCounter( OWNER* o, std::string const& name )
        : BufferableCounter<Atomicity, BinomialAccumulator, Arithmetic>( o, name, typeString ) {}

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.5g| |(%|#9.7g| +- %|-#8.7g|)%% |"
                               : "#=%|-7lu| Sum=%|-11.5g| Eff=|(%|#9.7g| +- %|-#8.6g|)%%|" );
      return o << boost::format{ fmt } % this->nEntries() % this->nTrueEntries() % ( this->efficiency() * 100 ) %
                      ( this->efficiencyErr() * 100 );
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }

    template <typename stream>
    stream& printImpl( stream& o, std::string_view tag ) const {
      // override default print to add a '*' in from of the name
      o << boost::format{ " |*%|-48.48s|%|50t|" } % ( std::string{ "\"" }.append( tag ).append( "\"" ) );
      return print( o, true );
    }
    /// prints the counter to a stream in table format, with the given tag
    std::ostream&          print( std::ostream& o, std::string_view tag ) const override { return printImpl( o, tag ); }
    MsgStream&             print( MsgStream& o, std::string_view tag ) const override { return printImpl( o, tag ); }
    bool                   toBePrinted() const override { return this->nEntries() > 0; }
    virtual nlohmann::json toJSON() const override {
      return { { "type", typeString },
               { "empty", this->nEntries() == 0 },
               { "nEntries", this->nTrueEntries() + this->nFalseEntries() },
               { "nTrueEntries", this->nTrueEntries() },
               { "nFalseEntries", this->nFalseEntries() },
               { "efficiency", this->efficiency() },
               { "efficiencyErr", this->efficiencyErr() } };
    }
    static BinomialCounter fromJSON( const nlohmann::json& j ) {
      return BinomialAccumulator<Atomicity, Arithmetic>::extractJSONData( j, { "nTrueEntries", "nFalseEntries" } );
    }
  };

  namespace details::MsgCounter {
    template <atomicity Atomicity>
    struct Handler : Adder<unsigned long, Atomicity> {
      using Base = Adder<unsigned long, Atomicity>;
      static void merge( typename Base::InternalType& orig, bool b ) {
        if ( b ) Base::merge( orig, 1 );
      }
    };
    // note that Arithmetic type is unused in this case but needs to be there in case
    // we want to create AccumulatorSets with this Accumulator
    template <atomicity Atomicity, typename Arithmetic = double>
    using MsgAccumulator = GenericAccumulator<bool, unsigned long, Atomicity, Identity, Identity, Handler<Atomicity>>;
  } // namespace details::MsgCounter

  template <MSG::Level level, atomicity Atomicity = atomicity::full>
  class MsgCounter : public PrintableCounter, public details::MsgCounter::MsgAccumulator<Atomicity> {
  public:
    inline static const std::string typeString{ "counter:MsgCounter" };
    template <typename OWNER>
    MsgCounter( OWNER* o, std::string const& ms, int nMax = 10 )
        : m_monitoringHub{ o->serviceLocator()->monitoringHub() }, logger( o ), msg( ms ), max( nMax ) {
      m_monitoringHub.registerEntity( o->name(), std::move( ms ), typeString, *this );
    }
    MsgCounter& operator++() {
      ( *this ) += true;
      return *this;
    }
    MsgCounter& operator+=( const bool by ) {
      details::MsgCounter::MsgAccumulator<Atomicity>::operator+=( by );
      if ( by ) log();
      return *this;
    }
    MsgCounter( MsgCounter const& ) = delete;
    MsgCounter& operator=( MsgCounter const& ) = delete;
    ~MsgCounter() { m_monitoringHub.removeEntity( *this ); }
    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      return o << boost::format{ tableFormat ? "|%|10d| |" : "#=%|-7lu|" } % this->value();
    }
    using PrintableCounter::print;
    std::ostream& print( std::ostream& os, bool tableFormat ) const override { return printImpl( os, tableFormat ); }
    MsgStream&    print( MsgStream& os, bool tableFormat ) const override { return printImpl( os, tableFormat ); }
    bool          toBePrinted() const override { return this->value() > 0; }
    virtual nlohmann::json toJSON() const override {
      return { { "type", typeString },
               { "empty", this->value() == 0 },
               { "nEntries", this->value() },
               { "level", level },
               { "max", max },
               { "msg", msg } };
    }
    static MsgCounter fromJSON( const nlohmann::json& j ) {
      MsgCounter c = MsgCounter::extractJSONData( j, { "nEntries" } );
      c.msg        = j.at( "msg" ).get<std::string>();
      c.max        = j.at( "max" ).get<unsigned long>();
    }

  private:
    Monitoring::Hub&           m_monitoringHub;
    const CommonMessagingBase* logger{ nullptr };
    std::string                msg;
    unsigned long              max;
    void                       log() {
      if ( UNLIKELY( this->value() <= max ) && logger ) {
        if ( this->value() == max ) {
          logger->msgStream( level ) << "Suppressing message: " << std::quoted( msg, '\'' ) << endmsg;
        } else {
          logger->msgStream( level ) << msg << endmsg;
        }
      }
    }
  };

  /**
   * A helper function for accumulating data from a container into a counter
   * This is internally using buffers so that the original counter is only
   * updated once.
   */
  template <typename Counter, typename Container, typename Fun>
  void accumulate( Counter& counter, const Container& container, Fun f = Identity{} ) {
    auto b = counter.buffer();
    for ( const auto& elem : container ) b += f( elem );
  }

} // namespace Gaudi::Accumulators
