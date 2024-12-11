/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <Gaudi/MonitoringHub.h>
#include <GaudiKernel/HistoDef.h>

#include <array>
#include <cassert>
#include <cmath>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
  // Helper class creating a "subtuple" type from a tuple type by keeping only
  // the first N items.
  template <typename Tuple, typename Seq>
  struct SubTuple;
  template <typename Tuple, size_t... I>
  struct SubTuple<Tuple, std::index_sequence<I...>> {
    using type = decltype( std::make_tuple( std::get<I>( std::declval<Tuple>() )... ) );
  };
  template <typename Tuple, unsigned int N>
  using SubTuple_t = typename SubTuple<Tuple, std::make_index_sequence<N>>::type;

  /// helper class to create a tuple of N identical types
  template <typename T, unsigned int ND, typename = std::make_integer_sequence<unsigned int, ND>>
  struct make_tuple;
  template <typename T, unsigned int ND, unsigned int... S>
  struct make_tuple<T, ND, std::integer_sequence<unsigned int, S...>> {
    template <unsigned int>
    using typeMap = T;
    using type    = std::tuple<typeMap<S>...>;
  };
  template <typename T, unsigned int ND>
  using make_tuple_t = typename make_tuple<T, ND>::type;

  /// template magic converting a tuple of Axis into the tuple of corresponding Arithmetic types
  template <typename AxisTupleType>
  struct AxisToArithmetic;
  template <typename... Axis>
  struct AxisToArithmetic<std::tuple<Axis...>> {
    using type = std::tuple<typename Axis::ArithmeticType...>;
  };
  template <typename AxisTupleType>
  using AxisToArithmetic_t = typename AxisToArithmetic<AxisTupleType>::type;
  template <typename ProfArithmetic, typename AxisTupleType>
  using ProfileAxisToArithmetic_t = decltype( std::tuple_cat( std::declval<AxisToArithmetic_t<AxisTupleType>>(),
                                                              std::declval<std::tuple<ProfArithmetic>>() ) );
} // namespace

namespace Gaudi::Accumulators {

  namespace details {
    inline void requireValidTitle( std::string_view sv ) {
      if ( !sv.empty() && ( std::isspace( sv.back() ) || std::isspace( sv.front() ) ) ) {
        throw GaudiException(
            fmt::format( "Histogram title \'{}\' has whitespace at front or back -- please remove", sv ),
            "Gaudi::Accumulators", StatusCode::FAILURE );
      }
    }
  } // namespace details

