#ifndef FUNCTIONAL_DETAILS_H
#define FUNCTIONAL_DETAILS_H

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/detected.h"

// Boost
#include "boost/optional.hpp"

// Range V3
#include <range/v3/view/const.hpp>
#include <range/v3/view/zip.hpp>

namespace Gaudi
{
  namespace Functional
  {
    namespace details
    {

      // CRJ : Stuff for zipping
      namespace zip
      {

        /// Print the parameter
        template <typename OS, typename Arg>
        void printSizes( OS& out, Arg&& arg )
        {
          out << "SizeOf'" << System::typeinfoName( typeid( Arg ) ) << "'=" << std::forward<Arg>( arg ).size();
        }

        /// Print the parameters
        template <typename OS, typename Arg, typename... Args>
        void printSizes( OS& out, Arg&& arg, Args&&... args )
        {
          printSizes( out, arg );
          out << ", ";
          printSizes( out, args... );
        }

        /// Resolve case there is only one container in the range
        template <typename A>
        inline bool check_sizes( const A& ) noexcept
        {
          return true;
        }

        /// Compare sizes of two containers
        template <typename A, typename B>
        inline bool check_sizes( const A& a, const B& b ) noexcept
        {
          return a.size() == b.size();
        }

        /// Compare sizes of 3 or more containers
        template <typename A, typename B, typename... C>
        inline bool check_sizes( const A& a, const B& b, const C&... c ) noexcept
        {
          return ( check_sizes( a, b ) && check_sizes( b, c... ) );
        }

        /// Verify the data container sizes have the same sizes
        template <typename... Args>
        inline decltype( auto ) verifySizes( Args&... args )
        {
          if ( UNLIKELY( !check_sizes( args... ) ) ) {
            std::ostringstream mess;
            mess << "Zipped containers have different sizes : ";
            printSizes( mess, args... );
            throw GaudiException( mess.str(), "Gaudi::Functional::details::zip::verifySizes", StatusCode::FAILURE );
          }
        }

        /// Zips multiple containers together to form a single range
        template <typename... Args>
        inline decltype( auto ) range( Args&&... args )
        {
#ifndef NDEBUG
          verifySizes( args... );
#endif
          return ranges::view::zip( std::forward<Args>( args )... );
        }

        /// Zips multiple containers together to form a single const range
        template <typename... Args>
        inline decltype( auto ) const_range( Args&&... args )
        {
#ifndef NDEBUG
          verifySizes( args... );
#endif
          return ranges::view::const_( ranges::view::zip( std::forward<Args>( args )... ) );
        }
      }

#if __cplusplus < 201703L
      // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
      template <typename T>
      constexpr std::add_const_t<T>& as_const( T& t ) noexcept
      {
        return t;
      }

      template <typename T>
      void as_const( T&& t ) = delete;
#else
      using std::as_const;
#endif
      /////////////////////////////////////////

      template <typename Out1, typename Out2, typename = std::enable_if_t<std::is_constructible<Out1, Out2>::value>>
      Out1* put( DataObjectHandle<Out1>& out_handle, Out2&& out )
      {
        return out_handle.put( std::make_unique<Out1>( std::forward<Out2>( out ) ) );
      }

      template <typename Out1, typename Out2, typename = std::enable_if_t<std::is_constructible<Out1, Out2>::value>>
      void put( AnyDataHandle<Out1>& out_handle, Out2&& out )
      {
        out_handle.put( std::forward<Out2>( out ) );
      }

      // optional put
      template <typename OutHandle, typename Out>
      void put( OutHandle& out_handle, boost::optional<Out>&& out )
      {
        if ( out ) put( out_handle, std::move( *out ) );
      }
      /////////////////////////////////////////
      // adapt to differences between eg. std::vector (which has push_back) and KeyedContainer (which has insert)
      // adapt to getting a T, and a container wanting T* by doing new T{ std::move(out) }
      // adapt to getting a boost::optional<T>

