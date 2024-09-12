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
#include <Gaudi/MonitoringHub.h>
#include <GaudiKernel/HistoDef.h>

#include <array>
#include <cmath>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Gaudi::Accumulators {

  namespace details {
    template <std::size_t, typename T>
    using alwaysT = T;
    // get a tuple of n types the given Type, or directly the type for n = 1
    template <typename Type, unsigned int ND>
    struct GetTuple;
    template <typename Type, unsigned int ND>
    using GetTuple_t = typename GetTuple<Type, ND>::type;
    template <typename Type, unsigned int ND>
    struct GetTuple {
      using type =
          decltype( std::tuple_cat( std::declval<std::tuple<Type>>(), std::declval<GetTuple_t<Type, ND - 1>>() ) );
    };
    template <typename Type>
    struct GetTuple<Type, 1> {
      using type = std::tuple<Type>;
    };

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
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, std::pair<unsigned long, Arithmetic>, Atomicity, Identity,
                           ExtractWeight, WeightedAdder<Arithmetic, Atomicity>> {
    using Base = GenericAccumulator<std::pair<Arithmetic, Arithmetic>, std::pair<unsigned long, Arithmetic>, Atomicity,
                                    Identity, ExtractWeight, WeightedAdder<Arithmetic, Atomicity>>;
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
   * Definition of an Histogram Axis
   */
  template <typename Arithmetic>
  struct Axis {
    Axis( unsigned int _nBins, Arithmetic _minValue, Arithmetic _maxValue, std::string _title = {},
          std::vector<std::string> _labels = {} )
        : nBins( _nBins )
        , minValue( _minValue )
        , maxValue( _maxValue )
        , title( std::move( _title ) )
        , labels( std::move( _labels ) )
        , ratio( _nBins / ( _maxValue - _minValue ) ) {
      details::requireValidTitle( title );
      for ( const auto& s : labels ) details::requireValidTitle( s );
    };
    explicit Axis( Gaudi::Histo1DDef const& def )
        : Axis( (unsigned int)def.bins(), def.lowEdge(), def.highEdge(), def.title() ){};
    /// number of bins for this Axis
    unsigned int nBins;
    /// min and max values on this axis
    Arithmetic minValue, maxValue;
    /// title of this axis
    std::string title;
    /// labels for the bins
    std::vector<std::string> labels;
    /**
     * precomputed ratio to convert a value into bin number
     * equal to nBins/(maxValue-minValue). Only used for floating Arithmetic
     */
    Arithmetic ratio;

    /// returns the bin number for a given value, ranging from 0 (underflow) to nBins+1 (overflow)
    unsigned int index( Arithmetic value ) const {
      // In case we use integer as Arithmetic type, we cannot use ratio for computing indices,
      // as ratios < 1.0 will simply be 0, so we have to pay the division in such a case
      int idx;
      if constexpr ( std::is_integral_v<Arithmetic> ) {
        idx = ( ( value - minValue ) * nBins / ( maxValue - minValue ) ) + 1;
      } else {
        idx = std::floor( ( value - minValue ) * ratio ) + 1;
      }
      return idx < 0 ? 0 : ( (unsigned int)idx > nBins ? nBins + 1 : (unsigned int)idx );
    }
    /// says whether the given value is within the range of the axis
    bool inAcceptance( Arithmetic value ) const { return value >= minValue && value <= maxValue; }
  };

  /// automatic conversion of the Axis type to json
  template <typename Arithmetic>
  void to_json( nlohmann::json& j, const Axis<Arithmetic>& axis ) {
    j = nlohmann::json{ { "nBins", axis.nBins },
                        { "minValue", axis.minValue },
                        { "maxValue", axis.maxValue },
                        { "title", axis.title } };
    if ( !axis.labels.empty() ) { j["labels"] = axis.labels; }
  }

  /// small class used as InputType for regular Histograms
  template <typename Arithmetic, unsigned int ND, unsigned int NIndex = ND>
  struct HistoInputType : std::array<Arithmetic, ND> {
    // allow construction from set of values
    template <class... ARGS, typename = typename std::enable_if_t<( sizeof...( ARGS ) == NIndex )>>
    HistoInputType( ARGS... args ) : std::array<Arithmetic, ND>{ static_cast<Arithmetic>( args )... } {}
    // The change on NIndex == 1 allow to have simpler syntax in that case, that is no tuple of one item
    using ValueType          = HistoInputType<Arithmetic, NIndex == 1 ? 1 : ND, NIndex>;
    using AxisArithmeticType = Arithmetic;
    using InternalType       = std::array<Arithmetic, ND>;
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      unsigned int index = 0;
      for ( unsigned int j = 0; j < NIndex; j++ ) {
        unsigned int dim = NIndex - j - 1;
        // compute local index for a given dimension
        int localIndex = axis[dim].index( ( *this )[dim] );
        // compute global index. Bins are stored in a column first manner
        index *= ( axis[dim].nBins + 2 );
        index += localIndex;
      }
      return index;
    }
    bool inAcceptance( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      for ( unsigned int dim = 0; dim < NIndex; dim++ ) {
        if ( !axis[dim].inAcceptance( ( *this )[dim] ) ) return false;
      }
      return true;
    }
    auto forInternalCounter() { return 1ul; }
    template <typename AxisType, long unsigned NAxis>
    static unsigned int computeTotNBins( std::array<AxisType, NAxis> axis ) {
      unsigned int nTotBins = 1;
      for ( unsigned int i = 0; i < NAxis; i++ ) { nTotBins *= ( axis[i].nBins + 2 ); }
      return nTotBins;
    }
  };

  /// specialization of HistoInputType for ND == 1 in order to have simpler syntax
  /// that is, no tuple of one item
  template <typename Arithmetic>
  class HistoInputType<Arithmetic, 1> {
  public:
    using ValueType          = HistoInputType;
    using AxisArithmeticType = Arithmetic;
    using InternalType       = Arithmetic;
    HistoInputType( Arithmetic a ) : value( a ) {}
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, 1>& axis ) const { return axis[0].index( value ); }
    bool inAcceptance( const std::array<Axis<Arithmetic>, 1>& axis ) const { return axis[0].inAcceptance( value ); }
    Arithmetic& operator[]( int ) { return value; }
    operator Arithmetic() const { return value; }
    auto forInternalCounter() { return 1ul; }
    template <typename AxisType>
    static unsigned int computeTotNBins( std::array<AxisType, 1> axis ) {
      return axis[0].nBins + 2;
    }

  private:
    Arithmetic value;
  };

  /// small class used as InputType for weighted Histograms
  template <typename Arithmetic, unsigned int ND, unsigned int NIndex = ND>
  struct WeightedHistoInputType : std::pair<HistoInputType<Arithmetic, ND, NIndex>, Arithmetic> {
    // The change on NIndex == 1 allow to have simpler syntax in that case, that is no tuple of one item
    using ValueType          = HistoInputType<Arithmetic, NIndex == 1 ? 1 : ND, NIndex>;
    using AxisArithmeticType = Arithmetic;
    using std::pair<HistoInputType<Arithmetic, ND, NIndex>, Arithmetic>::pair;
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      return this->first.computeIndex( axis );
    }
    unsigned int inAcceptance( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      return this->first.inAcceptance( axis );
    }
    auto forInternalCounter() { return std::pair( this->first.forInternalCounter(), this->second ); }
    template <typename AxisType, long unsigned NAxis>
    static unsigned int computeTotNBins( std::array<AxisType, NAxis> axis ) {
      return HistoInputType<Arithmetic, ND, NIndex>::computeTotNBins( axis );
    }
  };

  /**
   * Internal Accumulator class dealing with Histograming. Templates parameters are :
   *  - Atomicity : none or full
   *  - Arithmetic : the arithmetic type used for values filled into the histogram
   *  - ND : the number of dimensions of the histogram.
   *    Note that ND is given as an integral_constant as it needs to be a type for the internal template
   *    business of the Counter (more precisely the Buffer class)
   *  - InputType : the type given as input of the Histogram. Typically (Weighted)HistoInputType
   *  - BaseAccumulator : the underlying accumulator used in each bin
   *
   * This accumulator is simply an array of BaseAccumulator, one per bin. The number of bins is
   * the product of the number of bins for each dimension, each of them increased by 2 for storing
   * the values under min and above max
   */
  template <atomicity Atomicity, typename InputType, typename Arithmetic, typename ND,
            template <atomicity Ato, typename Arith> typename BaseAccumulatorT>
  class HistogramingAccumulatorInternal {
    template <atomicity, typename, typename, typename, template <atomicity, typename> typename>
    friend class HistogramingAccumulatorInternal;

  public:
    using BaseAccumulator    = BaseAccumulatorT<Atomicity, Arithmetic>;
    using AxisArithmeticType = typename InputType::AxisArithmeticType;
    using AxisType           = Axis<AxisArithmeticType>;
    template <std::size_t... Is>
    HistogramingAccumulatorInternal( details::GetTuple_t<AxisType, ND::value> axis, std::index_sequence<Is...> )
        : m_axis{ std::get<Is>( axis )... }
        , m_totNBins{ InputType::computeTotNBins( m_axis ) }
        , m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    template <atomicity ato>
    HistogramingAccumulatorInternal(
        construct_empty_t,
        const HistogramingAccumulatorInternal<ato, InputType, Arithmetic, ND, BaseAccumulatorT>& other )
        : m_axis( other.m_axis )
        , m_totNBins{ InputType::computeTotNBins( m_axis ) }
        , m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    [[deprecated( "Use `++h1[x]`, `++h2[{x,y}]`, etc. instead." )]] HistogramingAccumulatorInternal&
    operator+=( InputType v ) {
      accumulator( v.computeIndex( m_axis ) ) += v.forInternalCounter();
      return *this;
    }
    void reset() {
      for ( unsigned int index = 0; index < m_totNBins; index++ ) accumulator( index ).reset();
    }
    template <atomicity ato>
    void mergeAndReset( HistogramingAccumulatorInternal<ato, InputType, Arithmetic, ND, BaseAccumulatorT>& other ) {
      assert( m_totNBins == other.m_totNBins );
      for ( unsigned int index = 0; index < m_totNBins; index++ ) {
        accumulator( index ).mergeAndReset( other.accumulator( index ) );
      }
    }
    [[nodiscard]] auto operator[]( typename InputType::ValueType v ) {
      return Buffer<BaseAccumulatorT, Atomicity, Arithmetic>{ accumulator( v.computeIndex( m_axis ) ) };
    }

    auto& axis() const { return m_axis; }
    auto  nBins( unsigned int i ) const { return m_axis[i].nBins; }
    auto  minValue( unsigned int i ) const { return m_axis[i].minValue; }
    auto  maxValue( unsigned int i ) const { return m_axis[i].maxValue; }
    auto  binValue( unsigned int i ) const { return accumulator( i ).value(); }
    auto  nEntries( unsigned int i ) const { return accumulator( i ).nEntries(); }
    auto  totNBins() const { return m_totNBins; }

  private:
    BaseAccumulator& accumulator( unsigned int index ) const {
      assert( index < m_totNBins );
      return m_value[index];
    }
    /// set of Axis of this Histogram
    std::array<AxisType, ND::value> m_axis;
    /// total number of bins in this histogram, under and overflow included
    unsigned int m_totNBins;
    /// Histogram content
    std::unique_ptr<BaseAccumulator[]> m_value;
  };

  /**
   * Class implementing a regular histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using HistogramingAccumulator = HistogramingAccumulatorInternal<Atomicity, HistoInputType<Arithmetic, ND::value>,
                                                                  unsigned long, ND, IntegralAccumulator>;

  /**
   * Class implementing a weighted histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using WeightedHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, WeightedHistoInputType<Arithmetic, ND::value>, Arithmetic, ND,
                                      WeightedCountAccumulator>;

  /**
   * Class implementing a profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using ProfileHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, HistoInputType<Arithmetic, ND::value + 1, ND::value>, Arithmetic, ND,
                                      SigmaAccumulator>;

  /**
   * Class implementing a weighted profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using WeightedProfileHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, WeightedHistoInputType<Arithmetic, ND::value + 1, ND::value>,
                                      Arithmetic, ND, WeightedSigmaAccumulator>;

  /**
   * A base counter dealing with Histograms
   *
   * Main features of that Counter :
   *  - can be any number of dimensions. The dimension is its first template parameter
   *  - for each dimension, a triplet of values have to be given at
   *    construction : nbins, minValue, maxValue. These triplets have to be
   *    embedded into braces, as the constructor takes an array of them
   *  - the operator+= takes either an array of values (one per dimension)
   *    or a tuple<array of values, weight>. The value inside the bin
   *    corresponding to the given values is then increased by 1 or weight
   *  - the prefered syntax is to avoid operator+= and use operator[] to get a
   *    buffer on the bin you're updating. Syntax becomes :
   *        ++counter[{x,y}]   or   wcounter[{x,y]] += w
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
   * WeightedHistogram, ProfileHistogram and WeightedProfileHistogram
   *
   * Typical usage :
   * \code
   * Histogram<2, double, atomicity::full>
   *   counter{owner, "CounterName", "HistoTitle", {{nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2}}};
   * ++counter[{val1, val2}];    // prefered syntax
   * counter += {val1, val2};    // original syntax inherited from counters
   *
   * WeightedHistogram<2, double, atomicity::full>
   *   wcounter{owner, "CounterName", "HistoTitle", {{nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2}}};
   * wcounter[{val1, val2}] += w;    // prefered syntax
   * wcounter += {{val1, val2}, w};  // original syntax inherited from counters
   * \endcode
   *
   * When serialized to json, this counter uses new types histogram:Histogram:<prec>, histogram:ProfileHistogram:<prec>,
   * histogram:WeightedHistogram:<prec> and histrogram:WeightedProfileHistogram:<prec>
   * <prec> id described in Accumulators.h and decribes here the precision of the bin data.
   * All these types have the same fields, namely :
   *   dimension(integer), title(string), empty(bool), nEntries(integer), axis(array), bins(array)
   * where :
   *     + axis is an array of tuples, one per dimension, with content (nBins(integer), minValue(number),
   * maxValue(number), title(string))
   *     + bins is an array of values
   *       - The length of the array is the product of (nBins+2) for all axis
   *       - the +2 is because the bin 0 is the one for values below minValue and bin nBins+1 is the one for values
   * above maxValue bins are stored row first so we iterate first on highest dimension
   *       - the value is a number for non profile histograms
   *       - the value is of the form ( (nEntries(integer), sum(number) ), sum2(number) ) for profile histograms
   *         Note the pair with a pair as first entry
   */
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type,
            template <atomicity, typename, typename> typename Accumulator, typename Seq>
  class HistogramingCounterBaseInternal;
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type,
            template <atomicity, typename, typename> typename Accumulator, std::size_t... NDs>
  class HistogramingCounterBaseInternal<ND, Atomicity, Arithmetic, Type, Accumulator, std::index_sequence<NDs...>>
      : public BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<int, ND>> {
  public:
    using Parent           = BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<int, ND>>;
    using AccumulatorType  = Accumulator<Atomicity, Arithmetic, std::integral_constant<int, ND>>;
    using NumberDimensions = std::integral_constant<unsigned int, ND>;
    inline static const std::string typeString{ std::string{ Type } + ':' + typeid( Arithmetic ).name() };
    template <typename OWNER>
    HistogramingCounterBaseInternal( OWNER* owner, std::string const& name, std::string const& title,
                                     details::GetTuple_t<typename AccumulatorType::AxisType, ND> axis )
        : Parent( owner, name, *this, axis, std::make_index_sequence<ND>{} ), m_title( title ) {
      details::requireValidTitle( m_title );
    }
    template <typename OWNER>
    HistogramingCounterBaseInternal( OWNER* owner, std::string const& name, std::string const& title,
                                     details::alwaysT<NDs, typename AccumulatorType::AxisType>... allAxis )
        : HistogramingCounterBaseInternal( owner, name, title, std::make_tuple( allAxis... ) ) {}
    using Parent::print;
    template <typename stream>
    stream& printImpl( stream& o, bool /*tableFormat*/ ) const {
      o << ND << "D Histogram with config ";
      for ( unsigned int i = 0; i < ND; i++ ) {
        o << this->nBins( i ) << " " << this->minValue( i ) << " " << this->maxValue( i ) << " ";
      }
      return o;
    }
    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream&  print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    friend void reset( HistogramingCounterBaseInternal& c ) { c.reset(); }
    friend void mergeAndReset( HistogramingCounterBaseInternal& h, HistogramingCounterBaseInternal& o ) {
      h.mergeAndReset( o );
    }
    friend void  to_json( nlohmann::json& j, HistogramingCounterBaseInternal const& h ) { h.to_json( j ); }
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
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type,
            template <atomicity, typename, typename> typename Accumulator>
  using HistogramingCounterBase =
      HistogramingCounterBaseInternal<ND, Atomicity, Arithmetic, Type, Accumulator, std::make_index_sequence<ND>>;

  namespace naming {
    inline constexpr char histogramString[]                = "histogram:Histogram";
    inline constexpr char weightedHistogramString[]        = "histogram:WeightedHistogram";
    inline constexpr char profilehistogramString[]         = "histogram:ProfileHistogram";
    inline constexpr char weightedProfilehistogramString[] = "histogram:WeightedProfileHistogram";
  } // namespace naming
  /// standard histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using Histogram =
      HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::histogramString, HistogramingAccumulator>;

  /// standard histograming counter with weight. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using WeightedHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::weightedHistogramString,
                                                    WeightedHistogramingAccumulator>;

  /// profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using ProfileHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::profilehistogramString,
                                                   ProfileHistogramingAccumulator>;

  /// weighted profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using WeightedProfileHistogram =
      HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::weightedProfilehistogramString,
                              WeightedProfileHistogramingAccumulator>;

} // namespace Gaudi::Accumulators
