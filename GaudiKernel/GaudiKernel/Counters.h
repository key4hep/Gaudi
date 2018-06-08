#ifndef GAUDIKERNEL_COUNTERS_H
#define GAUDIKERNEL_COUNTERS_H 1

/**
 * This file provides efficient counter implementations for Gaudi.
 *
 * A number of concepts and templated classes are defined in this file
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
#include <initializer_list>
#include <iostream>
#include <limits>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "GaudiKernel/apply.h"
#include "GaudiKernel/detected.h"

namespace Gaudi
{
  namespace Accumulators
  {

    /// Defines atomicity of the accumulators
    enum class atomicity { none, full };

    /**
     * An functor always returning the value N
     */
    template <unsigned long N>
    struct Constant {
      template <typename U>
      constexpr unsigned long operator()( U&& ) const noexcept
      {
        return N;
      }
    };

    /**
     * An Identity functor
     */
    struct Identity {
      template <typename U>
      constexpr decltype( auto ) operator()( U&& v ) const noexcept
      {
        return std::forward<U>( v );
      }
    };

    /**
     * A Square functor
     */
    struct Square {
      template <typename U>
      constexpr decltype( auto ) operator()( U&& v ) const noexcept
      {
        return v * v;
      }
    };

    /**
     * type_traits for checking the presence of fetch_add in std::atomic<T>
     */
    template <typename T, typename = int>
    using has_fetch_add_ = decltype( std::atomic<T>{}.fetch_add( 0 ) );
    template <typename T>
    using has_fetch_add = typename Gaudi::cpp17::is_detected<has_fetch_add_, T>::value_t;

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
      static constexpr OutputType getValue( const InternalType& v ) noexcept
      {
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
      static void merge( InternalType& a, Arithmetic b ) noexcept { a += b; };
    };

    /**
     * Adder specialization in the case of atomicity full
     */
    template <typename Arithmetic>
    struct Adder<Arithmetic, atomicity::full> : BaseValueHandler<Arithmetic, atomicity::full> {
      using typename BaseValueHandler<Arithmetic, atomicity::full>::OutputType;
      using typename BaseValueHandler<Arithmetic, atomicity::full>::InternalType;
      static constexpr OutputType DefaultValue() { return Arithmetic{}; }
#if __cplusplus > 201402L
      static void merge( InternalType& a, Arithmetic b ) noexcept
      {
        if ( DefaultValue() == b ) return; // avoid atomic operation if b is "0"
        // C++ 17 version
        if
          constexpr( has_fetch_add<InternalType>::value ) { a.fetch_add( b, std::memory_order_relaxed ); }
        else {
          auto current = BaseValueHandler<Arithmetic, atomicity::full>::getValue( a );
          while ( !a.compare_exchange_weak( current, current + b ) )
            ;
        }
      };
#else
      // C++11 version
    private:
      template <typename T>
      static void add( InternalType& a, T b, std::false_type )
      {
        auto current = a.load( std::memory_order_relaxed );
        while ( !a.compare_exchange_weak( current, current + b ) )
          ;
      }
      template <typename T>
      static void add( InternalType& a, T b, std::true_type )
      {
        a.fetch_add( b, std::memory_order_relaxed );
      }

    public:
      static void merge( InternalType& a, Arithmetic b ) noexcept
      {
        if ( DefaultValue() == b ) return; // avoid atomic operation if b is "0"
        add( a, b, has_fetch_add<InternalType>{} );
      }
#endif
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
      static void merge( InternalType& a, Arithmetic b ) noexcept
      {
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
      static void merge( InternalType& a, Arithmetic b ) noexcept
      {
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
    class GenericAccumulator
    {
      template <typename A, typename B, atomicity C, typename D, typename E, typename F>
      friend class GenericAccumulator;

    public:
      using OutputType = std::decay_t<std::result_of_t<OutputTransform( InnerType )>>;
      GenericAccumulator operator+=( const InputType by )
      {
        ValueHandler::merge( m_value, InputTransform{}( by ) );
        return *this;
      }
      GenericAccumulator() : m_value( ValueHandler::DefaultValue() ) {}
      GenericAccumulator( const GenericAccumulator& other ) : m_value( ValueHandler::getValue( other.m_value ) ) {}
      GenericAccumulator& operator=( const GenericAccumulator& other )
      {
        m_value = ValueHandler::getValue( other.m_value );
        return *this;
      }
      OutputType          value() const { return OutputTransform{}( ValueHandler::getValue( m_value ) ); }
      void                reset() { reset( ValueHandler::DefaultValue() ); }
      template <atomicity ato, typename VH>
      void mergeAndReset( GenericAccumulator<InputType, InnerType, ato, InputTransform, OutputTransform, VH>&& other )
      {
        auto otherValue = VH::exchange( other.m_value, VH::DefaultValue() );
        ValueHandler::merge( m_value, otherValue );
      }

    protected:
      void reset( InnerType in ) { m_value = std::move( in ); }
    private:
      typename ValueHandler::InternalType m_value;
    };

    /**
     * AccumulatorSet is an Accumulator that holds a set of Accumulators templated by same Arithmetic and Atomicity
     * and increase them altogether
     */
    template <typename Arithmetic, atomicity Atomicity, template <typename, atomicity> class... Bases>
    class AccumulatorSet : public Bases<Arithmetic, Atomicity>...
    {
    public:
      using InputType            = Arithmetic;
      using OutputType           = std::tuple<typename Bases<Arithmetic, Atomicity>::OutputType...>;
      constexpr AccumulatorSet() = default;
      AccumulatorSet& operator+=( const InputType by )
      {
        (void)std::initializer_list<int>{( Bases<Arithmetic, Atomicity>::operator+=( by ), 0 )...};
        return *this;
      }
      OutputType          value() const { return std::make_tuple( Bases<Arithmetic, Atomicity>::value()... ); }
      void                reset() { (void)std::initializer_list<int>{( Bases<Arithmetic, Atomicity>::reset(), 0 )...}; }
      template <atomicity Ato>
      void mergeAndReset( AccumulatorSet<Arithmetic, Ato, Bases...>&& other )
      {
        (void)std::initializer_list<int>{
            ( Bases<Arithmetic, Atomicity>::mergeAndReset( static_cast<Bases<Arithmetic, Ato>&&>( other ) ), 0 )...};
      }

    protected:
      void reset( const std::tuple<typename Bases<Arithmetic, Atomicity>::OutputType...>& t )
      {
        Gaudi::apply(
            [this]( const auto&... i ) {
              (void)std::initializer_list<int>{( this->Bases<Arithmetic, Atomicity>::reset( i ), 0 )...};
            },
            t );
      }
    };

    /**
     * MaxAccumulator. A MaxAccumulator is an Accumulator storing the max value of the provided arguments
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct MaxAccumulator
        : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity, Maximum<Arithmetic, Atomicity>> {
      Arithmetic max() const { return this->value(); }
    };

    /**
     * MinAccumulator. A MinAccumulator is an Accumulator storing the min value of the provided arguments
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct MinAccumulator
        : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity, Identity, Minimum<Arithmetic, Atomicity>> {
      Arithmetic min() const { return this->value(); }
    };

    /**
     * CountAccumulator. A CountAccumulator is an Accumulator storing the number of provided values
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct CountAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, Constant<1>> {
      unsigned long nEntries() const { return this->value(); }
    };

    /**
     * SumAccumulator. A SumAccumulator is an Accumulator storing the sum of the provided values
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct SumAccumulator : GenericAccumulator<Arithmetic, Arithmetic, Atomicity, Identity> {
      Arithmetic sum() const { return this->value(); }
    };

    /**
     * SquareAccumulator. A SquareAccumulator is an Accumulator storing the sum of squares of the provided values
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
     */
    template <typename Arithmetic, atomicity Atomicity>
    struct TrueAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, TrueTo1> {
      unsigned long nTrueEntries() const { return this->value(); };
    };

    /// helper functor for the FalseAccumulator
    struct FalseTo1 {
      unsigned int operator()( bool v ) const { return !v; }
    };

    /**
     * FalseAccumulator. A FalseAccumulator is an Accumulator counting the number of False values in the data
     * data
     */
    template <typename Arithmetic, atomicity Atomicity>
    struct FalseAccumulator : GenericAccumulator<Arithmetic, unsigned long, Atomicity, FalseTo1> {
      unsigned long nFalseEntries() const { return this->value(); };
    };

    /**
     * BinomialAccumulator. A BinomialAccumulator is an Accumulator able to compute the efficiency of a process
     * data
     */
    template <typename Arithmetic, atomicity Atomicity = atomicity::full>
    struct BinomialAccumulator : AccumulatorSet<bool, Atomicity, TrueAccumulator, FalseAccumulator> {
      unsigned long nEntries() const { return this->nTrueEntries() + this->nFalseEntries(); };
      Arithmetic    efficiency() const
      {
        auto nbEntries = nEntries();
        if ( 1 > nbEntries ) return -1;
        return static_cast<Arithmetic>( this->nTrueEntries() ) / static_cast<Arithmetic>( nbEntries );
      }
      Arithmetic eff() const { return efficiency(); }
      Arithmetic efficiencyErr() const
      {
        // Note the usage of using, aiming at using the std version of sqrt by default, without preventing
        // more specialized versions to be used via ADL (see http://en.cppreference.com/w/cpp/language/adl)
        using std::sqrt;
        Arithmetic nbEntries = static_cast<Arithmetic>( nEntries() );
        if ( 1 > nbEntries ) return -1;
        return sqrt( static_cast<Arithmetic>( this->nTrueEntries() * this->nFalseEntries() ) / nbEntries ) / nbEntries;
      }
      Arithmetic effErr() const { return efficiencyErr(); }
    };

    /**
     * AveragingAccumulator. A AveragingAccumulator is an Accumulator able to compute an average
     */
    template <typename Arithmetic, atomicity Atomicity = atomicity::full>
    struct AveragingAccumulator : AccumulatorSet<Arithmetic, Atomicity, CountAccumulator, SumAccumulator> {
      Arithmetic mean() const
      {
        auto n = this->nEntries();
        return n ? this->sum() / n : Arithmetic{};
      }
    };

    /**
     * SigmaAccumulator. A SigmaAccumulator is an Accumulator able to compute an average and variance/rms
     */
    template <typename Arithmetic, atomicity Atomicity = atomicity::full>
    struct SigmaAccumulator : AccumulatorSet<Arithmetic, Atomicity, AveragingAccumulator, SquareAccumulator> {
      Arithmetic biased_sample_variance() const
      {
        auto count = this->nEntries();
        auto sum   = this->sum();
        return count ? ( this->sum2() - sum * ( sum / count ) ) / count : Arithmetic{};
      };
      Arithmetic unbiased_sample_variance() const
      {
        auto count = this->nEntries();
        auto sum   = this->sum();
        return count ? ( this->sum2() - sum * ( sum / count ) ) / ( count - 1 ) : Arithmetic{};
      };
      Arithmetic standard_deviation() const
      {
        // Note the usage of using, aiming at using the std version of sqrt by default, without preventing
        // more specialized versions to be used via ADL (see http://en.cppreference.com/w/cpp/language/adl)
        using std::sqrt;
        auto v = biased_sample_variance();
        return ( 0 > v ) ? Arithmetic{} : sqrt( v );
      }
      [[deprecated( "The name 'rms' has changed to standard_deviation" )]] Arithmetic rms() const
      {
        return standard_deviation();
      }
      Arithmetic meanErr() const
      {
        auto n = this->nEntries();
        if ( 0 == n ) return Arithmetic{};
        // Note the usage of using, aiming at using the std version of sqrt by default, without preventing
        // more specialized versions to be used via ADL (see http://en.cppreference.com/w/cpp/language/adl)
        using std::sqrt;
        auto v = biased_sample_variance();
        if ( 0 > v ) return Arithmetic{};
        return sqrt( v / n );
      }
    };

    /**
     * StatAccumulator. A StatAccumulator is an Accumulator able to compute an average, variance/rms and min/max
     */
    template <typename Arithmetic, atomicity Atomicity = atomicity::full>
    using StatAccumulator = AccumulatorSet<Arithmetic, Atomicity, SigmaAccumulator, MinAccumulator, MaxAccumulator>;

    /**
     * Buffer is a non atomic Accumulator which, when it goes out-of-scope,
     * updates the underlying thread-safe Accumulator for all previous updates in one go.
     * It is templated by the basic accumulator type and has same interface
     */
    template <typename Arithmetic, template <typename Int, atomicity Ato> class ContainedAccumulator>
    class Buffer : public ContainedAccumulator<Arithmetic, atomicity::none>
    {
      using prime_type = ContainedAccumulator<Arithmetic, atomicity::full>;
      using base_type  = ContainedAccumulator<Arithmetic, atomicity::none>;

    public:
      Buffer() = delete;
      Buffer( prime_type& p ) : base_type(), m_prime( p ) {}
      Buffer( const Buffer& ) = delete;
      void operator=( const Buffer& ) = delete;
      Buffer( Buffer&& other ) : base_type( other ), m_prime( other.m_prime ) { other.reset(); }
      void             push() { m_prime.mergeAndReset( static_cast<base_type&&>( *this ) ); }
      ~Buffer() { push(); }
    private:
      prime_type& m_prime;
    };

    /**
     * An empty ancester of all counters that knows how to print themselves
     */
    struct PrintableCounter {
      PrintableCounter() = default;
      template <class OWNER>
      PrintableCounter( OWNER* o, const std::string& tag )
      {
        o->registerCounter( tag, *this );
      }
      /// destructor
      virtual ~PrintableCounter() = default;
      /// prints the counter to a stream
      virtual std::ostream& print( std::ostream&, bool tableFormat = false ) const = 0;
      /// prints the counter to a stream in table format, with the given tag
      virtual std::ostream& print( std::ostream& o, const std::string& tag ) const
      {
        o << boost::format{" | %|-48.48s|%|50t|"} % ( "\"" + tag + "\"" );
        return print( o, true );
      }
      /// get a string representation
      std::string toString() const
      {
        std::ostringstream ost;
        print( ost );
        return ost.str();
      }
    };

    /**
     * external printout operator to std::ostream
     */
    inline std::ostream& operator<<( std::ostream& stream, const PrintableCounter& counter )
    {
      return counter.print( stream );
    }

    /**
     * An empty ancester of all counters that provides a buffer method that
     * returns a buffer on itself
     */
    template <typename Arithmetic, atomicity Atomicity, template <typename Int, atomicity Ato> class Accumulator>
    struct BufferableCounter : PrintableCounter {
      using PrintableCounter::PrintableCounter;
      Buffer<Arithmetic, Accumulator> buffer() { return {*static_cast<Accumulator<Arithmetic, Atomicity>*>( this )}; }
    };

    /**
     * A basic counter counting input values
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct Counter : BufferableCounter<Arithmetic, Atomicity, Counter>, CountAccumulator<Arithmetic, Atomicity> {
      using BufferableCounter<Arithmetic, Atomicity, Counter>::BufferableCounter;
      Counter& operator++()
      {
        ( *this ) += Arithmetic{};
        return *this;
      }
      Counter operator++( int )
      {
        auto copy = *this;
        ++( *this );
        return copy;
      }
      using BufferableCounter<Arithmetic, Atomicity, Counter>::print;
      std::ostream& print( std::ostream& o, bool tableFormat = false ) const override
      {
        std::string fmt( "#=%|-7lu|" );
        if ( tableFormat ) {
          fmt = "|%|7d| |";
        }
        o << boost::format{fmt} % this->nEntries();
        return o;
      }
    };

    /**
     * A counter aiming at computing sum and average
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct AveragingCounter : BufferableCounter<Arithmetic, Atomicity, AveragingCounter>,
                              AveragingAccumulator<Arithmetic, Atomicity> {
      using BufferableCounter<Arithmetic, Atomicity, AveragingCounter>::BufferableCounter;
      using BufferableCounter<Arithmetic, Atomicity, AveragingCounter>::print;
      std::ostream& print( std::ostream& o, bool tableFormat = false ) const override
      {
        std::string fmt;
        if ( tableFormat ) {
          fmt = "|%|7d| |%|11.7g| |%|#11.5g| |";
        } else {
          fmt = "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g|";
        }
        return o << boost::format{fmt} % this->nEntries() % this->sum() % this->mean();
      }
    };
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    using SummingCounter = AveragingCounter<Arithmetic, Atomicity>;

    /**
     * A counter aiming at computing average and sum2 / variance / standard deviation
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct SigmaCounter : BufferableCounter<Arithmetic, Atomicity, SigmaCounter>,
                          SigmaAccumulator<Arithmetic, Atomicity> {
      using BufferableCounter<Arithmetic, Atomicity, SigmaCounter>::BufferableCounter;
      using BufferableCounter<Arithmetic, Atomicity, SigmaCounter>::print;
      std::ostream& print( std::ostream& o, bool tableFormat = false ) const override
      {
        std::string fmt;
        if ( tableFormat ) {
          fmt = "|%|7d| |%|11.7g| |%|#11.5g| |%|#10.5g| |";
        } else {
          fmt = "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g|";
        }
        return o << boost::format{fmt} % this->nEntries() % this->sum() % this->mean() % this->standard_deviation();
      }
    };

    /**
     * A counter aiming at computing average and sum2 / variance / standard deviation
     */
    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct StatCounter : BufferableCounter<Arithmetic, Atomicity, StatCounter>, StatAccumulator<Arithmetic, Atomicity> {
      using BufferableCounter<Arithmetic, Atomicity, StatCounter>::BufferableCounter;
      using BufferableCounter<Arithmetic, Atomicity, StatCounter>::print;
      std::ostream& print( std::ostream& o, bool tableFormat = false ) const override
      {
        std::string fmt;
        if ( tableFormat ) {
          fmt = "|%|7d| |%|11.7g| |%|#11.5g| |%|#10.5g| |%|#10.5g| |%|#10.5g| |";
        } else {
          fmt = "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g| Min/Max=%|#10.4g|/%|-#10.4g|";
        }
        return o << boost::format{fmt} % this->nEntries() % this->sum() % this->mean() % this->standard_deviation() %
                        this->min() % this->max();
      }
    };

    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>
    struct BinomialCounter : BufferableCounter<bool, Atomicity, BinomialCounter>,
                             BinomialAccumulator<Arithmetic, Atomicity> {
      using BufferableCounter<bool, Atomicity, BinomialCounter>::BufferableCounter;
      std::ostream& print( std::ostream& o, bool tableFormat = false ) const override
      {
        std::string fmt;
        if ( tableFormat ) {
          fmt = "|%|7d| |%|11.5g| |(%|#9.7g| +- %|-#8.6g|)%%|";
        } else {
          fmt = "#=%|-7lu| Sum=%|-11.5g| Eff=|(%|#9.7g| +- %|-#8.6g|)%%|";
        }
        return o << boost::format{fmt} % this->nEntries() % this->nTrueEntries() % ( this->efficiency() * 100 ) %
                        ( this->efficiencyErr() * 100 );
      }
      /// prints the counter to a stream in table format, with the given tag
      virtual std::ostream& print( std::ostream& o, const std::string& tag ) const override
      {
        // override default print to add a '*' in from of the name
        o << boost::format{"*| %|-48.48s|%|50t|"} % ( "\"" + tag + "\"" );
        return print( o, true );
      }
    };

  } // namespace Accumulators

} // namespace Gaudi

/**
 * backward compatible StatEntity class. Should not be used.
 * Only here for backward compatibility
 */
class StatEntity : public Gaudi::Accumulators::PrintableCounter,
                   public Gaudi::Accumulators::AccumulatorSet<double, Gaudi::Accumulators::atomicity::full,
                                                              Gaudi::Accumulators::StatAccumulator,
                                                              Gaudi::Accumulators::BinomialAccumulator>
{
public:
  using AccParent = Gaudi::Accumulators::AccumulatorSet<double, Gaudi::Accumulators::atomicity::full,
                                                        Gaudi::Accumulators::StatAccumulator,
                                                        Gaudi::Accumulators::BinomialAccumulator>;
  using BinomialAccParent = Gaudi::Accumulators::BinomialAccumulator<double, Gaudi::Accumulators::atomicity::full>;
  using Gaudi::Accumulators::StatAccumulator<double, Gaudi::Accumulators::atomicity::full>::nEntries;
  using AccParent::reset;
  /// the constructor with automatic registration in the owner's counter map
  StatEntity() = default;
  template <class OWNER>
  StatEntity( OWNER* o, const std::string& tag )
  {
    o->registerCounter( tag, *this );
  }
  StatEntity( const unsigned long entries, const double flag, const double flag2, const double minFlag,
              const double maxFlag )
  {
    reset( std::make_tuple(
        std::make_tuple( std::make_tuple( std::make_tuple( entries, flag ), flag2 ), minFlag, maxFlag ),
        std::make_tuple( 0, 0 ) ) );
  }
  void reset() { AccParent::reset(); }
  void operator=( double by )
  {
    this->reset();
    ( *this ) += by;
  }
  StatEntity& operator-=( double by )
  {
    ( *this ) += ( -by );
    return *this;
  }
  StatEntity& operator++()
  {
    ( *this ) += 1.0;
    return *this;
  }
  StatEntity operator++( int )
  {
    auto copy = *this;
    ++( *this );
    return copy;
  }
  StatEntity& operator--()
  {
    ( *this ) += -1.0;
    return *this;
  }
  StatEntity operator--( int )
  {
    auto copy = *this;
    --( *this );
    return copy;
  }
  bool operator<( const StatEntity& se ) const
  {
    return std::make_tuple( nEntries(), sum(), min(), max(), sum2() ) <
           std::make_tuple( se.nEntries(), se.sum(), se.min(), se.max(), se.sum2() );
  };
  // using AccumulatorSet::operator+=;
  StatEntity& operator+=( double by )
  {
    this->AccumulatorSet::operator+=( by );
    return *this;
  }
  StatEntity& operator+=( StatEntity by )
  {
    mergeAndReset( std::move( by ) );
    return *this;
  }
  unsigned long add( const double v )
  {
    *this += v;
    return nEntries();
  }
  unsigned long addFlag( const double v ) { return add( v ); }
  // aliases (a'la ROOT)
  double Sum() const { return sum(); }                // get sum
  double Mean() const { return mean(); }              // get mean
  double MeanErr() const { return meanErr(); }        // get error in mean
  double rms() const { return standard_deviation(); } // get rms
  double Rms() const { return standard_deviation(); } // get rms
  double RMS() const { return standard_deviation(); } // get rms
  double Eff() const { return eff(); }                // get efficiency
  double Min() const { return min(); }                // get minimal value
  double Max() const { return max(); }                // get maximal value
  // some legacy methods, to be removed ...
  double      flag() const { return sum(); }
  double      flag2() const { return sum2(); }
  double      flagMean() const { return mean(); }
  double      flagRMS() const { return standard_deviation(); }
  double      flagMeanErr() const { return meanErr(); }
  double      flagMin() const { return min(); }
  double      flagMax() const { return max(); }
  static bool effCounter( const std::string& name )
  {
    using boost::algorithm::icontains;
    return icontains( name, "eff" ) || icontains( name, "acc" ) || icontains( name, "filt" ) ||
           icontains( name, "fltr" ) || icontains( name, "pass" );
  }
  std::ostream& printFormatted( std::ostream& o, const std::string& format ) const
  {
    boost::format fmt{format};
    fmt % nEntries() % sum() % mean() % standard_deviation() % min() % max();
    return o << fmt.str();
  }
  using Gaudi::Accumulators::PrintableCounter::print;
  std::ostream& print( std::ostream& o, bool tableFormat, const std::string& name, bool flag = true,
                       std::string fmtHead = "%|-48.48s|%|27t|" ) const
  {
    if ( flag && effCounter( name ) && 0 <= eff() && 0 <= effErr() && sum() <= nEntries() &&
         ( 0 == min() || 1 == min() ) && ( 0 == max() || 1 == max() ) ) {
      // efficiency printing
      std::string fmt;
      if ( tableFormat ) {
        if ( name.empty() ) {
          fmt = "|%|10d| |%|11.5g| |(%|#9.7g| +- %|-#8.7g|)%%|   -------   |   -------   |";
          return o << boost::format{fmt} % BinomialAccParent::nEntries() % sum() % ( efficiency() * 100 ) %
                          ( efficiencyErr() * 100 );
        } else {
          fmt = "*" + fmtHead + "|%|10d| |%|11.5g| |(%|#9.7g| +- %|-#8.7g|)%%|   -------   |   -------   |";
          return o << boost::format{fmt} % ( "\"" + name + "\"" ) % BinomialAccParent::nEntries() % sum() %
                          ( efficiency() * 100 ) % ( efficiencyErr() * 100 );
        }
      } else {
        fmt = "#=%|-7lu| Sum=%|-11.5g| Eff=|(%|#9.7g| +- %|-#8.6g|)%%|";
        return o << boost::format{fmt} % BinomialAccParent::nEntries() % sum() % ( efficiency() * 100 ) %
                        ( efficiencyErr() * 100 );
      }
    } else {
      // Standard printing
      std::string fmt;
      if ( tableFormat ) {
        if ( name.empty() ) {
          fmt = "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |";
          return o << boost::format{fmt} % nEntries() % sum() % mean() % standard_deviation() % min() % max();

        } else {
          fmt = " " + fmtHead + "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |";
          return o << boost::format{fmt} % ( "\"" + name + "\"" ) % nEntries() % sum() % mean() % standard_deviation() %
                          min() % max();
        }
      } else {
        fmt = "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g| Min/Max=%|#10.4g|/%|-#10.4g|";
        return o << boost::format{fmt} % nEntries() % sum() % mean() % standard_deviation() % min() % max();
      }
    }
  }
  virtual std::ostream& print( std::ostream& o, const std::string& tag ) const override
  {
    return print( o, true, tag, true );
  }
  std::ostream& print( std::ostream& o, bool tableFormat = false ) const override
  {
    std::string emptyName;
    return print( o, tableFormat, emptyName, true );
  }
  std::string toString() const
  {
    std::ostringstream ost;
    print( ost );
    return ost.str();
  }
  std::ostream& fillStream( std::ostream& o ) const { return print( o ); }
};

#endif // GAUDIKERNEL_COUNTERS_H