      constexpr struct insert_t {
        // for Container<T*>, return T
        template <typename Container>
        using c_remove_ptr_t = std::remove_pointer_t<typename Container::value_type>;

        template <typename Container, typename Value>
        auto operator()( Container& c, Value&& v ) const -> decltype( c.push_back( v ) )
        {
          return c.push_back( std::forward<Value>( v ) );
        }

        template <typename Container, typename Value>
        auto operator()( Container& c, Value&& v ) const -> decltype( c.insert( v ) )
        {
          return c.insert( std::forward<Value>( v ) );
        }

        // Container<T*> with T&& as argument
        template <typename Container,
                  typename = std::enable_if_t<std::is_pointer<typename Container::value_type>::value>>
        auto operator()( Container& c, c_remove_ptr_t<Container>&& v ) const
        {
          return operator()( c, new c_remove_ptr_t<Container>{std::move( v )} );
        }

        template <typename Container, typename Value>
        void operator()( Container& c, boost::optional<Value>&& v ) const
        {
          if ( v ) operator()( c, std::move( *v ) );
        }
      } insert{};

      /////////////////////////////////////////

      constexpr struct deref_t {
        template <typename In, typename = std::enable_if_t<!std::is_pointer<In>::value>>
        const In& operator()( const In& in ) const
        {
          return in;
        }

        template <typename In>
        const In& operator()( const In* in ) const
        {
          assert( in != nullptr );
          return *in;
        }
      } deref{};

      /////////////////////////////////////////

      namespace details2
      {
        template <typename T>
        struct remove_optional {
          typedef T type;
        };
        template <typename T>
        struct remove_optional<boost::optional<T>> {
          typedef T type;
        };
        // template< typename T > struct remove_optional< std::optional<T> >  {typedef T type;};
      }
      template <typename T>
      using remove_optional_t = typename details2::remove_optional<T>::type;
      template <typename T>
      struct is_optional : std::false_type {
      };
      template <typename T>
      struct is_optional<boost::optional<T>> : std::true_type {
      };
      // C++17: template <typename T> constexpr bool is_optional_v = is_optional<T>::value;

      /////////////////////////////////////////
      // if Container is a pointer, then we're optional items
      namespace details2
      {
        template <typename Container, typename Value>
        void push_back( Container& c, const Value& v, std::true_type )
        {
          c.push_back( v );
        }
        template <typename Container, typename Value>
        void push_back( Container& c, const Value& v, std::false_type )
        {
          c.push_back( &v );
        }

        template <typename In>
        struct get_from_handle {
          template <template <typename> class Handle, typename I,
                    typename = std::enable_if_t<std::is_convertible<I, In>::value>>
          auto operator()( const Handle<I>& h ) -> const In&
          {
            return *h.get();
          }
          template <template <typename> class Handle, typename I,
                    typename = std::enable_if_t<std::is_convertible<I*, In>::value>>
          auto operator()( const Handle<I>& h ) -> const In
          {
            return h.getIfExists();
          } // In is-a pointer
        };

        template <typename T>
        T* deref_if( T* const t, std::false_type )
        {
          return t;
        }
        template <typename T>
        T& deref_if( T* const t, std::true_type )
        {
          return *t;
        }
      }

      template <typename Container>
      class vector_of_const_
      {
        static constexpr bool is_optional = std::is_pointer<Container>::value;
        using val_t                       = std::add_const_t<std::remove_pointer_t<Container>>;
        using ptr_t                       = std::add_pointer_t<val_t>;
        using ref_t                       = std::add_lvalue_reference_t<val_t>;
        using ContainerVector             = std::vector<ptr_t>;
        ContainerVector m_containers;

      public:
        using value_type = std::conditional_t<is_optional, ptr_t, val_t>;
        using size_type  = typename ContainerVector::size_type;
        class iterator
        {
          typename ContainerVector::const_iterator m_i;
          friend class vector_of_const_;
          iterator( typename ContainerVector::const_iterator iter ) : m_i( iter ) {}
          using ret_t = std::conditional_t<is_optional, ptr_t, ref_t>;

