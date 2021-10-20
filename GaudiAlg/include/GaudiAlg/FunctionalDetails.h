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
#ifndef FUNCTIONAL_DETAILS_H
#define FUNCTIONAL_DETAILS_H

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/detected.h"

// Range V3
#include <range/v3/version.hpp>
#include <range/v3/view/const.hpp>
#include <range/v3/view/zip.hpp>
// upstream has renamed namespace ranges::view ranges::views
#if RANGE_V3_VERSION < 900
namespace ranges::views {
  using namespace ranges::view;
}
#endif

#if defined( __clang__ ) && ( __clang_major__ < 11 ) || defined( __APPLE__ ) && ( __clang_major__ < 12 )
#  define GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN                                                                     \
    _Pragma( "clang diagnostic push" ) _Pragma( "clang diagnostic ignored \"-Wunused-lambda-capture\"" )
#  define GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END _Pragma( "clang diagnostic pop" )
#else
#  define GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
#  define GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
#endif

// temporary hack to help in transition to updated constructor
// allows to write code which is forward and backwards compatible
#define GAUDI_FUNCTIONAL_CONSTRUCTOR_USES_TUPLE

namespace Gaudi::Functional::details {

  // CRJ : Stuff for zipping
  namespace zip {

    /// Print the parameter
    template <typename OS, typename Arg>
    void printSizes( OS& out, Arg&& arg ) {
      out << "SizeOf'" << System::typeinfoName( typeid( Arg ) ) << "'=" << std::forward<Arg>( arg ).size();
    }

    /// Print the parameters
    template <typename OS, typename Arg, typename... Args>
    void printSizes( OS& out, Arg&& arg, Args&&... args ) {
      printSizes( out, arg );
      out << ", ";
      printSizes( out, args... );
    }

    /// Resolve case there is only one container in the range
    template <typename A>
    inline bool check_sizes( const A& ) noexcept {
      return true;
    }

    /// Compare sizes of two containers
    template <typename A, typename B>
    inline bool check_sizes( const A& a, const B& b ) noexcept {
      return a.size() == b.size();
    }

    /// Compare sizes of 3 or more containers
    template <typename A, typename B, typename... C>
    inline bool check_sizes( const A& a, const B& b, const C&... c ) noexcept {
      return ( check_sizes( a, b ) && check_sizes( b, c... ) );
    }

    /// Verify the data container sizes have the same sizes
    template <typename... Args>
    inline decltype( auto ) verifySizes( Args&... args ) {
      if ( UNLIKELY( !check_sizes( args... ) ) ) {
        std::ostringstream mess;
        mess << "Zipped containers have different sizes : ";
        printSizes( mess, args... );
        throw GaudiException( mess.str(), "Gaudi::Functional::details::zip::verifySizes", StatusCode::FAILURE );
      }
    }

    /// Zips multiple containers together to form a single range
    template <typename... Args>
    inline decltype( auto ) range( Args&&... args ) {
#ifndef NDEBUG
      verifySizes( args... );
#endif
      return ranges::views::zip( std::forward<Args>( args )... );
    }

    /// Zips multiple containers together to form a single const range
    template <typename... Args>
    inline decltype( auto ) const_range( Args&&... args ) {
#ifndef NDEBUG
      verifySizes( args... );
#endif
      return ranges::views::const_( ranges::views::zip( std::forward<Args>( args )... ) );
    }
  } // namespace zip

  /////////////////////////////////////////
  namespace details2 {
    // note: boost::optional in boost 1.66 does not have 'has_value()'...
    // that requires boost 1.68 or later... so for now, use operator bool() instead ;-(
    template <typename T>
    using is_optional_ = decltype( bool{ std::declval<T>() }, std::declval<T>().value() );
  } // namespace details2
  template <typename Arg>
  constexpr bool is_optional_v = Gaudi::cpp17::is_detected_v<details2::is_optional_, Arg>;

  template <typename Arg>
  using require_is_optional = std::enable_if_t<is_optional_v<Arg>>;

  template <typename Arg>
  using require_is_not_optional = std::enable_if_t<!is_optional_v<Arg>>;

