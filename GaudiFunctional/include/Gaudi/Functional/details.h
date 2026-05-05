/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IBinder.h>
#include <GaudiKernel/ThreadLocalContext.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <source_location>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// TODO: migrate downstream users to include `zip.h` directly, instead of transitively,
//       and then drop this include here...
#include "deprecated.h"
#include "zip.h"

namespace Gaudi::Functional::details {

  inline std::vector<DataObjID> to_DataObjID( const std::vector<std::string>& in ) {
    std::vector<DataObjID> out;
    out.reserve( in.size() );
    std::transform( in.begin(), in.end(), std::back_inserter( out ),
                    []( const std::string& i ) { return DataObjID{ i }; } );
    return out;
  }

  /////////////////////////////////////////
  template <typename T>
  concept is_optional = requires( T const& t ) {
    t.has_value();
    t.value();
    typename T::value_type;
  };

  namespace details2 {

    template <typename T>
    struct value_type_of {
      using type = T;
    };

    template <is_optional T>
    struct value_type_of<T> {
      using type = T::value_type;
    };

  } // namespace details2

  template <typename T>
  using remove_optional_t = typename details2::value_type_of<T>::type;

  constexpr struct invoke_optionally_t {
    template <typename F, typename Arg>
      requires( !is_optional<Arg> )
    decltype( auto ) operator()( F&& f, Arg&& arg ) const {
      return std::invoke( std::forward<F>( f ), std::forward<Arg>( arg ) );
    }
    template <typename F, is_optional Arg>
    void operator()( F&& f, Arg&& arg ) const {
      if ( arg ) std::invoke( std::forward<F>( f ), *std::forward<Arg>( arg ) );
    }
  } invoke_optionally{};
  /////////////////////////////////////////
#if 0
  template <typename Value, auto>
  using repeat_t = Value;
  template <typename Value, auto N>
  using RepeatValues_ =
      decltype( []<std::size_t... I>( std::index_sequence<I...> ) -> std::tuple<repeat_t<Value, I>...> {
      }( std::make_index_sequence<N>{} ) );
#else
  template <typename Value, std::size_t... I>
  auto get_values_helper( std::index_sequence<I...> ) {
    return std::make_tuple( ( (void)I, Value{} )... );
  }

  template <typename Value, auto N>
  using RepeatValues_ = decltype( get_values_helper<Value>( std::make_index_sequence<N>() ) );
#endif

  /////////////////////////////////////////
  template <std::derived_from<DataObject> Out1, std::convertible_to<Out1> Out2>
  auto put( const DataObjectHandle<Out1>& out_handle, Out2&& out ) {
    return out_handle.put( std::make_unique<Out1>( std::forward<Out2>( out ) ) );
  }

  template <typename Out1, std::convertible_to<Out1> Out2>
  auto put( const DataObjectHandle<AnyDataWrapper<Out1>>& out_handle, Out2&& out ) {
    return out_handle.put( std::forward<Out2>( out ) );
  }

