
#pragma once

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
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/detected.h"

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
   * type_traits for checking the presence of fetch_add in std::atomic<T>
   */
  template <typename T, typename = int>
  using has_fetch_add_ = decltype( std::atomic<T>{}.fetch_add( 0 ) );
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
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
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
  template <typename InputType, typename InnerType, atomicity Atomicity = atomicity::full,
            typename InputTransform = Identity, typename OutputTransform = Identity,
            typename ValueHandler = Adder<InnerType, Atomicity>>
  class GenericAccumulator {
    template <typename, typename, atomicity, typename, typename, typename>
    friend class GenericAccumulator;

  public:
    using OutputType = std::decay_t<std::result_of_t<OutputTransform( InnerType )>>;
    GenericAccumulator operator+=( const InputType by ) {
      ValueHandler::merge( m_value, InputTransform{}( by ) );
      return *this;
    }
    GenericAccumulator() = default;
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

  protected:
    void reset( InnerType in ) { m_value = std::move( in ); }

  private:
    typename ValueHandler::InternalType m_value{ValueHandler::DefaultValue()};
  };

  /**
   * AccumulatorSet is an Accumulator that holds a set of Accumulators templated by same Arithmetic and Atomicity
   * and increase them altogether
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity, template <typename, atomicity> class... Bases>
  class AccumulatorSet : public Bases<Arithmetic, Atomicity>... {
  public:
    using InputType            = Arithmetic;
    using OutputType           = std::tuple<typename Bases<Arithmetic, Atomicity>::OutputType...>;
    constexpr AccumulatorSet() = default;
    AccumulatorSet& operator+=( const InputType by ) {
      ( Bases<Arithmetic, Atomicity>::operator+=( by ), ... );
      return *this;
    }
    OutputType value() const { return std::make_tuple( Bases<Arithmetic, Atomicity>::value()... ); }
    void       reset() { ( Bases<Arithmetic, Atomicity>::reset(), ... ); }
    template <atomicity Ato>
    void mergeAndReset( AccumulatorSet<Arithmetic, Ato, Bases...>&& other ) {
      ( Bases<Arithmetic, Atomicity>::mergeAndReset( static_cast<Bases<Arithmetic, Ato>&&>( other ) ), ... );
    }

  protected:
    void reset( const std::tuple<typename Bases<Arithmetic, Atomicity>::OutputType...>& t ) {
      std::apply( [this]( const auto&... i ) { ( this->Bases<Arithmetic, Atomicity>::reset( i ), ... ); }, t );
    }
  };

  /**
   * MaxAccumulator. A MaxAccumulator is an Accumulator storing the max value of the provided arguments
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct MaxAccumulator
      : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity, Maximum<Arithmetic, Atomicity>> {
    Arithmetic max() const { return this->value(); }
  };

  /**
   * MinAccumulator. A MinAccumulator is an Accumulator storing the min value of the provided arguments
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct MinAccumulator
      : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity, Minimum<Arithmetic, Atomicity>> {
    Arithmetic min() const { return this->value(); }
  };

  /**
   * CountAccumulator. A CountAccumulator is an Accumulator storing the number of provided values
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct CountAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, Constant<unsigned long, 1UL>> {
    unsigned long nEntries() const { return this->value(); }
  };

  /**
   * SumAccumulator. A SumAccumulator is an Accumulator storing the sum of the provided values
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct SumAccumulator : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity> {
    Arithmetic sum() const { return this->value(); }
  };

  /**
   * SquareAccumulator. A SquareAccumulator is an Accumulator storing the sum of squares of the provided values
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct SquareAccumulator : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Square> {
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
  template <typename Arithmetic, atomicity Atomicity>
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
  template <typename Arithmetic, atomicity Atomicity>
  struct FalseAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, FalseTo1> {
    using GenericAccumulator<Arithmetic, unsigned long, Atomicity, FalseTo1>::GenericAccumulator;
    unsigned long nFalseEntries() const { return this->value(); };
  };

  /**
   * BinomialAccumulator. A BinomialAccumulator is an Accumulator able to compute the efficiency of a process
   * data
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
  struct BinomialAccumulator : AccumulatorSet<bool, Atomicity, TrueAccumulator, FalseAccumulator> {
    unsigned long nEntries() const { return this->nTrueEntries() + this->nFalseEntries(); };

    template <typename Result = fp_result_type<Arithmetic>>
    auto efficiency() const {
      auto nbEntries = nEntries();
      if ( 1 > nbEntries ) return Result{-1};
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
      if ( 1 > nbEntries ) return Result{-1};
      return sqrt( static_cast<Result>( this->nTrueEntries() * this->nFalseEntries() ) / nbEntries ) / nbEntries;
    }
    auto                                                                      effErr() const { return efficiencyErr(); }
    using AccumulatorSet<bool, Atomicity, TrueAccumulator, FalseAccumulator>::operator+=;
    struct binomial_t {
      unsigned long nPass;
      unsigned long nTotal;
    };
    BinomialAccumulator& operator+=( binomial_t b ) {
      assert( b.nPass <= b.nTotal );
      TrueAccumulator<bool, Atomicity>::mergeAndReset( TrueAccumulator<bool, atomicity::none>{std::in_place, b.nPass} );
      FalseAccumulator<bool, Atomicity>::mergeAndReset(
          FalseAccumulator<bool, atomicity::none>{std::in_place, b.nTotal - b.nPass} );
      return *this;
    }
  };

  /**
   * AveragingAccumulator. An AveragingAccumulator is an Accumulator able to compute an average
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
  struct AveragingAccumulator : AccumulatorSet<Arithmetic, Atomicity, CountAccumulator, SumAccumulator> {

    template <typename Result = fp_result_type<Arithmetic>>
    auto mean() const {
      auto   n   = this->nEntries();
      Result sum = this->sum();
      return ( n > 0 ) ? static_cast<Result>( sum / n ) : Result{};
    }
  };

  /**
   * SigmaAccumulator. A SigmaAccumulator is an Accumulator able to compute an average and variance/rms
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
  struct SigmaAccumulator : AccumulatorSet<Arithmetic, Atomicity, AveragingAccumulator, SquareAccumulator> {

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
      return ( Result{0} > v ) ? Result{} : static_cast<Result>( sqrt( v ) );
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
      return ( Result{0} > v ) ? Result{} : static_cast<Result>( sqrt( v / n ) );
    }
  };

  /**
   * StatAccumulator. A StatAccumulator is an Accumulator able to compute an average, variance/rms and min/max
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity = atomicity::full>
  using StatAccumulator = AccumulatorSet<Arithmetic, Atomicity, SigmaAccumulator, MinAccumulator, MaxAccumulator>;

  /**
   * Buffer is a non atomic Accumulator which, when it goes out-of-scope,
   * updates the underlying thread-safe Accumulator for all previous updates in one go.
   * It is templated by the basic accumulator type and has same interface
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, template <typename Int, atomicity Ato> class ContainedAccumulator>
  class Buffer : public ContainedAccumulator<Arithmetic, atomicity::none> {
    using prime_type = ContainedAccumulator<Arithmetic, atomicity::full>;
    using base_type  = ContainedAccumulator<Arithmetic, atomicity::none>;

  public:
    Buffer() = delete;
    Buffer( prime_type& p ) : base_type(), m_prime( p ) {}
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
    template <class OWNER>
    PrintableCounter( OWNER* o, const std::string& tag ) {
      o->declareCounter( tag, *this );
    }
    /// destructor
    virtual ~PrintableCounter() = default;
    // add tag to printout
    template <typename stream>
    stream& printImpl( stream& s, const std::string& tag ) const {
      s << boost::format{" | %|-48.48s|%|50t|"} % ( "\"" + tag + "\"" );
      return print( s, true );
    }
    /// prints the counter to a stream
    virtual std::ostream& print( std::ostream&, bool tableFormat = false ) const = 0;
    virtual MsgStream&    print( MsgStream&, bool tableFormat = true ) const     = 0;
    /// prints the counter to a stream in table format, with the given tag
    virtual std::ostream& print( std::ostream& o, const std::string& tag ) const { return printImpl( o, tag ); }
    virtual MsgStream&    print( MsgStream& o, const std::string& tag ) const { return printImpl( o, tag ); }
    /** hint whether we should print that counter or not.
        Typically empty counters may not be printed */
    virtual bool toBePrinted() const { return true; }
    /// get a string representation
    std::string toString() const {
      std::ostringstream ost;
      print( ost );
      return ost.str();
    }
  };

  /**
   * external printout operator to a stream type
   */
  inline std::ostream& operator<<( std::ostream& s, const PrintableCounter& counter ) { return counter.print( s ); }
  inline MsgStream&    operator<<( MsgStream& s, const PrintableCounter& counter ) { return counter.print( s ); }
  /**
   * An empty ancester of all counters that provides a buffer method that
   * returns a buffer on itself
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic, atomicity Atomicity, template <typename Int, atomicity Ato> class Accumulator>
  struct BufferableCounter : PrintableCounter {
    using PrintableCounter::PrintableCounter;
    Buffer<Arithmetic, Accumulator> buffer() { return {*static_cast<Accumulator<Arithmetic, Atomicity>*>( this )}; }
  };

  /**
   * A basic counter counting input values
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct Counter : BufferableCounter<Arithmetic, Atomicity, Counter>, CountAccumulator<Arithmetic, Atomicity> {
    using BufferableCounter<Arithmetic, Atomicity, Counter>::BufferableCounter;
    Counter& operator++() {
      ( *this ) += Arithmetic{};
      return *this;
    }
    Counter operator++( int ) {
      auto copy = *this;
      ++( *this );
      return copy;
    }
    using BufferableCounter<Arithmetic, Atomicity, Counter>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      // Avoid printing empty counters in non DEBUG mode
      auto fmt = ( tableFormat ? "|%|10d| |" : "#=%|-7lu|" );
      return o << boost::format{fmt} % this->nEntries();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    bool       toBePrinted() const override { return this->nEntries() > 0; }
  };

  /**
   * A counter aiming at computing sum and average
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct AveragingCounter : BufferableCounter<Arithmetic, Atomicity, AveragingCounter>,
                            AveragingAccumulator<Arithmetic, Atomicity> {
    using BufferableCounter<Arithmetic, Atomicity, AveragingCounter>::BufferableCounter;
    using BufferableCounter<Arithmetic, Atomicity, AveragingCounter>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.7g| |%|#11.5g| |" : "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g|" );
      return o << boost::format{fmt} % this->nEntries() % this->sum() % this->mean();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }

    bool toBePrinted() const override { return this->nEntries() > 0; }
  };
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  using SummingCounter = AveragingCounter<Arithmetic, Atomicity>;

  /**
   * A counter aiming at computing average and sum2 / variance / standard deviation
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct SigmaCounter : BufferableCounter<Arithmetic, Atomicity, SigmaCounter>,
                        SigmaAccumulator<Arithmetic, Atomicity> {
    using BufferableCounter<Arithmetic, Atomicity, SigmaCounter>::BufferableCounter;
    using BufferableCounter<Arithmetic, Atomicity, SigmaCounter>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |"
                               : "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g|" );
      return o << boost::format{fmt} % this->nEntries() % this->sum() % this->mean() % this->standard_deviation();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    bool       toBePrinted() const override { return this->nEntries() > 0; }
  };

  /**
   * A counter aiming at computing average and sum2 / variance / standard deviation
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct StatCounter : BufferableCounter<Arithmetic, Atomicity, StatCounter>, StatAccumulator<Arithmetic, Atomicity> {
    using BufferableCounter<Arithmetic, Atomicity, StatCounter>::BufferableCounter;
    using BufferableCounter<Arithmetic, Atomicity, StatCounter>::print;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |"
                               : "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g| Min/Max=%|#10.4g|/%|-#10.4g|" );
      return o << boost::format{fmt} % this->nEntries() % this->sum() % this->mean() % this->standard_deviation() %
                      this->min() % this->max();
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    bool       toBePrinted() const override { return this->nEntries() > 0; }
  };

  /**
   * A counter dealing with binomial data
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
  struct BinomialCounter : BufferableCounter<Arithmetic, Atomicity, BinomialCounter>,
                           BinomialAccumulator<Arithmetic, Atomicity> {
    using BufferableCounter<Arithmetic, Atomicity, BinomialCounter>::BufferableCounter;

    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      auto fmt = ( tableFormat ? "|%|10d| |%|11.5g| |(%|#9.7g| +- %|-#8.7g|)%% |"
                               : "#=%|-7lu| Sum=%|-11.5g| Eff=|(%|#9.7g| +- %|-#8.6g|)%%|" );
      return o << boost::format{fmt} % this->nEntries() % this->nTrueEntries() % ( this->efficiency() * 100 ) %
                      ( this->efficiencyErr() * 100 );
    }

    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }

    template <typename stream>
    stream& printImpl( stream& o, const std::string& tag ) const {
      // override default print to add a '*' in from of the name
      o << boost::format{" |*%|-48.48s|%|50t|"} % ( "\"" + tag + "\"" );
      return print( o, true );
    }
    /// prints the counter to a stream in table format, with the given tag
    std::ostream& print( std::ostream& o, const std::string& tag ) const override { return printImpl( o, tag ); }
    MsgStream&    print( MsgStream& o, const std::string& tag ) const override { return printImpl( o, tag ); }
    bool          toBePrinted() const override { return this->nEntries() > 0; }
  };

  namespace details::MsgCounter {
    struct Logger {
      CommonMessagingBase const* parent = nullptr;
      std::string                msg;
      MSG::Level                 lvl;

      void operator()( bool suppress = false ) const {
        if ( suppress ) {
          parent->msgStream( lvl ) << "Suppressing message: " << std::quoted( msg, '\'' ) << endmsg;
        } else {
          parent->msgStream( lvl ) << msg << endmsg;
        }
      }
    };

    template <atomicity Atomicity>
    struct Data {
      std::conditional_t<Atomicity == atomicity::none, unsigned long, std::atomic<unsigned long>> count  = {0};
      unsigned long                                                                               max    = 0;
      Logger const*                                                                               logger = nullptr;

      constexpr Data( Logger const* p, unsigned long mx ) : max{mx}, logger{p} {}

      template <atomicity OtherAtomicity>
      Data( Data<OtherAtomicity> const& other ) {
        count  = other.count;
        max    = other.max;
        logger = other.logger;
      }
    };

    template <atomicity Atomicity>
    struct Handler {
      using InternalType = Data<Atomicity>;
      using OutputType   = Data<atomicity::none>;

      static constexpr OutputType getValue( InternalType const& v ) noexcept { return v; }

      static void merge( InternalType& orig, bool b ) {
        if ( b ) {
          auto count = ++orig.count;
          if ( UNLIKELY( count <= orig.max ) ) ( *orig.logger )( count == orig.max );
        }
      }
    };
    struct OutputTransform {
      template <atomicity Atomicity>
      constexpr unsigned long operator()( Data<Atomicity> const& v ) const noexcept {
        return v.count;
      }
    };
  } // namespace details::MsgCounter

  template <MSG::Level level, atomicity Atomicity = atomicity::full>
  struct MsgCounter
      : PrintableCounter,
        details::MsgCounter::Logger,
        GenericAccumulator<bool, details::MsgCounter::Data<Atomicity>, Atomicity, Identity,
                           details::MsgCounter::OutputTransform, details::MsgCounter::Handler<Atomicity>> {
    template <typename OWNER>
    MsgCounter( OWNER* o, std::string const& msg, int nMax = 10 )
        : PrintableCounter( o, msg )
        , details::MsgCounter::Logger{o, msg, level}
        , GenericAccumulator<bool, details::MsgCounter::Data<Atomicity>, Atomicity, Identity,
                             details::MsgCounter::OutputTransform, details::MsgCounter::Handler<Atomicity>>{
              std::in_place, this, nMax} {}
    MsgCounter& operator++() {
      ( *this ) += true;
      return *this;
    }
    template <typename stream>
    stream& printImpl( stream& o, bool tableFormat ) const {
      return o << boost::format{tableFormat ? "|%|10d| |" : "#=%|-7lu|"} % this->value();
    }
    using PrintableCounter::print;
    std::ostream& print( std::ostream& os, bool tableFormat ) const override { return printImpl( os, tableFormat ); }
    MsgStream&    print( MsgStream& os, bool tableFormat ) const override { return printImpl( os, tableFormat ); }
    bool          toBePrinted() const override { return this->value() > 0; }
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