  template <typename T>
  using remove_optional_t = std::conditional_t<is_optional_v<T>, typename T::value_type, T>;

  constexpr struct invoke_optionally_t {
    template <typename F, typename Arg, typename = require_is_not_optional<Arg>>
    decltype( auto ) operator()( F&& f, Arg&& arg ) const {
      return std::invoke( std::forward<F>( f ), std::forward<Arg>( arg ) );
    }
    template <typename F, typename Arg, typename = require_is_optional<Arg>>
    void operator()( F&& f, Arg&& arg ) const {
      if ( arg ) std::invoke( std::forward<F>( f ), *std::forward<Arg>( arg ) );
    }
  } invoke_optionally{};
  /////////////////////////////////////////

  template <typename Value, std::size_t... I>
  auto get_values_helper( std::index_sequence<I...> ) {
    return std::make_tuple( ( (void)I, Value{} )... );
  }

  template <typename Value, auto N>
  using RepeatValues_ = decltype( get_values_helper<Value>( std::make_index_sequence<N>() ) );

  /////////////////////////////////////////

  template <typename Out1, typename Out2,
            typename = std::enable_if_t<std::is_constructible_v<Out1, Out2> && std::is_base_of_v<DataObject, Out1>>>
  auto put( const DataObjectHandle<Out1>& out_handle, Out2&& out ) {
    return out_handle.put( std::make_unique<Out1>( std::forward<Out2>( out ) ) );
  }

  template <typename Out1, typename Out2, typename = std::enable_if_t<std::is_constructible_v<Out1, Out2>>>
  auto put( const DataObjectHandle<AnyDataWrapper<Out1>>& out_handle, Out2&& out ) {
    return out_handle.put( std::forward<Out2>( out ) );
  }

  // optional put
  template <typename OutHandle, typename OptOut, typename = require_is_optional<OptOut>>
  void put( const OutHandle& out_handle, OptOut&& out ) {
    if ( out ) put( out_handle, *std::forward<OptOut>( out ) );
  }
  /////////////////////////////////////////
  // adapt to differences between eg. std::vector (which has push_back) and KeyedContainer (which has insert)
  // adapt to getting a T, and a container wanting T* by doing new T{ std::move(out) }
  // adapt to getting a optional<T>

  constexpr struct insert_t {
    // for Container<T*>, return T
    template <typename Container>
    using c_remove_ptr_t = std::remove_pointer_t<typename Container::value_type>;

    template <typename Container, typename Value>
    auto operator()( Container& c, Value&& v ) const -> decltype( c.push_back( v ) ) {
      return c.push_back( std::forward<Value>( v ) );
    }

    template <typename Container, typename Value>
    auto operator()( Container& c, Value&& v ) const -> decltype( c.insert( v ) ) {
      return c.insert( std::forward<Value>( v ) );
    }

    // Container<T*> with T&& as argument
    template <typename Container, typename Value,
              typename = std::enable_if_t<std::is_pointer_v<typename Container::value_type>>,
              typename = std::enable_if_t<std::is_convertible_v<Value, c_remove_ptr_t<Container>>>>
    auto operator()( Container& c, Value&& v ) const {
      return operator()( c, new c_remove_ptr_t<Container>{ std::forward<Value>( v ) } );
    }

  } insert{};

  /////////////////////////////////////////

  constexpr struct deref_t {
    template <typename In, typename = std::enable_if_t<!std::is_pointer_v<In>>>
    const In& operator()( const In& in ) const {
      return in;
    }

    template <typename In, typename = std::enable_if_t<!std::is_pointer_v<std::decay_t<In>>>>
    In operator()( In&& in ) const {
      return std::forward<In>( in );
    }

    template <typename In>
    const In& operator()( const In* in ) const {
      assert( in != nullptr );
      return *in;
    }
  } deref{};

  /////////////////////////////////////////
  // if Container is a pointer, then we're optional items
  namespace details2 {
    template <typename T>
    struct is_gaudi_range : std::false_type {};

    template <typename T, typename IT>
    struct is_gaudi_range<Gaudi::Range_<T, IT>> : std::true_type {};