  // optional put
  template <typename OutHandle, typename OptOut>
    requires( is_optional<OptOut> )
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
    template <typename Container, typename Value>
      requires( std::is_pointer_v<typename Container::value_type> &&
                std::is_convertible_v<Value, c_remove_ptr_t<Container>> )
    auto operator()( Container& c, Value&& v ) const {
      return operator()( c, new c_remove_ptr_t<Container>{ std::forward<Value>( v ) } );
    }

  } insert{};

  /////////////////////////////////////////

  constexpr struct deref_t {
    template <typename In>
      requires( !std::is_pointer_v<In> )
    const In& operator()( const In& in ) const {
      return in;
    }

    template <typename In>
      requires( !std::is_pointer_v<std::decay_t<In>> )
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
    constexpr static bool is_gaudi_range_v = false;

    template <typename T>
    constexpr static bool is_gaudi_range_v<Gaudi::Range_<T>> = true;

    template <typename T>
    constexpr static bool is_gaudi_range_v<Gaudi::NamedRange_<T>> = true;

    template <typename T>
    constexpr static bool is_gaudi_range_v<std::optional<Gaudi::NamedRange_<T>>> = true;

    template <typename In>
    struct get_from_handle {
      template <template <typename> class Handle, std::convertible_to<In> I>
      auto operator()( const Handle<I>& h ) -> const In& {
        return *h.get();
      }
      template <template <typename> class Handle, typename I>
      auto operator()( const Handle<Gaudi::Range_<I>>& h ) -> In {
        return h.get();
      }
      template <template <typename> class Handle, typename I>
      auto operator()( const Handle<Gaudi::NamedRange_<I>>& h ) -> In {
        return h.get();
      }
      template <template <typename> class Handle, typename I>
      auto operator()( const Handle<std::optional<Gaudi::NamedRange_<I>>>& h ) -> In {
        return h.get();
      }
      template <template <typename> class Handle, typename I>
        requires( std::is_convertible_v<I*, In> )
      auto operator()( const Handle<I>& h ) -> In {
        return h.getIfExists();
      } // In is-a pointer
    };

    template <typename Iterator>
    class indirect_iterator {
      using traits = std::iterator_traits<Iterator>;
      Iterator m_iter;

    public:
      using iterator_category [[maybe_unused]] = typename traits::iterator_category;
      using difference_type [[maybe_unused]]   = typename traits::difference_type;
      using reference                          = decltype( **m_iter );
      using value_type [[maybe_unused]]        = std::remove_reference_t<reference>;
      using pointer [[maybe_unused]]           = std::add_pointer_t<value_type>;

      indirect_iterator() = default;
      constexpr explicit indirect_iterator( Iterator i ) : m_iter( std::move( i ) ) {}

      constexpr reference operator*() const { return **m_iter; }

      constexpr bool operator==( const indirect_iterator& rhs ) const { return m_iter == rhs.m_iter; }

      constexpr indirect_iterator& operator++() {
        ++m_iter;
        return *this;
      }
      constexpr indirect_iterator operator++( int ) {
        auto i = *this;
        ++*this;
        return i;
      }

      // bidirectional iterator operations (if possible)
      constexpr indirect_iterator& operator--()
        requires std::bidirectional_iterator<Iterator>
      {
        --m_iter;
        return *this;
      }
      constexpr indirect_iterator operator--( int )
        requires std::bidirectional_iterator<Iterator>
      {
        auto i = *this;
        --*this;
        return i;
      }

      // random access iterator operations (if possible)
      constexpr indirect_iterator& operator+=( difference_type n )
        requires std::random_access_iterator<Iterator>
      {
        m_iter += n;
        return *this;
      }
      constexpr indirect_iterator operator+( difference_type n ) const
        requires std::random_access_iterator<Iterator>
      {
        auto i = *this;
        return i += n;
      }
      constexpr indirect_iterator& operator-=( difference_type n )
        requires std::random_access_iterator<Iterator>
      {
        m_iter -= n;
        return *this;
      }
      constexpr indirect_iterator operator-( difference_type n ) const
        requires std::random_access_iterator<Iterator>
      {
        auto i = *this;
        return i -= n;
      }
      constexpr difference_type operator-( const indirect_iterator& other ) const
        requires std::random_access_iterator<Iterator>
      {
        return m_iter - other.m_iter;
      }
      constexpr reference operator[]( difference_type n ) const
        requires std::random_access_iterator<Iterator>
      {
        return **( m_iter + n );
      }
    };

  } // namespace details2

  template <typename Container>
  class vector_of_const_ {
    static constexpr bool is_pointer = std::is_pointer_v<Container>;
    static constexpr bool is_range   = details2::is_gaudi_range_v<Container>;
    template <typename C>
    using borrowed_value_t = std::add_const_t<std::remove_pointer_t<C>>;
    // TODO: refuse pointer to a range... range must always be by value
    using val_t           = borrowed_value_t<Container>;
    using ptr_t           = std::add_pointer_t<val_t>;
    using ContainerVector = std::vector<std::conditional_t<is_range, std::remove_const_t<val_t>, ptr_t>>;
    ContainerVector m_containers;

    constexpr static decltype( auto ) wrap( ContainerVector::const_reference t ) {
      if constexpr ( is_pointer || is_range ) {
        return t;
      } else {
        return *t;
      }
    }
    constexpr static auto wrap( ContainerVector::const_iterator i ) {
      if constexpr ( is_pointer || is_range ) {
        return i;
      } else {
        return details2::indirect_iterator{ i };
      }
    }

  public:
    using value_type = std::conditional_t<is_pointer, ptr_t, val_t>;
    using size_type  = typename ContainerVector::size_type;

    vector_of_const_() = default;
    void reserve( size_type size ) { m_containers.reserve( size ); }
    template <typename T>
      requires( is_pointer || is_range )
    void push_back( T&& container ) {
      m_containers.push_back( container );
    }
    template <typename C = Container>
      requires( !std::is_pointer_v<C> && !details2::is_gaudi_range_v<C> )
    void push_back( borrowed_value_t<C>& container ) {
      // note: does not copy its argument, so we're not really a container...
      m_containers.push_back( &container );
    }
    template <typename C = Container>
      requires( !std::is_pointer_v<C> && !details2::is_gaudi_range_v<C> )
    void push_back( borrowed_value_t<C>&& ) = delete;

    auto             begin() const { return wrap( m_containers.begin() ); }
    auto             end() const { return wrap( m_containers.end() ); }
    decltype( auto ) front() const { return wrap( m_containers.front() ); }
    decltype( auto ) back() const { return wrap( m_containers.back() ); }
    decltype( auto ) operator[]( size_type i ) const { return wrap( m_containers[i] ); }
    decltype( auto ) at( size_type i ) const { return wrap( m_containers.at( i ) ); }
    size_type        size() const { return m_containers.size(); }
  };

  template <template <typename> class Handle, typename I>
  class HandleVector {
    struct Payload {
      std::vector<Handle<I>>                  handles;
      Gaudi::Property<std::vector<DataObjID>> property;

      template <typename Algorithm>
      Payload( Algorithm* parent, std::pair<std::string, std::vector<std::string>> const& keys )
          : property{ parent, keys.first, details::to_DataObjID( keys.second ),
                      [ptr = &handles, parent]( auto& self_ ) {
                        auto& self = dynamic_cast<Gaudi::Property<std::vector<DataObjID>>&>( self_ );
                        ptr->clear();
                        ptr->reserve( self.value().size() );
                        std::ranges::transform( self.value(), std::back_inserter( *ptr ),
                                                [&]( const auto& location ) -> Handle<I> {
                                                  return { location, parent };
                                                } );
                      },
                      Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } } {}

      Payload( Payload&& )                 = delete;
      Payload& operator=( Payload&& )      = delete;
      Payload( Payload const& )            = delete;
      Payload& operator=( Payload const& ) = delete;
    };
    std::unique_ptr<Payload> m_payload; // need a stable rendez-vous for the callback & property to work

  public:
    template <typename Algorithm>
    HandleVector( Algorithm* parent, std::pair<std::string, std::vector<std::string>> const& keys )
        : m_payload{ std::make_unique<Payload>( parent, keys ) } {}

    // allow construction by DataHandleMixin
    template <typename A, typename K>
    HandleVector( std::tuple<A, K>&& tup ) : HandleVector{ std::get<0>( tup ), std::get<1>( tup ) } {}

    vector_of_const_<I> get( EventContext const& ) const {
      vector_of_const_<I> ins;
      ins.reserve( m_payload->handles.size() );
      std::ranges::transform( m_payload->handles, std::back_inserter( ins ), details2::get_from_handle<I>{} );
      return ins;
    }
    template <typename Out>
    void put( Out&& out ) const {
      auto const n = size();
      if ( out.size() != n ) {
        throw GaudiException( "Error during transform in " +
                                  std::string{ std::source_location::current().function_name() } + ": expected " +
                                  std::to_string( n ) + " containers, got " + std::to_string( out.size() ) + " instead",
                              "Gaudi::Functional::details::HandleVector::put", StatusCode::FAILURE );
      }
      for ( std::size_t i = 0; i != n; ++i ) details::put( handles()[i], std::move( out[i] ) );
    }

    std::vector<Handle<I>> const& handles() const { return m_payload->handles; }
    std::vector<DataObjID> const& locations() const { return m_payload->property.value(); }
    DataObjID const&              at( size_t i ) const { return m_payload->property.value().at( i ); }
    auto                          size() const { return m_payload->handles.size(); }
  };

  template <template <typename> class Handle, typename... I, typename Algorithm, typename InKeys>
  auto make_HandleVectorTuple( Algorithm* parent, InKeys const& keys ) {
    return std::apply(
        [parent]( auto const&... key ) {
          static_assert( sizeof...( key ) == sizeof...( I ) );
          return std::tuple{ HandleVector<Handle, I>{ parent, key }... };
        },
        keys );
  }

  template <template <typename> class Handle, typename... I>
  decltype( auto ) getLocations( std::tuple<HandleVector<Handle, I>...> const& vectors, unsigned int i ) {
    return std::apply(
        [i]( auto const&... elems ) -> decltype( auto ) { return *std::array{ &elems.locations()... }.at( i ); },
        vectors );
  }

  template <typename F>
  StatusCode execute( CommonMessagingBase const& alg, F&& f ) {
    try {
      return std::forward<F>( f )();
    } catch ( GaudiException& e ) {
      if ( e.code().isFailure() ) alg.error() << e.tag() << " : " << e.message() << endmsg;
      return e.code();
    }
  }

  /////////////////////////////////////////
  namespace detail2 { // utilities for detected_or_t{,_} usage

    // keep only for backwards compatibility... for now.
    template <typename Tr>
    using BaseClass_t = typename Tr::BaseClass;

    template <typename Tr, typename Default>
    struct BaseClass {
      using type = Default;
    };
    template <typename Tr, typename Default>
      requires requires { typename Tr::BaseClass; }
    struct BaseClass<Tr, Default> {
      using type = Tr::BaseClass;
    };

    template <typename T, typename Tr, template <typename...> typename Default>
    struct OutputHandle {
      using type = Default<T>;
    };
    template <typename T, typename Tr, template <typename...> typename Default>
      requires requires { typename Tr::template OutputHandle<T>; }
    struct OutputHandle<T, Tr, Default> {
      using type = Tr::template OutputHandle<T>;
    };

    template <typename T, typename Tr, template <typename...> typename Default>
    struct InputHandle {
      using type = Default<T>;
    };
    template <typename T, typename Tr, template <typename...> typename Default>
      requires requires { typename Tr::template InputHandle<T>; }
    struct InputHandle<T, Tr, Default> {
      using type = Tr::template InputHandle<T>;
    };

    template <typename T>
    using DefaultInputHandle =
        std::conditional_t<std::derived_from<std::decay_t<T>, IAlgTool>,
                           ToolHandle<Gaudi::Interface::Bind::IBinder<std::decay_t<T>>>, DataObjectReadHandle<T>>;
  } // namespace detail2

  // check whether Tr::BaseClass is a valid type,
  // if so, define BaseClass_t<Tr> as being Tr::BaseClass
  // else   define                 as being Gaudi::Algorithm
  template <typename Tr, typename Default = Gaudi::Algorithm>
  using BaseClass_t = detail2::BaseClass<Tr, Default>::type;

  // check whether Traits::{Input,Output}Handle<T> is a valid type,
  // if so, define {Input,Output}Handle_t<Traits,T> as being Traits::{Input,Output}Handle<T>
  // else   define                                  as being DataObject{Read,,Write}Handle<T>
  template <typename Tr, typename T>
  using OutputHandle_t = typename detail2::OutputHandle<T, Tr, DataObjectWriteHandle>::type;

  template <typename Tr, typename T>
  using InputHandle_t = typename detail2::InputHandle<T, Tr, detail2::DefaultInputHandle>::type;

  template <typename Traits>
  inline constexpr bool isLegacy =
      std::is_base_of_v<Gaudi::details::LegacyAlgorithmAdapter, details::BaseClass_t<Traits>>;

  /////////

  template <typename Handle, typename Algo>
  auto get( const Handle& handle, const Algo&,
            const EventContext& ) -> decltype( details::deref( handle.get() ) ) // make it SFINAE friendly...
  {
    return details::deref( handle.get() );
  }

  template <typename IFace, typename Algo>
  auto get( const ToolHandle<Gaudi::Interface::Bind::IBinder<IFace>>& handle, const Algo&, const EventContext& ctx ) {
    return handle.bind( ctx );
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
  };

  template <typename... In>
  using filter_evtcontext = typename filter_evtcontext_t<In...>::type;

  template <typename OutputSpec, typename InputSpec, typename Traits_>
  class DataHandleMixin;

  template <typename... Out, typename... In, typename Traits_>
    requires std::derived_from<BaseClass_t<Traits_>, Algorithm>
  class DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_> {

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
      requires( N_in != 0 && N_out != 0 )
        : DataHandleMixin( std::move( name ), pSvcLocator, inputs, std::index_sequence_for<In...>{}, outputs,
                           std::index_sequence_for<Out...>{} ) {}

    // special cases: forward to the generic case...
    // 0 -> 0
    DataHandleMixin( std::string name, ISvcLocator* locator, std::tuple<> = {}, std::tuple<> = {} )
      requires( N_in == 0 && N_out == 0 )
        : DataHandleMixin( std::move( name ), locator, std::tuple<>{}, std::index_sequence<>{}, std::tuple<>{},
                           std::index_sequence<>{} ) {}
    // 1 -> 0
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& input )
      requires( N_in == 1 && N_out == 0 )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( input ), std::index_sequence<0>{},
                           std::tuple<>{}, std::index_sequence<>{} ) {}
    // 0 -> 1
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& output )
      requires( N_in == 0 && N_out == 1 )
        : DataHandleMixin( std::move( name ), locator, std::tuple<>{}, std::index_sequence<>{},
                           std::forward_as_tuple( output ), std::index_sequence<0>{} ) {}
    // 1 -> 1
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& input, const KeyValue& output )
      requires( N_in == 1 && N_out == 1 )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( input ),
                           std::index_sequence<size_t{ 0 }>{}, std::forward_as_tuple( output ),
                           std::index_sequence<size_t{ 0 }>{} ) {}
    // 1 -> N
    DataHandleMixin( std::string name, ISvcLocator* locator, const KeyValue& input,
                     RepeatValues_<KeyValue, N_out> const& outputs )
      requires( N_in == 1 && N_out != 0 )
        : DataHandleMixin( std::move( name ), locator, std::forward_as_tuple( input ),
                           std::index_sequence<size_t{ 0 }>{}, outputs, std::index_sequence_for<Out...>{} ) {}
    // N -> 1
    DataHandleMixin( std::string name, ISvcLocator* locator, RepeatValues_<KeyValue, N_in> const& inputs,
                     const KeyValue& output )
      requires( N_in != 0 && N_out == 1 )
        : DataHandleMixin( std::move( name ), locator, inputs, std::index_sequence_for<In...>{},
                           std::forward_as_tuple( output ), std::index_sequence<size_t{ 0 }>{} ) {}
    // N -> 0
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, RepeatValues_<KeyValue, N_in> const& inputs )
      requires( N_in != 0 && N_out == 0 )
        : DataHandleMixin( std::move( name ), pSvcLocator, inputs, std::index_sequence_for<In...>{}, std::tuple<>{},
                           std::index_sequence<>{} ) {}
    // 0 -> N
    DataHandleMixin( std::string name, ISvcLocator* pSvcLocator, RepeatValues_<KeyValue, N_out> const& outputs )
      requires( N_in == 0 && N_out != 0 )
        : DataHandleMixin( std::move( name ), pSvcLocator, std::tuple<>{}, std::index_sequence<>{}, outputs,
                           std::index_sequence_for<Out...>{} ) {}

    template <std::size_t N = 0>
    decltype( auto ) inputLocation() const
      requires( N_in > 0 )
    {
      return getKey( std::get<N>( m_inputs ) );
    }
    template <typename T>
    decltype( auto ) inputLocation() const
      requires( N_in > 0 )
    {
      return getKey( std::get<details::InputHandle_t<Traits_, std::decay_t<T>>>( m_inputs ) );
    }
    constexpr unsigned int inputLocationSize() const { return N_in; }

    template <std::size_t N = 0>
    decltype( auto ) outputLocation() const
      requires( N_out > 0 )
    {
      return getKey( std::get<N>( m_outputs ) );
    }
    template <typename T>
    decltype( auto ) outputLocation() const
      requires( N_out > 0 )
    {
      return getKey( std::get<details::OutputHandle_t<Traits_, std::decay_t<T>>>( m_outputs ) );
    }
    constexpr unsigned int outputLocationSize() const { return N_out; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::InputHandle_t<Traits_, In>...>   m_inputs;
    std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
  };

  template <typename Traits_>
  class DataHandleMixin<std::tuple<void>, std::tuple<>, Traits_>
      : public DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
  public:
    using DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;
  };

} // namespace Gaudi::Functional::details