  /**
   * A functor to extract weight, take a pair (valueTuple, weight) as input
   */
  struct ExtractWeight {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<unsigned long, Arithmetic>& v ) const noexcept {
      return v.second;
    }
  };

  /**
   * A Product functor, take a pair (value, weight) as input
   */
  struct WeightedProduct {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<Arithmetic, Arithmetic>& v ) const noexcept {
      return v.first * v.second;
    }
  };

  /**
   * A WeightedSquare functor, take a pair (value, weight) as input
   */
  struct WeightedSquare {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<Arithmetic, Arithmetic>& v ) const noexcept {
      return v.first * v.first * v.second;
    }
  };

  /**
   * An inputTransform for WeightedProfile histograms, keeping weight and replacing value by 1
   */
  struct WeightedProfileTransform {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<Arithmetic, Arithmetic>& v ) const noexcept {
      return std::pair<unsigned int, Arithmetic>{ 1ul, v.second };
    }
  };

  /**
   * An Adder ValueHandler, taking weight into account and computing a count plus the sum of the weights
   * In case of full atomicity, fetch_add or compare_exchange_weak are used for each element,
   * that is we do not have full atomicity accross the two elements
   */
  template <typename Arithmetic, atomicity Atomicity>
  struct WeightedAdder {
    using RegularType              = std::pair<unsigned long, Arithmetic>;
    using AtomicType               = std::pair<std::atomic<unsigned long>, std::atomic<Arithmetic>>;
    using OutputType               = RegularType;
    static constexpr bool isAtomic = Atomicity == atomicity::full;
    using InternalType             = std::conditional_t<isAtomic, AtomicType, OutputType>;
    static constexpr OutputType getValue( const InternalType& v ) noexcept {
      if constexpr ( isAtomic ) {
        return { v.first.load( std::memory_order_relaxed ), v.second.load( std::memory_order_relaxed ) };
      } else {
        return v;
      }
    };
    static RegularType exchange( InternalType& v, RegularType newv ) noexcept {
      if constexpr ( isAtomic ) {
        return { v.first.exchange( newv.first ), v.second.exchange( newv.second ) };
      } else {
        return { std::exchange( v.first, newv.first ), std::exchange( v.second, newv.second ) };
      }
    }
    static constexpr OutputType DefaultValue() { return { 0, Arithmetic{} }; }
    static void                 merge( InternalType& a, RegularType b ) noexcept {
      if constexpr ( isAtomic ) {
        fetch_add( a.first, b.first );
        fetch_add( a.second, b.second );
      } else {
        a.first += b.first;
        a.second += b.second;
      }
    };
  };

  /**
   * WeightedCountAccumulator. A WeightedCountAccumulator is an Accumulator storing the number of provided values,
   * as well as the weighted version of it, aka. the sum of weights. It takes a pair (valueTuple, weight) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct WeightedCountAccumulator
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, std::pair<unsigned long, Arithmetic>, Atomicity,
                           WeightedProfileTransform, ExtractWeight, WeightedAdder<Arithmetic, Atomicity>> {
    using Base = GenericAccumulator<std::pair<Arithmetic, Arithmetic>, std::pair<unsigned long, Arithmetic>, Atomicity,
                                    WeightedProfileTransform, ExtractWeight, WeightedAdder<Arithmetic, Atomicity>>;
    using Base::Base;
    using Base::operator+=;
    /// overload of operator+= to be able to only give weight and no value
    WeightedCountAccumulator operator+=( const Arithmetic weight ) {
      *this += { 1ul, weight };
      return *this;
    }
    unsigned long nEntries() const { return this->rawValue().first; }
    Arithmetic    sumOfWeights() const { return this->rawValue().second; }
  };

  /**
   * WeightedSumAccumulator. A WeightedSumAccumulator is an Accumulator storing a weighted sum of values.
   * It takes a pair (valueTuple, weight) and basically sums the product of the last item othe 2 part of its in put pair
   * : weight and value
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct WeightedSumAccumulator
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedProduct> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity,
                             WeightedProduct>::GenericAccumulator;
    Arithmetic sum() const { return this->value(); }
  };

  /**
   * WeightedSquareAccumulator. A WeightedSquareAccumulator is an Accumulator storing a weighted sum of squared values.
   * It basically takes a pair (value, weight) as input and sums weight*value*value
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct WeightedSquareAccumulator
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedSquare> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity,
                             WeightedSquare>::GenericAccumulator;
    Arithmetic sum2() const { return this->value(); };
  };

  /**
   * WeightedAveragingAccumulator. An WeightedAveragingAccumulator is an Accumulator able to compute an average
   * This implementation takes a pair (value, weight) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using WeightedAveragingAccumulator =
      AveragingAccumulatorBase<Atomicity, Arithmetic, WeightedCountAccumulator, WeightedSumAccumulator>;

  /**
   * WeightedSigmaAccumulator. A WeightedSigmaAccumulator is an Accumulator able to compute an average and variance/rms
   * This implementation takes a pair (value, weight) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using WeightedSigmaAccumulator =
      SigmaAccumulatorBase<Atomicity, Arithmetic, WeightedAveragingAccumulator, WeightedSquareAccumulator>;

  /**
   * Definition of a default type of Histogram Axis
   * It contains number of bins, min and max value plus a title
   * and defines the basic type of Axis (non log)
   * It may also contain labels for the bins
   */
  template <typename Arithmetic>
  class Axis {
  public:
    using ArithmeticType = Arithmetic;
    Axis( unsigned int nBins = 0, Arithmetic minValue = Arithmetic{}, Arithmetic maxValue = Arithmetic{},
          std::string title = {}, std::vector<std::string> labels = {} )
        : m_title( std::move( title ) )
        , nBins( nBins )
        , m_minValue( minValue )
        , m_maxValue( maxValue )
        , m_labels( std::move( labels ) ) {
      details::requireValidTitle( m_title );
      recomputeRatio();
      for ( const auto& s : m_labels ) details::requireValidTitle( s );
    };
    explicit Axis( Gaudi::Histo1DDef const& def )
        : Axis( (unsigned int)def.bins(), def.lowEdge(), def.highEdge(), def.title() ){};

    /// returns the bin number for a given value, ranging from 0 (underflow) to nBins+1 (overflow)
    unsigned int index( Arithmetic value ) const {
      // In case we use integer as Arithmetic type, we cannot use ratio for computing indices,
      // as ratios < 1.0 will simply be 0, so we have to pay the division in such a case
      int idx;
      if constexpr ( std::is_integral_v<Arithmetic> ) {
        idx = ( ( value - m_minValue ) * nBins / ( m_maxValue - m_minValue ) ) + 1;
      } else {
        idx = std::floor( ( value - m_minValue ) * m_ratio ) + 1;
      }
      return idx < 0 ? 0 : ( (unsigned int)idx > numBins() ? numBins() + 1 : (unsigned int)idx );
    }

    friend std::ostream& operator<<( std::ostream& o, Axis const& axis ) {
      // Note that we print python code here, as the generic toStream implementation uses this
      // operator to generate python code.
      o << "(" << axis.numBins() << ", " << axis.minValue() << ", " << axis.maxValue() << ", "
        << "\"" << axis.m_title << "\", (";
      for ( auto const& label : axis.m_labels ) { o << "\"" << label << "\", "; }
      return o << "))";
    }

    /// says whether the given value is within the range of the axis
    bool inAcceptance( Arithmetic value ) const { return value >= m_minValue && value <= m_maxValue; }

    // accessors
    unsigned int numBins() const { return nBins; };
    void         setNumBins( unsigned int n ) {
      nBins = n;
      recomputeRatio();
    }
    Arithmetic minValue() const { return m_minValue; };
    void       setMinValue( Arithmetic v ) {
      m_minValue = v;
      recomputeRatio();
    }
    Arithmetic maxValue() const { return m_maxValue; };
    void       setMaxValue( Arithmetic v ) {
      m_maxValue = v;
      recomputeRatio();
    }
    std::string const&              title() const { return m_title; }
    void                            setTitle( std::string const& t ) { m_title = t; };
    std::vector<std::string> const& labels() const { return m_labels; }

  private:
    /// title of this axis
    std::string m_title;

  public:
    /// number of bins for this Axis
    /// FIXME : should be private and called m_nBins but will break backward compatibility with previous implementation.
    unsigned int nBins;

  private:
    /// min and max values on this axis
    Arithmetic m_minValue, m_maxValue;
    /**
     * precomputed ratio to convert a value into bin number
     * equal to nBins/(maxValue-minValue). Only used for floating Arithmetic
     */
    Arithmetic m_ratio;
    /// labels for the bins
    std::vector<std::string> m_labels;

    void recomputeRatio() {
      m_ratio = ( m_maxValue == m_minValue ) ? Arithmetic{} : nBins / ( m_maxValue - m_minValue );
    }
  };

  /// automatic conversion of the Axis type to json
  template <typename Arithmetic>
  void to_json( nlohmann::json& j, const Axis<Arithmetic>& axis ) {
    j = nlohmann::json{ { "nBins", axis.numBins() },
                        { "minValue", axis.minValue() },
                        { "maxValue", axis.maxValue() },
                        { "title", axis.title() } };
    if ( !axis.labels().empty() ) { j["labels"] = axis.labels(); }
  }

  /**
   * small class used as InputType for regular Histograms
   * basically a tuple of the given values, specialized in case of a single
   * entry so that the syntax is more natural.
   * NIndex should be lower than number of Arithmetic types and denotes the
   * number of items used as index. There can typically be one more type in
   * the list for profile histograms, not use as index on an axis
   * ValueType is the actual type used to fill the histogram, that is
   * the ArithmeticTuple reduced to NIndex items
   *
   * Note : the specialization is only needed to ensure backward compatibility
   * with previous implementation where there was only one Arithmetic type common
   * to all axis. It should in principal be the one and only implementation
   * FIXME : remove specialization when client code was adapted
   */
  template <typename Arithmetic, unsigned int NIndex>
  struct HistoInputType : HistoInputType<make_tuple_t<Arithmetic, NIndex>, NIndex> {
    using HistoInputType<make_tuple_t<Arithmetic, NIndex>, NIndex>::HistoInputType;
  };
  template <unsigned int NIndex, typename... Elements>
  struct HistoInputType<std::tuple<Elements...>, NIndex> : std::tuple<Elements...> {
    using InternalType = std::tuple<Elements...>;
    using ValueType    = HistoInputType<SubTuple_t<InternalType, NIndex>, NIndex>;
    using std::tuple<Elements...>::tuple;
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == NIndex )>>
    unsigned int computeIndex( std::tuple<AxisType...> const& axis ) const {
      return computeIndexInternal<0, std::tuple<AxisType...>>( axis );
    }
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == NIndex )>>
    static unsigned int computeTotNBins( std::tuple<AxisType...> const& axis ) {
      return computeTotNBinsInternal<0, std::tuple<AxisType...>>( axis );
    }
    auto forInternalCounter() const { return 1ul; }
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == NIndex )>>
    bool inAcceptance( std::tuple<AxisType...> const& axis ) const {
      return inAcceptanceInternal<0, std::tuple<AxisType...>>( axis );
    }

  private:
    template <int N, class Tuple>
    unsigned int computeIndexInternal( Tuple const& allAxis ) const {
      // compute global index. Bins are stored in a column first manner
      auto const&  axis       = std::get<N>( allAxis );
      unsigned int localIndex = axis.index( std::get<N>( *this ) );
      if constexpr ( N + 1 == NIndex )
        return localIndex;
      else
        return localIndex + ( axis.numBins() + 2 ) * computeIndexInternal<N + 1, Tuple>( allAxis );
    }
    template <int N, class Tuple>
    static unsigned int computeTotNBinsInternal( Tuple const& allAxis ) {
      auto const&  axis       = std::get<N>( allAxis );
      unsigned int localNBins = axis.numBins() + 2;
      if constexpr ( N + 1 == NIndex )
        return localNBins;
      else
        return localNBins * computeTotNBinsInternal<N + 1, Tuple>( allAxis );
    }
    template <int N, class Tuple>
    bool inAcceptanceInternal( Tuple const& allAxis ) const {
      auto const& axis        = std::get<N>( allAxis );
      bool        localAnswer = axis.inAcceptance( std::get<N>( *this ) );
      if constexpr ( N + 1 == NIndex )
        return localAnswer;
      else
        return localAnswer || inAcceptanceInternal<N + 1, Tuple>( allAxis );
    }
  };

  /**
   * small class used as InputType for weighted Histograms
   * only a pair of the InnerType and the weight.
   * See description of HistoInputType for more details
   */
  template <typename ArithmeticTuple, unsigned int NIndex, typename WArithmetic>
  struct WeightedHistoInputType : std::pair<HistoInputType<ArithmeticTuple, NIndex>, WArithmetic> {
    using ValueType = typename HistoInputType<ArithmeticTuple, NIndex>::ValueType;
    using std::pair<HistoInputType<ArithmeticTuple, NIndex>, WArithmetic>::pair;
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == NIndex )>>
    unsigned int computeIndex( std::tuple<AxisType...> const& axis ) const {
      return this->first.computeIndex( axis );
    }
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == NIndex )>>
    static unsigned int computeTotNBins( std::tuple<AxisType...> const& axis ) {
      return HistoInputType<ArithmeticTuple, NIndex>::computeTotNBins( axis );
    }
    auto forInternalCounter() const { return std::pair( this->first.forInternalCounter(), this->second ); }
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == NIndex )>>
    bool inAcceptance( std::tuple<AxisType...> const& axis ) const {
      return this->first.inAcceptance( axis );
    }
  };

  /**
   * Internal Accumulator class dealing with Histograming. Templates parameters are :
   *  - Atomicity : none or full
   *  - InputType : a class holding a value given as input of the Histogram,
   *    and able to answer questions on this value given a number of axis matching
   *    the type of value.
   *    e.g. it would hold a pair of double for a non weighted 2D histogram or
   *    a pair of triplet of doubles and double for the weighted 3D histogram.
   *    Example implementations are (Weighted)HistoInputType.
   *    This class must define :
   *      + a constructor taking a set of value to build the InputType
   *      + a static method `unsigned int computeTotNBins( std::tuple<AxisType...> const& )`
   *        able to compute the total number of bins needed with this input type and
   *        these axis. It will typically be the product of the number of bins for each
   *        dimension, potentially increased by 2 for each if underflow and overflow
   *        is supported
   *      + a type ValueType alias defining the type of the input values to give to InputType
   *        This type needs to implement :
   *        * a method
   *          `unsigned int computeIndex( std::tuple<AxisType...> const& ) const`
   *          able to compute the bin corresponding to a given value
   *        * a method `auto forInternalCounter() const` returning the value to be used to
   *          inscrease the accumulator dealing with the bin associated with the current value.
   *          In most simple cases, it return `Arithmetic{}` or even `1` but for weighted
   *          histograms, it returns a pair with the weight as second item
   *        * in case of usage within a RootHistogram, it should also define a method
   *          `bool inAcceptance( std::tuple<AxisType...> const& )` checking whether a given
   *          value in within the range of the accumulator
   *  - Arithmetic : the arithmetic type used for values stored inside the histogram
   *    e.g. unsigned int for regular histogram as we only count entries, or float/double
   *    for weighted histograms, as we store actual sums of original values
   *  - BaseAccumulator : the underlying accumulator used in each bin
   *  - AxisTupleType : the types of the axis as a tuple. Its length defines the dimension
   *    of the Histogram this accumulator handles.
   *    The constraints on the AxisType are : FIXME use concepts when available
   *      + that they can be copied
   *      + that they have a ArithmeticType type alias
   *      + that they have a `unsigned int numBins() const` method
   *      + that they have a friend operator<< using std::ostream for printing
   *      + that they have a friend to_json method using nlohmann library
   *      + that they implement whatever is needed by the computeIndex and computeTotNBins methods
   *         of the InputType used. Plus the inAcceptance one if Roothistograms are used
   *    A default Axis class is provided for most cases
   * This accumulator is simply an array of BaseAccumulator, one per bin.
   */
  template <atomicity Atomicity, typename InputType, typename Arithmetic,
            template <atomicity Ato, typename Arith> typename BaseAccumulatorT, typename AxisTupleType>
  class HistogramingAccumulatorInternal {
    template <atomicity, typename, typename, template <atomicity, typename> typename, typename>
    friend class HistogramingAccumulatorInternal;

  public:
    using ND                      = std::integral_constant<unsigned int, std::tuple_size_v<AxisTupleType>>;
    using BaseAccumulator         = BaseAccumulatorT<Atomicity, Arithmetic>;
    using AxisTupleArithmeticType = typename InputType::ValueType;
    HistogramingAccumulatorInternal( AxisTupleType axis )
        : m_axis{ axis }
        , m_totNBins{ InputType::computeTotNBins( m_axis ) }
        , m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    template <atomicity ato>
    HistogramingAccumulatorInternal(
        construct_empty_t,
        const HistogramingAccumulatorInternal<ato, InputType, Arithmetic, BaseAccumulatorT, AxisTupleType>& other )
        : m_axis( other.m_axis ), m_totNBins{ other.m_totNBins }, m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    [[deprecated( "Use `++h1[x]`, `++h2[{x,y}]`, etc. instead." )]] HistogramingAccumulatorInternal&
    operator+=( InputType v ) {
      accumulator( v.computeIndex( m_axis ) ) += v.forInternalCounter();
      return *this;
    }
    void reset() {
      for ( unsigned int index = 0; index < m_totNBins; index++ ) { accumulator( index ).reset(); }
    }
    template <atomicity ato>
    void mergeAndReset(
        HistogramingAccumulatorInternal<ato, InputType, Arithmetic, BaseAccumulatorT, AxisTupleType>& other ) {
      assert( m_totNBins == other.m_totNBins );
      for ( unsigned int index = 0; index < m_totNBins; index++ ) {
        accumulator( index ).mergeAndReset( other.accumulator( index ) );
      }
    }
    [[nodiscard]] auto operator[]( typename InputType::ValueType v ) {
      return Buffer<BaseAccumulatorT, Atomicity, Arithmetic>{ accumulator( v.computeIndex( m_axis ) ) };
    }

    template <unsigned int N>
    auto& axis() const {
      return std::get<N>( m_axis );
    }
    auto& axis() const { return m_axis; }
    auto  binValue( unsigned int i ) const { return accumulator( i ).value(); }
    auto  nEntries( unsigned int i ) const { return accumulator( i ).nEntries(); }
    auto  totNBins() const { return m_totNBins; }

    // FIXME These methods are there for backwrad compatibility with previous implementation
    // where all Axis had to be of type Axis<...> and were stored in an array
    // Newer code should call axis<N>().foo for whatever foo is defined in that axis type
    auto nBins( unsigned int i ) const { return _getAxis( i, std::integral_constant<size_t, 0>() ).numBins(); }
    auto minValue( unsigned int i ) const { return _getAxis( i, std::integral_constant<size_t, 0>() ).minValue(); }
    auto maxValue( unsigned int i ) const { return _getAxis( i, std::integral_constant<size_t, 0>() ).maxValue(); }

  private:
    BaseAccumulator& accumulator( unsigned int index ) const {
      assert( index < m_totNBins );
      assert( m_value.get() );
      return m_value[index];
    }

    // FIXME Only used for backward compatibility. should be dropped at some stage
    // Can only work if all axis have same type, which is no more the case
    std::tuple_element_t<0, AxisTupleType> const& _getAxis( size_t i,
                                                            typename std::tuple_size<AxisTupleType>::type ) const {
      throw std::logic_error(
          fmt::format( "Retrieving axis {} in Histogram of dimension {}", i, std::tuple_size_v<AxisTupleType> ) );
    }
    template <size_t N, typename = std::enable_if_t<std::tuple_size_v<AxisTupleType> != N>>
    auto& _getAxis( size_t i, std::integral_constant<size_t, N> ) const {
      if ( i == N ) return std::get<N>( m_axis );
      return _getAxis( i, std::integral_constant<size_t, N + 1>() );
    }

    /// set of Axis of this Histogram
    AxisTupleType m_axis;
    /// total number of bins in this histogram, under and overflow included
    unsigned int m_totNBins{};
    /// Histogram content
    std::unique_ptr<BaseAccumulator[]> m_value;
  };

  /**
   * Class implementing a regular histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND, typename AxisTupleType>
  using HistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, HistoInputType<AxisToArithmetic_t<AxisTupleType>, ND::value>,
                                      unsigned long, IntegralAccumulator, AxisTupleType>;

  /**
   * Class implementing a weighted histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND, typename AxisTupleType>
  using WeightedHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity,
                                      WeightedHistoInputType<AxisToArithmetic_t<AxisTupleType>, ND::value, Arithmetic>,
                                      Arithmetic, WeightedCountAccumulator, AxisTupleType>;

  /**
   * Class implementing a profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND, typename AxisTupleType>
  using ProfileHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity,
                                      HistoInputType<ProfileAxisToArithmetic_t<Arithmetic, AxisTupleType>, ND::value>,
                                      Arithmetic, SigmaAccumulator, AxisTupleType>;

  /**
   * Class implementing a weighted profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND, typename AxisTupleType>
  using WeightedProfileHistogramingAccumulator = HistogramingAccumulatorInternal<
      Atomicity, WeightedHistoInputType<ProfileAxisToArithmetic_t<Arithmetic, AxisTupleType>, ND::value, Arithmetic>,
      Arithmetic, WeightedSigmaAccumulator, AxisTupleType>;

  /**
   * A base counter dealing with Histograms
   *
   * Main features of that Counter :
   *  - can be any number of dimensions. The dimension is its first template parameter
   *  - for each dimension, an Axis is associated. Axis can be of any type depending
   *    on the underlying accumulator
   *  - the constructor expects one extra argument per axis, typically a tuple
   *    of values allowing to create the Axis objects in the back
   *  - the operator+= takes either an array of values (one per dimension)
   *    or a tuple<array of values, weight>. The value inside the bin
   *    corresponding to the given values is then increased by 1 or weight
   *  - the prefered syntax is to avoid operator+= and use operator[] to get a
   *    buffer on the bin you're updating. Syntax becomes :
   *        ++counter[{x,y}]   or   wcounter[{x,y}] += w
   *  - the Counter is templated by the types of the values given to
   *    operator+ and also by the type stored into the bins
   *  - the counter can be atomic or not and supports buffering. Note that
   *    the atomicity is classical eventual consistency. So each bin is
   *    atomically updated but bins are not garanted to be coherent when
   *    reading all of them back
   *  - profile histograms are also supported, operator+= takes one more
   *    value in the array of values in that case
   *
   * This base class is then aliases for the 4 standard cases of Histogram,
   * WeightedHistogram, ProfileHistogram and WeightedProfileHistogram.
   * For all predefined Histogram types, the axis type is a simple triplet
   * of values nbins, minValue, maxValue plus a title.
   *
   * Typical usage :
   * \code
   * Histogram<2, double, atomicity::full>
   *   counter{owner, "CounterName", "HistoTitle", {{nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2,
   * "AxisTitle"}}};
   * ++counter[{val1, val2}];    // prefered syntax
   * counter += {val1, val2};    // original syntax inherited from counters
   *
   * WeightedHistogram<2, double, atomicity::full>
   *   wcounter{owner, "CounterName", "HistoTitle", {{nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2,
   * "AxisTitle"}}}; wcounter[{val1, val2}] += w;    // prefered syntax wcounter += {{val1, val2}, w};  // original
   * syntax inherited from counters \endcode
   *
   * When serialized to json, this counter uses new types histogram:Histogram:<prec>, histogram:ProfileHistogram:<prec>,
   * histogram:WeightedHistogram:<prec> and histrogram:WeightedProfileHistogram:<prec>
   * <prec> id described in Accumulators.h and decribes here the precision of the bin data.
   * All these types have the same fields, namely :
   *   dimension(integer), title(string), empty(bool), nEntries(integer), axis(array), bins(array)
   * where :
   *     + axis is an array of tuples, one per dimension, with content (nBins(integer), minValue(number),
   * maxValue(number), title(string)) for the default type of Axis
   *     + bins is an array of values
   *       - The length of the array is the product of (nBins+2) for all axis
   *       - the +2 is because the bin 0 is the one for values below minValue and bin nBins+1 is the one for values
   * above maxValue bins are stored row first so we iterate first on highest dimension
   *       - the value is a number for non profile histograms
   *       - the value is of the form ( (nEntries(integer), sum(number) ), sum2(number) ) for profile histograms
   *         Note the pair with a pair as first entry
   */
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type,
            template <atomicity, typename, typename, typename> typename Accumulator, typename AxisTupleType>
  class HistogramingCounterBase;
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type,
            template <atomicity, typename, typename, typename> typename Accumulator, typename... AxisTypes>
  class HistogramingCounterBase<ND, Atomicity, Arithmetic, Type, Accumulator, std::tuple<AxisTypes...>>
      : public BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<unsigned int, ND>,
                                 std::tuple<AxisTypes...>> {
  public:
    using AxisTupleType    = std::tuple<AxisTypes...>;
    using NumberDimensions = std::integral_constant<unsigned int, ND>;
    using Parent           = BufferableCounter<Atomicity, Accumulator, Arithmetic, NumberDimensions, AxisTupleType>;
    using AccumulatorType  = Accumulator<Atomicity, Arithmetic, NumberDimensions, AxisTupleType>;
    using AxisTupleArithmeticType = typename AccumulatorType::AxisTupleArithmeticType;
    /// for backward compatibility with previous implementation, should not be used FIXME
    using AxisArithmeticType = typename std::tuple_element<0, AxisTupleType>::type::ArithmeticType;
    inline static const std::string typeString{ std::string{ Type } + ':' + typeid( Arithmetic ).name() };
    /// This constructor takes the axis as a tuple
    template <typename OWNER>
    HistogramingCounterBase( OWNER* owner, std::string const& name, std::string const& title, AxisTupleType axis )
        : Parent( owner, name, *this, axis ), m_title( title ) {
      details::requireValidTitle( m_title );
    }
    /// This constructor takes the axis one by one, when ND >= 2. If ND = 1, the other one can be used
    template <typename OWNER>
    HistogramingCounterBase( OWNER* owner, std::string const& name, std::string const& title, AxisTypes... allAxis )
        : HistogramingCounterBase( owner, name, title, std::make_tuple( allAxis... ) ) {}
    using Parent::print;
    template <typename stream>
    stream& printImpl( stream& o, bool /*tableFormat*/ ) const {
      o << ND << "D Histogram with config ";
      std::apply( [&o]( auto&&... args ) { ( ( o << args << "\n" ), ... ); }, this->axis() );
      return o;
    }
    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream&   print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    friend void  reset( HistogramingCounterBase& c ) { c.reset(); }
    friend void  mergeAndReset( HistogramingCounterBase& h, HistogramingCounterBase& o ) { h.mergeAndReset( o ); }
    friend void  to_json( nlohmann::json& j, HistogramingCounterBase const& h ) { h.to_json( j ); }
    virtual void to_json( nlohmann::json& j ) const {
      // get all bin values and compute total nbEntries
      std::vector<typename AccumulatorType::BaseAccumulator::OutputType> bins;
      bins.reserve( this->totNBins() );
      unsigned long totNEntries{ 0 };
      for ( unsigned int i = 0; i < this->totNBins(); i++ ) {
        bins.push_back( this->binValue( i ) );
        totNEntries += this->nEntries( i );
      }
      // build json
      j = { { "type", std::string( Type ) + ":" + typeid( Arithmetic ).name() },
            { "title", m_title },
            { "dimension", ND },
            { "empty", totNEntries == 0 },
            { "nEntries", totNEntries },
            { "axis", this->axis() },
            { "bins", bins } };
    }
    std::string const& title() const { return m_title; }

  protected:
    std::string const m_title;
  };

  namespace naming {
    inline constexpr char histogramString[]                = "histogram:Histogram";
    inline constexpr char weightedHistogramString[]        = "histogram:WeightedHistogram";
    inline constexpr char profilehistogramString[]         = "histogram:ProfileHistogram";
    inline constexpr char weightedProfilehistogramString[] = "histogram:WeightedProfileHistogram";
  } // namespace naming

  /// standard static histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using StaticHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::histogramString,
                                                  HistogramingAccumulator, AxisTupleType>;

  /// standard static histograming counter with weight. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using StaticWeightedHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::weightedHistogramString,
                                                          WeightedHistogramingAccumulator, AxisTupleType>;

  /// profile static histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using StaticProfileHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::profilehistogramString,
                                                         ProfileHistogramingAccumulator, AxisTupleType>;

  /// weighted static profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using StaticWeightedProfileHistogram =
      HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::weightedProfilehistogramString,
                              WeightedProfileHistogramingAccumulator, AxisTupleType>;

} // namespace Gaudi::Accumulators