    template <typename T, typename IT>
    struct is_gaudi_range<Gaudi::NamedRange_<T, IT>> : std::true_type {};

    template <typename T>
    constexpr static bool is_gaudi_range_v = is_gaudi_range<T>::value;

    template <typename Container, typename Value>
    void push_back( Container& c, const Value& v, std::true_type ) {
      c.push_back( v );
    }
    template <typename Container, typename Value>
    void push_back( Container& c, const Value& v, std::false_type ) {
      c.push_back( &v );
    }

    template <typename In>
    struct get_from_handle {
      template <template <typename> class Handle, typename I, typename = std::enable_if_t<std::is_convertible_v<I, In>>>
      auto operator()( const Handle<I>& h ) -> const In& {
        return *h.get();
      }
      template <template <typename> class Handle, typename I, typename IT>
      auto operator()( const Handle<Gaudi::Range_<I, IT>>& h ) -> const In {
        return h.get();
      }
      template <template <typename> class Handle, typename I, typename IT>
      auto operator()( const Handle<Gaudi::NamedRange_<I, IT>>& h ) -> const In {
        return h.get();
      }
      template <template <typename> class Handle, typename I,
                typename = std::enable_if_t<std::is_convertible_v<I*, In>>>
      auto operator()( const Handle<I>& h ) -> const In {
        return h.getIfExists();
      } // In is-a pointer
    };

    template <typename T>
    T* deref_if( T* const t, std::false_type ) {
      return t;
    }
    template <typename T>
    T& deref_if( T* const t, std::true_type ) {
      return *t;
    }
  } // namespace details2

  template <typename Container>
  class vector_of_const_ {
    static constexpr bool is_pointer = std::is_pointer_v<Container>;
    static constexpr bool is_range   = details2::is_gaudi_range_v<Container>;
    using val_t                      = std::add_const_t<std::remove_pointer_t<Container>>;
    using ptr_t                      = std::add_pointer_t<val_t>;
    using ref_t                      = std::add_lvalue_reference_t<val_t>;
    using ContainerVector            = std::vector<std::conditional_t<is_range, std::remove_const_t<val_t>, ptr_t>>;
    ContainerVector m_containers;

  public:
    using value_type = std::conditional_t<is_pointer, ptr_t, val_t>;
    using size_type  = typename ContainerVector::size_type;
    class iterator {
      using it_t = typename ContainerVector::const_iterator;
      it_t m_i;
      friend class vector_of_const_;
      iterator( it_t iter ) : m_i( iter ) {}
      using ret_t = std::conditional_t<is_pointer, ptr_t, ref_t>;

    public:
      using iterator_category = typename it_t::iterator_category;
      using value_type        = typename it_t::iterator_category;
      using reference         = typename it_t::reference;
      using pointer           = typename it_t::pointer;
      using difference_type   = typename it_t::difference_type;

      friend bool operator!=( const iterator& lhs, const iterator& rhs ) { return lhs.m_i != rhs.m_i; }
      friend bool operator==( const iterator& lhs, const iterator& rhs ) { return lhs.m_i == rhs.m_i; }
      friend auto operator-( const iterator& lhs, const iterator& rhs ) { return lhs.m_i - rhs.m_i; }
      ret_t       operator*() const {
        if constexpr ( is_range ) {
          return *m_i;
        } else {
          return details2::deref_if( *m_i, std::bool_constant<!is_pointer>{} );
        }
      }
      iterator& operator++() {
        ++m_i;
        return *this;
      }
      iterator& operator--() {
        --m_i;
        return *this;
      }
      bool     is_null() const { return !*m_i; }
      explicit operator bool() const { return !is_null(); }
    };
    vector_of_const_() = default;
    void reserve( size_type size ) { m_containers.reserve( size ); }
    template <typename T> // , typename = std::is_convertible<T,std::conditional_t<is_pointer,ptr_t,val_t>>
    void push_back( T&& container ) {
      details2::push_back( m_containers, std::forward<T>( container ),
                           std::bool_constant < is_pointer or is_range > {} );
    } // note: does not copy its argument, so we're not really a container...
    iterator  begin() const { return m_containers.begin(); }
    iterator  end() const { return m_containers.end(); }
    size_type size() const { return m_containers.size(); }