        public:
          friend bool operator!=( const iterator& lhs, const iterator& rhs ) { return lhs.m_i != rhs.m_i; }
          ret_t operator*() const { return details2::deref_if( *m_i, std::integral_constant<bool, !is_optional>{} ); }
          iterator& operator++()
          {
            ++m_i;
            return *this;
          }
          iterator& operator--()
          {
            --m_i;
            return *this;
          }
          bool is_null() const { return !*m_i; }
          explicit operator bool() const { return !is_null(); }
        };
        vector_of_const_() = default;
        void reserve( size_type size ) { m_containers.reserve( size ); }
        template <typename T> // , typename = std::is_convertible<T,std::conditional_t<is_optional,ptr_t,val_t>>
        void push_back( T&& container )
        {
          details2::push_back( m_containers, std::forward<T>( container ),
                               std::integral_constant<bool, is_optional>{} );
        } // note: does not copy its argument, so we're not really a container...
        iterator  begin() const { return m_containers.begin(); }
        iterator  end() const { return m_containers.end(); }
        size_type size() const { return m_containers.size(); }
        const Container& operator[]( size_type i ) const { return *m_containers[i]; }
        const Container& at( size_type i ) const
        {
          if ( UNLIKELY( i >= size() ) ) throw std::out_of_range{"vector_of_const_::at"};
          return *m_containers[i];
        }
        bool is_null( size_type i ) const { return !m_containers[i]; }
      };

      /////////////////////////////////////////
      namespace detail2
      { // utilities for detected_or_t{,_} usage
        template <typename Tr>
        using BaseClass_t = typename Tr::BaseClass;
        template <typename Tr, typename T>
        using OutputHandle_t = typename Tr::template OutputHandle<T>;
        template <typename Tr, typename T>
        using InputHandle_t = typename Tr::template InputHandle<T>;
      }

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
      using InputHandle_t = Gaudi::cpp17::detected_or_t<DataObjectReadHandle<T>, detail2::InputHandle_t, Tr, T>;

      /////////

      template <typename Handles>
      Handles make_vector_of_handles( IDataHandleHolder* owner, const std::vector<std::string>& init )
      {
        Handles handles;
        handles.reserve( init.size() );
        std::transform( init.begin(), init.end(), std::back_inserter( handles ),
                        [&]( const std::string& loc ) -> typename Handles::value_type {
                          return {loc, owner};
                        } );
        return handles;
      }

      ///////////////////////

      template <typename OutputSpec, typename InputSpec, typename Traits_>
      class DataHandleMixin;

      template <typename... Out, typename... In, typename Traits_>
      class DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_>
      {
        static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value,
                       "BaseClass must inherit from Algorithm" );