    template <typename X = Container>
    std::enable_if_t<!std::is_pointer_v<X>, ref_t> operator[]( size_type i ) const {
      return *m_containers[i];
    }

    template <typename X = Container>
    std::enable_if_t<std::is_pointer_v<X>, ptr_t> operator[]( size_type i ) const {
      return m_containers[i];
    }

    template <typename X = Container>
    std::enable_if_t<!std::is_pointer_v<X>, ref_t> at( size_type i ) const {
      return *m_containers[i];
    }

    template <typename X = Container>
    std::enable_if_t<std::is_pointer_v<X>, ptr_t> at( size_type i ) const {
      return m_containers[i];
    }

    bool is_null( size_type i ) const { return !m_containers[i]; }
  };

  /////////////////////////////////////////
  namespace detail2 { // utilities for detected_or_t{,_} usage
    template <typename Tr>
    using BaseClass_t = typename Tr::BaseClass;
    template <typename Tr, typename T>
    using OutputHandle_t = typename Tr::template OutputHandle<T>;
    template <typename Tr, typename T>
    using InputHandle_t = typename Tr::template InputHandle<T>;

    template <typename T>
    using DefaultInputHandle =
        std::conditional_t<std::is_base_of_v<IAlgTool, T>, ToolHandle<T>, DataObjectReadHandle<T>>;
  } // namespace detail2

  // check whether Traits::BaseClass is a valid type,
  // if so, define BaseClass_t<Traits> as being Traits::BaseClass
  // else   define                     as being GaudiAlgorithm
  template <typename Tr>
  using BaseClass_t = Gaudi::cpp17::detected_or_t<GaudiAlgorithm, detail2::BaseClass_t, Tr>;

  // check whether Traits::{Input,Output}Handle<T> is a valid type,
  // if so, define {Input,Output}Handle_t<Traits,T> as being Traits::{Input,Output}Handle<T>
  // else   define                                  as being DataObject{Read,,Write}Handle<T>

  template <typename Tr, typename T>
  using OutputHandle_t = Gaudi::cpp17::detected_or_t<DataObjectWriteHandle<T>, detail2::OutputHandle_t, Tr, T>;
  template <typename Tr, typename T>
  using InputHandle_t = Gaudi::cpp17::detected_or_t<detail2::DefaultInputHandle<T>, detail2::InputHandle_t, Tr, T>;

  template <typename Traits>
  inline constexpr bool isLegacy =
      std::is_base_of_v<Gaudi::details::LegacyAlgorithmAdapter, details::BaseClass_t<Traits>>;

  /////////

  template <typename Handles>
  Handles make_vector_of_handles( IDataHandleHolder* owner, const std::vector<std::string>& init ) {
    Handles handles;
    handles.reserve( init.size() );
    std::transform( init.begin(), init.end(), std::back_inserter( handles ),
                    [&]( const std::string& loc ) -> typename Handles::value_type {
                      return { loc, owner };
                    } );
    return handles;
  }

  template <typename Handle, typename Algo>
  auto get( const Handle& handle, const Algo&, const EventContext& )
      -> decltype( details::deref( handle.get() ) ) // make it SFINAE friendly...
  {
    return details::deref( handle.get() );
  }

  template <typename Handle>
  auto getKey( const Handle& h ) -> decltype( h.objKey() ) {
    return h.objKey();
  }

  ///////////////////////
  // given a pack, return a corresponding tuple
  template <typename... In>
  struct filter_evtcontext_t {
    using type = std::tuple<In...>;

    static_assert( !std::disjunction_v<std::is_same<EventContext, In>...>,
                   "EventContext can only appear as first argument" );

    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, Handles& handles ) {
      return std::apply(
          [&]( const auto&... handle ) { return algo( get( handle, algo, Gaudi::Hive::currentContext() )... ); },
          handles );
    }
    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, const EventContext& ctx, Handles& handles ) {
      return std::apply( [&]( const auto&... handle ) { return algo( get( handle, algo, ctx )... ); }, handles );
    }
  };

  // except when it starts with EventContext, then drop it
  template <typename... In>
  struct filter_evtcontext_t<EventContext, In...> {
    using type = std::tuple<In...>;

    static_assert( !std::disjunction_v<std::is_same<EventContext, In>...>,
                   "EventContext can only appear as first argument" );

    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, const EventContext& ctx, Handles& handles ) {
      return std::apply( [&]( const auto&... handle ) { return algo( ctx, get( handle, algo, ctx )... ); }, handles );
    }

    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, Handles& handles ) {
      return apply( algo, Gaudi::Hive::currentContext(), handles );
    }
  };

  template <typename... In>
  using filter_evtcontext = typename filter_evtcontext_t<In...>::type;

  template <typename OutputSpec, typename InputSpec, typename Traits_>
  class DataHandleMixin;

  template <typename Out, typename In, typename Tr>
  void updateHandleLocation( DataHandleMixin<Out, In, Tr>& parent, const std::string& prop,
                             const std::string& newLoc ) {
    auto sc = parent.setProperty( prop, newLoc );
    if ( sc.isFailure() ) throw GaudiException( "Could not set Property", prop + " -> " + newLoc, sc );
  }

  template <typename Out, typename In, typename Tr>
  void updateHandleLocations( DataHandleMixin<Out, In, Tr>& parent, const std::string& prop,
                              const std::vector<std::string>& newLocs ) {
    std::ostringstream ss;
    GaudiUtils::details::ostream_joiner(
        ss << '[', newLocs, ", ", []( std::ostream & os, const auto& i ) -> auto& { return os << "'" << i << "'"; } )
        << ']';
    auto sc = parent.setProperty( prop, ss.str() );
    if ( sc.isFailure() ) throw GaudiException( "Could not set Property", prop + " -> " + ss.str(), sc );
  }

  template <typename... Out, typename... In, typename Traits_>
  class DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of_v<Algorithm, BaseClass_t<Traits_>>, "BaseClass must inherit from Algorithm" );

    template <typename IArgs, typename OArgs, std::size_t... I, std::size_t... J>
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, const IArgs& inputs, std::index_sequence<I...>,
                     const OArgs& outputs, std::index_sequence<J...> )
        : BaseClass_t<Traits_>( std::move( name ), pSvcLocator )
        , m_inputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( inputs ) )... )
        , m_outputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<J>( outputs ) )... ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 ).ignore();
    }

  public:
    constexpr static std::size_t N_in  = sizeof...( In );
    constexpr static std::size_t N_out = sizeof...( Out );

    using KeyValue  = std::pair<std::string, std::string>;
    using KeyValues = std::pair<std::string, std::vector<std::string>>;

    // generic constructor:  N -> M
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, RepeatValues_<KeyValue, N_in> const& inputs,
                     RepeatValues_<KeyValue, N_out> const& outputs )
        : DataHandleMixin( std::move( name ), pSvcLocator, inputs, std::index_sequence_for<In...>{}, outputs,
                           std::index_sequence_for<Out...>{} ) {}

    // special cases: forward to the generic case...
    // 1 -> 1
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& input, const KeyValue& output )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( input ),
                           std::forward_as_tuple( output ) ) {}
    // 1 -> N
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& input,
                     RepeatValues_<KeyValue, N_out> const& outputs )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( input ), outputs ) {}
    // N -> 1
    DataHandleMixin( std::string name, ISvcLocator* locator, RepeatValues_<KeyValue, N_in> const& inputs,
                     const KeyValue& output )
        : DataHandleMixin( std::move( name ), locator, inputs, std::forward_as_tuple( output ) ) {}

    template <std::size_t N = 0>
    decltype( auto ) inputLocation() const {
      return getKey( std::get<N>( m_inputs ) );
    }
    template <typename T>
    decltype( auto ) inputLocation() const {
      return getKey( std::get<details::InputHandle_t<Traits_, std::decay_t<T>>>( m_inputs ) );
    }
    constexpr unsigned int inputLocationSize() const { return N_in; }

    template <std::size_t N = 0>
    decltype( auto ) outputLocation() const {
      return getKey( std::get<N>( m_outputs ) );
    }
    template <typename T>
    decltype( auto ) outputLocation() const {
      return getKey( std::get<details::OutputHandle_t<Traits_, std::decay_t<T>>>( m_outputs ) );
    }
    constexpr unsigned int outputLocationSize() const { return N_out; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::InputHandle_t<Traits_, In>...>   m_inputs;
    std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
  };

  template <typename Traits_>
  class DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of_v<Algorithm, BaseClass_t<Traits_>>, "BaseClass must inherit from Algorithm" );

  public:
    using KeyValue  = std::pair<std::string, std::string>;
    using KeyValues = std::pair<std::string, std::vector<std::string>>;
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, std::tuple<> = {}, std::tuple<> = {} )
        : BaseClass_t<Traits_>( std::move( name ), pSvcLocator ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 ).ignore();
    }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<> m_inputs;
  };

  template <typename... In, typename Traits_>
  class DataHandleMixin<std::tuple<>, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of_v<Algorithm, BaseClass_t<Traits_>>, "BaseClass must inherit from Algorithm" );

    template <typename IArgs, std::size_t... I>
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, const IArgs& inputs, std::index_sequence<I...> )
        : BaseClass_t<Traits_>( std::move( name ), pSvcLocator )
        , m_inputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( inputs ) )... ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 ).ignore();
    }

  public:
    using KeyValue                    = std::pair<std::string, std::string>;
    using KeyValues                   = std::pair<std::string, std::vector<std::string>>;
    constexpr static std::size_t N_in = sizeof...( In );

    // generic constructor:  N -> 0
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, RepeatValues_<KeyValue, N_in> const& inputs )
        : DataHandleMixin( std::move( name ), pSvcLocator, inputs, std::index_sequence_for<In...>{} ) {}

    // special cases: forward to the generic case...
    // 1 -> 0
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& input )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( input ) ) {}

    template <std::size_t N = 0>
    decltype( auto ) inputLocation() const {
      return getKey( std::get<N>( m_inputs ) );
    }
    template <typename T>
    decltype( auto ) inputLocation() const {
      return getKey( std::get<details::InputHandle_t<Traits_, std::decay_t<T>>>( m_inputs ) );
    }
    constexpr unsigned int inputLocationSize() const { return N_in; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::InputHandle_t<Traits_, In>...> m_inputs;
  };

  template <typename Traits_>
  class DataHandleMixin<std::tuple<void>, std::tuple<>, Traits_>
      : public DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
  public:
    using DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;
  };

  template <typename... Out, typename Traits_>
  class DataHandleMixin<std::tuple<Out...>, std::tuple<>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of_v<Algorithm, BaseClass_t<Traits_>>, "BaseClass must inherit from Algorithm" );

    template <typename OArgs, std::size_t... J>
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, const OArgs& outputs, std::index_sequence<J...> )
        : BaseClass_t<Traits_>( std::move( name ), pSvcLocator )
        , m_outputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<J>( outputs ) )... ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 ).ignore();
    }

  public:
    constexpr static std::size_t N_out = sizeof...( Out );
    using KeyValue                     = std::pair<std::string, std::string>;
    using KeyValues                    = std::pair<std::string, std::vector<std::string>>;

    // generic constructor:  0 -> N
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, RepeatValues_<KeyValue, N_out> const& outputs )
        : DataHandleMixin( std::move( name ), pSvcLocator, outputs, std::index_sequence_for<Out...>{} ) {}

    // 0 -> 1
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& output )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( output ) ) {}

    template <std::size_t N = 0>
    decltype( auto ) outputLocation() const {
      return getKey( std::get<N>( m_outputs ) );
    }
    constexpr unsigned int outputLocationSize() const { return N_out; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
  };

  /////////////////
  template <typename Fun, typename Container, typename... Args>
  constexpr void applyPostProcessing( const Fun&, Container&, Args... ) {
    static_assert( sizeof...( Args ) == 0, "Args should not be used!" );
  }

  template <typename Fun, typename Container>
  auto applyPostProcessing( const Fun& fun, Container& c ) -> decltype( fun.postprocess( c ), void() ) {
    fun.postprocess( c );
  }

} // namespace Gaudi::Functional::details

#endif