        template <typename IArgs, typename OArgs, std::size_t... I, std::size_t... J>
        DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const IArgs& inputs,
                         std::index_sequence<I...>, const OArgs& outputs, std::index_sequence<J...> )
            : BaseClass_t<Traits_>( name, pSvcLocator )
            , m_inputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( inputs ) )... )
            , m_outputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<J>( outputs ) )... )
        {
          // make sure this algorithm is seen as reentrant by Gaudi
          this->setProperty( "Cardinality", 0 );
        }

      public:
        using KeyValue                     = std::pair<std::string, std::string>;
        constexpr static std::size_t N_in  = sizeof...( In );
        constexpr static std::size_t N_out = sizeof...( Out );

        // generic constructor:  N -> M
        DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N_in>& inputs,
                         const std::array<KeyValue, N_out>& outputs )
            : DataHandleMixin( name, pSvcLocator, inputs, std::index_sequence_for<In...>{}, outputs,
                               std::index_sequence_for<Out...>{} )
        {
        }

        // special cases: forward to the generic case...
        // 1 -> 1
        DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& input, const KeyValue& output )
            : DataHandleMixin( name, locator, std::array<KeyValue, 1>{input}, std::array<KeyValue, 1>{output} )
        {
        }
        // 1 -> N
        DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& input,
                         const std::array<KeyValue, N_out>& outputs )
            : DataHandleMixin( name, locator, std::array<KeyValue, 1>{input}, outputs )
        {
        }
        // N -> 1
        DataHandleMixin( const std::string& name, ISvcLocator* locator, const std::array<KeyValue, N_in>& inputs,
                         const KeyValue& output )
            : DataHandleMixin( name, locator, inputs, std::array<KeyValue, 1>{output} )
        {
        }

        template <std::size_t N = 0>
        const std::string&    inputLocation() const
        {
          return std::get<N>( m_inputs ).objKey();
        }
        constexpr unsigned int inputLocationSize() const { return N_in; }

        template <std::size_t N = 0>
        const std::string&    outputLocation() const
        {
          return std::get<N>( m_outputs ).objKey();
        }
        constexpr unsigned int outputLocationSize() const { return N_out; }

      protected:
        std::tuple<details::InputHandle_t<Traits_, In>...>   m_inputs;
        std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
      };

      template <typename... In, typename Traits_>
      class DataHandleMixin<void, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_>
      {
        static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value,
                       "BaseClass must inherit from Algorithm" );

        template <typename IArgs, std::size_t... I>
        DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const IArgs& inputs,
                         std::index_sequence<I...> )
            : BaseClass_t<Traits_>( name, pSvcLocator )
            , m_inputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( inputs ) )... )
        {
          // make sure this algorithm is seen as reentrant by Gaudi
          this->setProperty( "Cardinality", 0 );
        }

      public:
        using KeyValue                    = std::pair<std::string, std::string>;
        constexpr static std::size_t N_in = sizeof...( In );

        // generic constructor:  N -> 0
        DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N_in>& inputs )
            : DataHandleMixin( name, pSvcLocator, inputs, std::index_sequence_for<In...>{} )
        {
        }

        // special cases: forward to the generic case...
        // 1 -> 0
        DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& input )
            : DataHandleMixin( name, locator, std::array<KeyValue, 1>{input} )
        {
        }

        template <std::size_t N = 0>
        const std::string&    inputLocation() const
        {
          return std::get<N>( m_inputs ).objKey();
        }
        constexpr unsigned int inputLocationSize() const { return N_in; }

      protected:
        std::tuple<details::InputHandle_t<Traits_, In>...> m_inputs;
      };

      template <typename... Out, typename Traits_>
      class DataHandleMixin<std::tuple<Out...>, void, Traits_> : public BaseClass_t<Traits_>
      {
        static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value,
                       "BaseClass must inherit from Algorithm" );

        template <typename OArgs, std::size_t... J>
        DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const OArgs& outputs,
                         std::index_sequence<J...> )
            : BaseClass_t<Traits_>( name, pSvcLocator )
            , m_outputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<J>( outputs ) )... )
        {
          // make sure this algorithm is seen as reentrant by Gaudi
          this->setProperty( "Cardinality", 0 );
        }

      public:
        using KeyValue                     = std::pair<std::string, std::string>;
        constexpr static std::size_t N_out = sizeof...( Out );

        // generic constructor:  0 -> N
        DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N_out>& outputs )
            : DataHandleMixin( name, pSvcLocator, outputs, std::index_sequence_for<Out...>{} )
        {
        }

        // 0 -> 1
        DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& output )
            : DataHandleMixin( name, locator, std::array<KeyValue, 1>{output} )
        {
        }

        template <std::size_t N = 0>
        const std::string&    outputLocation() const
        {
          return std::get<N>( m_outputs ).objKey();
        }
        constexpr unsigned int outputLocationSize() const { return N_out; }

      protected:
        std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
      };

      /////////////////
      template <typename Fun, typename Container, typename... Args>
      constexpr void applyPostProcessing( const Fun&, Container&, Args... )
      {
        static_assert( sizeof...( Args ) == 0, "Args should not be used!" );
      }

      template <typename Fun, typename Container>
      auto applyPostProcessing( const Fun& fun, Container& c ) -> decltype( fun.postprocess( c ), void() )
      {
        fun.postprocess( c );
      }

      /////////////////
    }
  }
}

#endif
