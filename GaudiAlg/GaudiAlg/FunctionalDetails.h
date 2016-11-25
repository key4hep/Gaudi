#ifndef FUNCTIONAL_DETAILS_H
#define FUNCTIONAL_DETAILS_H

#include <type_traits>
#include <stdexcept>
#include <cassert>

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiAlg/GaudiAlgorithm.h"

// Boost
#include "boost/optional.hpp"

// Range V3
#include <range/v3/view/zip.hpp>
#include <range/v3/view/const.hpp>

namespace Gaudi { namespace Functional { namespace details {

    // CRJ : Stuff for zipping
    namespace zip
    {
    
      /// Resolve case there is only one container in the range
      template < typename A >
      inline bool check_sizes( const A& ) noexcept { return true; }

      /// Compare sizes of two containers
      template < typename A, typename B >
      inline bool check_sizes( const A& a, const B& b ) noexcept
      {
        return a.size() == b.size();
      }
      
      /// Compare sizes of 3 or more containers
      template < typename A, typename B, typename... C >
      inline bool check_sizes( const A& a, const B& b, const C& ... c ) noexcept
      {
        return ( check_sizes(a,b) && check_sizes(b,c...) );
      }

      /// Verify the data container sizes have the same sizes
      template< typename... Args >
      inline decltype(auto) verifySizes( Args&... args )
      {
        if ( UNLIKELY( !check_sizes( args... ) ) ) 
        { throw GaudiException( "Zipped containers have different sizes.", 
                                "Gaudi::Functional::details::zip::verifySizes",
                                StatusCode::FAILURE ); }
      }
 
      /// Zips multiple containers together to form a single range
      template< typename... Args >
      inline decltype(auto) range( Args&& ... args )
      {
        //assert( check_sizes( args... ) );
        verifySizes( args... );
        return ranges::view::zip( std::forward<Args>(args)... );
      }
      
      /// Zips multiple containers together to form a single const range
      template< typename... Args >
      inline decltype(auto) const_range( Args&& ... args )
      {
        //assert( check_sizes( args... ) );
        verifySizes( args... );
        return ranges::view::const_( ranges::view::zip( std::forward<Args>(args)... ) );
      }

    }

    // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

    /////////////////////////////////////////

    template <typename Out1, typename Out2,
              typename = typename std::enable_if<std::is_constructible<Out1,Out2>::value>::type>
    Out1* put( DataObjectHandle<Out1>& out_handle, Out2&& out ) {
        return out_handle.put( new Out1( std::forward<Out2>(out) ) );
    }

    template <typename Out1, typename Out2,
              typename = typename std::enable_if<std::is_constructible<Out1,Out2>::value>::type>
    void put( AnyDataHandle<Out1>& out_handle, Out2&& out ) {
        out_handle.put( std::forward<Out2>(out) );
    }

    // optional put
    template <typename OutHandle, typename Out>
    void put( OutHandle& out_handle, boost::optional<Out>&& out) {
       if (out) put(out_handle,std::move(*out));
    }
    /////////////////////////////////////////
    // adapt to differences between eg. std::vector (which has push_back) and KeyedContainer (which has insert)
    // adapt to getting a T, and a container wanting T* by doing new T{ std::move(out) }
    // adapt to getting a boost::optional<T>

    constexpr struct insert_t {
        // for Container<T*>, return T
        template <typename Container>
        using c_remove_ptr_t = typename std::remove_pointer<typename Container::value_type>::type;

        template <typename Container, typename Value>
        auto operator()(Container& c, Value&& v) const -> decltype( c.push_back(v) ) { return c.push_back( std::forward<Value>(v) ); }

        template <typename Container, typename Value>
        auto operator()(Container& c, Value&& v) const -> decltype( c.insert(v) ) { return c.insert( std::forward<Value>(v) ); }

        // Container<T*> with T&& as argument
        template <typename Container, typename = typename std::enable_if< std::is_pointer<typename Container::value_type>::value >::type >
        auto operator()(Container& c, c_remove_ptr_t<Container>&& v) const
        { return operator()( c, new c_remove_ptr_t<Container>{ std::move(v) } ); }

        template <typename Container, typename Value>
        void operator()(Container& c, boost::optional<Value>&& v) const { if (v) operator()(c,std::move(*v)); }
    } insert {};

    /////////////////////////////////////////

    constexpr struct deref_t {
        template <typename In, typename = typename std::enable_if< !std::is_pointer<In>::value>::type>
        In& operator()( In& in ) const { return in; }

        template <typename In>
        In& operator()( In* in ) const { assert(in!=nullptr); return *in; }
    } deref {};

    template <template <typename> class Handle, typename T>
    const T& get_from_handle(const Handle<T>& handle) {
      return *handle.get();
    }

    template <template <typename> class Handle, typename T>
      const Gaudi::Range_<T> get_from_handle(const Handle<Gaudi::Range_<T>>& handle) {
      return handle.get();
    }

    template <template <typename> class Handle, typename T>
      const Gaudi::NamedRange_<T> get_from_handle(const Handle<Gaudi::NamedRange_<T>>& handle) {
      return handle.get();
    }

    /////////////////////////////////////////

    namespace details2 {
        template< typename T > struct remove_optional      {typedef T type;};
        template< typename T > struct remove_optional< boost::optional<T> >  {typedef T type;};
        // template< typename T > struct remove_optional< std::optional<T> >  {typedef T type;};

    }
    template <typename T> using remove_optional_t = typename details2::remove_optional<T>::type;
    template< typename T> struct is_optional : std::false_type {};
    template< typename T> struct is_optional< boost::optional<T> > : std::true_type {};
    // C++17: template <typename T> constexpr bool is_optional_v = is_optional<T>::value;

    /////////////////////////////////////////
    // if Container is a pointer, then we're optional items
    namespace details2 {
         template <typename Container, typename Value>
         void push_back(Container& c, const Value& v,  std::true_type) { c.push_back(v); }
         template <typename Container, typename Value>
         void push_back(Container& c, const Value& v, std::false_type) { c.push_back(&v); }

         template <typename In>
         struct get_from_handle {
           template <template <typename> class Handle, typename I, typename = typename std::enable_if< std::is_convertible<Gaudi::Range_<I>,In>::value >::type >
           auto operator()( const Handle<Gaudi::Range_<I>>& h ) -> const In { return h.get(); }
           template <template <typename> class Handle, typename I, typename = typename std::enable_if< std::is_convertible<I,In>::value >::type >
           auto operator()( const Handle<I>& h ) -> const In& { return *h.get(); }
           template <template <typename> class Handle, typename I, typename = typename std::enable_if<std::is_convertible<I*,In>::value>::type >
           auto operator()( const Handle<I>& h ) -> const In { return h.getIfExists(); } // In is-a pointer
         };

         template <typename T> T* deref_if(T* const t,std::false_type) { return t; }
         template <typename T> T& deref_if(T* const t,std::true_type)  { return *t; }
    }

    template <typename Container>
    class vector_of_const_ {
        static constexpr bool is_optional = std::is_pointer<Container>::value;
        using val_t = std::add_const_t<std::remove_pointer_t<Container>>;
        using ptr_t = std::add_pointer_t<val_t>;
        using ref_t = std::add_lvalue_reference_t<val_t>;
        using ContainerVector = std::vector<ptr_t>;
        ContainerVector m_containers;
    public:
        using value_type = std::conditional_t<is_optional,ptr_t,val_t>;
        using size_type  = typename ContainerVector::size_type;
        class iterator {
             typename ContainerVector::const_iterator m_i;
             friend class vector_of_const_;
             iterator(typename ContainerVector::const_iterator iter) : m_i(iter) {}
             using ret_t = std::conditional_t<is_optional,ptr_t,ref_t>;
         public:
             friend bool operator!=(const iterator& lhs, const iterator& rhs) { return lhs.m_i != rhs.m_i; }
             ret_t operator*() const { return details2::deref_if(*m_i, std::integral_constant<bool,!is_optional>{}); }
             iterator& operator++() { ++m_i; return *this; }
             iterator& operator--() { --m_i; return *this; }
             bool is_null() const { return !*m_i; }
             explicit operator bool() const { return !is_null(); }
        };
        vector_of_const_() = default;
        void reserve(size_type size) { m_containers.reserve(size); }
        template <typename T> // , typename = std::is_convertible<T,std::conditional_t<is_optional,ptr_t,val_t>>
        void push_back(T&& container) { details2::push_back(m_containers,std::forward<T>(container), std::integral_constant<bool,is_optional>{});} // note: does not copy its argument, so we're not really a container...
        iterator begin() const { return m_containers.begin(); }
        iterator end() const { return m_containers.end(); }
        size_type size() const { return m_containers.size(); }
        const Container& operator[](size_type i) const { return *m_containers[i]; }
        const Container& at(size_type i) const { if (i>=size()) throw std::out_of_range{"vector_of_const_::at"} ; return *m_containers[i]; }
        bool is_null(size_type i) const { return !m_containers[i]; }
    };

    /////////////////////////////////////////

    // detect whether a traits class defines the requested type,
    //   if so, use it,
    //   otherwise use the default
    //
    // based on http://en.cppreference.com/w/cpp/experimental/is_detected
    // and the libstdc++ source, specificially libstdc++-v3/include/std/type_traits

    namespace detail2 {
#ifdef HAVE_CPP17
        template<typename...> using void_t = void;
#else
        template <typename...> struct void_t_ { using type = void; };
        template <typename... T> using void_t = typename void_t_<T...>::type;
#endif

        /// Implementation of the detection idiom (negative case).
        template<typename Default, typename AlwaysVoid,
                 template<typename...> class Op, typename... Args>
        struct detector {
            using type = Default;
        };

        /// Implementation of the detection idiom (positive case).
        template<typename Default,
                 template<typename...> class Op, typename... Args>
        struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
            using type = Op<Args...>;
        };
    }

    // Op<Args...> if that is a valid type, otherwise Default.
    template<typename Default,
             template<typename...> class Op, typename... Args>
    using detected_or_t = typename detail2::detector<Default, void, Op, Args...>::type;

     // Op<Args...> if that is a valid type, otherwise Default<Args...>.
    template<template<typename...> class Default,
             template<typename...> class Op,
      typename Tr, typename T>
      using detected_or_t_ = detected_or_t<Default<Tr, T>, Op, Tr, T>;

    ///////////////
    namespace detail2 { // utilities for detected_or_t{,_} usage

        template <typename Tr> using BaseClass_  = typename Tr::BaseClass;
        template <typename Tr, typename T> using defaultHandle_ = typename std::conditional< std::is_base_of<DataObject,T>::value,
                                                                                             DataObjectHandle<T>,
                                                                                             AnyDataHandle<T>>::type;
        template <typename Tr, typename T> using OutputHandle_  = typename Tr::template OutputHandle<T>;
        template <typename Tr, typename T> using InputHandle_   = typename Tr::template InputHandle<T>;
    }

    // check whether Traits::BaseClass is a valid type,
    // if so, define BaseClass_t<Traits> as being Traits::BaseClass
    // else   define                     as being GaudiAlgorithm
    template <typename Tr> using BaseClass_t = detected_or_t< GaudiAlgorithm, detail2::BaseClass_, Tr >;

    // check whether Traits::{Input,Output}Handle<T> is a valid type,
    // if so, define {Input,Output}Handle_t<Traits,T> as being Traits::{Input,Output}Handle<T>
    // else   define                                  as being DataObjectHandle<T> if T derives from DataObject, else AnyDataHandle<T>
    template <typename Tr, typename T> using OutputHandle_t = detected_or_t_< detail2::defaultHandle_, detail2::OutputHandle_, Tr, T>;
    template <typename Tr, typename T> using InputHandle_t  = detected_or_t_< detail2::defaultHandle_, detail2::InputHandle_,  Tr, T>;

    /////////

    namespace details2 {
        template <std::size_t N, typename Tuple >
        using element_t = typename std::tuple_element<N, Tuple>::type;

        template <typename Tuple, typename KeyValues, std::size_t... I>
        Tuple make_tuple_of_handles_helper( IDataHandleHolder* o, const KeyValues& initvalue, Gaudi::DataHandle::Mode m, std::index_sequence<I...> ) {
            return std::make_tuple( element_t<I,Tuple>{std::get<I>(initvalue).second, m, o} ... );
        }
        template <typename KeyValues, typename Properties,  std::size_t... I>
        void declare_tuple_of_properties_helper(GaudiAlgorithm* owner, const KeyValues& inputs, Properties& props,  std::index_sequence<I...>) {
            // note: be very careful here! Only GaudiAlgorithm has a declareProperty that works with a DataObjectHandleBase.
            // However, Algorithm does have a template that also matches (unless it is constrained explicitly against matching)
            // so if 'owner' is of type Algorithm instead of GaudiAlgortihm, this ends up calling the wrong declareProperty...
          (void) std::initializer_list<int>{
                ( owner->declareProperty( std::get<I>(inputs).first,
                                          std::get<I>(props)         ),0)...
            };
        }
    }

    template <typename Tuple, typename KeyValues >
    Tuple make_tuple_of_handles( IDataHandleHolder* owner, const KeyValues& initvalue, Gaudi::DataHandle::Mode mode ) {
        return details2::make_tuple_of_handles_helper<Tuple>( owner, initvalue, mode, std::make_index_sequence<std::tuple_size<Tuple>::value>{} );
    }

    template <typename KeyValues, typename Properties>
    void declare_tuple_of_properties(GaudiAlgorithm* owner, const KeyValues& inputs, Properties& props) {
        static_assert( std::tuple_size<KeyValues>::value == std::tuple_size<Properties>::value, "Inconsistent lengths" );
        constexpr auto N = std::tuple_size<KeyValues>::value;
        details2::declare_tuple_of_properties_helper( owner, inputs, props, std::make_index_sequence<N>{} );
    }

    template <typename Handles>
    Handles make_vector_of_handles(IDataHandleHolder* owner, const std::vector<std::string>& init, Gaudi::DataHandle::Mode mode) {
         Handles handles; handles.reserve(init.size());
         std::transform( init.begin(), init.end(), std::back_inserter(handles),
                         [&](const std::string& loc) -> typename Handles::value_type
                         { return {loc,mode, owner}; });
         return handles;
    }

    ///////////////////////


   template <typename OutputSpec, typename InputSpec, typename Traits_> class DataHandleMixin;

   template <typename... Out, typename... In, typename Traits_>
   class DataHandleMixin<std::tuple<Out...>, std::tuple<In...>,Traits_> : public BaseClass_t<Traits_> {
       static_assert( std::is_base_of<GaudiAlgorithm, BaseClass_t<Traits_>>::value,
                      "BaseClass must inherit from GaudiAlgorithm");
   public:
       using KeyValue = std::pair<std::string, std::string>;
       constexpr static std::size_t N_in = sizeof...(In);
       constexpr static std::size_t N_out = sizeof...(Out);

       // generic constructor:  N -> M
       DataHandleMixin(const std::string& name, ISvcLocator* pSvcLocator,
                     const std::array<KeyValue,N_in>& inputs,
                     const std::array<KeyValue,N_out>& outputs)
       : BaseClass_t<Traits_>( name , pSvcLocator ),
         m_inputs( make_tuple_of_handles<decltype(m_inputs)>( this, inputs, Gaudi::DataHandle::Reader ) ),
         m_outputs( make_tuple_of_handles<decltype(m_outputs)>( this, outputs, Gaudi::DataHandle::Writer ) )
       {
         declare_tuple_of_properties( this, inputs, m_inputs );
         declare_tuple_of_properties( this, outputs, m_outputs );
         // make sure this algorithm is seen as reentrant by Gaudi
         BaseClass_t<Traits_>::setProperty("Cardinality", 0);
       }

       // special cases: forward to the generic case...
       // 1 -> 1
       DataHandleMixin(const std::string& name, ISvcLocator* locator,
                     const KeyValue& input,
                     const KeyValue& output)
       : DataHandleMixin( name, locator, std::array<KeyValue,1>{ input }, std::array<KeyValue,1>{ output } )
       { }
       // 1 -> N
       DataHandleMixin(const std::string& name, ISvcLocator* locator,
                     const KeyValue& input,
                     const std::array<KeyValue,N_out>& outputs)
       : DataHandleMixin( name, locator, std::array<KeyValue,1>{ input }, outputs )
       { }
       // N -> 1
       DataHandleMixin(const std::string& name, ISvcLocator* locator,
                     const std::array<KeyValue,N_in>& inputs,
                     const KeyValue& output )
       : DataHandleMixin( name, locator, inputs, std::array<KeyValue,1>{ output } )
       { }

       template <std::size_t N=0>
       const std::string& inputLocation() const { return std::get<N>(m_inputs).objKey(); }
       unsigned int inputLocationSize() const { return std::tuple_size<decltype(m_inputs)>::value; }

       template <std::size_t N=0>
       const std::string& outputLocation() const { return std::get<N>(m_outputs).objKey(); }
       unsigned int outputLocationSize() const { return std::tuple_size<decltype(m_outputs)>::value; }

   protected:
       std::tuple<details::InputHandle_t<Traits_,In>...>  m_inputs;
       std::tuple<details::OutputHandle_t<Traits_,Out>...> m_outputs;
   };

   template <typename... In, typename Traits_>
   class DataHandleMixin<void, std::tuple<In...>,Traits_> : public BaseClass_t<Traits_> {
       static_assert( std::is_base_of<GaudiAlgorithm, BaseClass_t<Traits_>>::value,
                      "BaseClass must inherit from GaudiAlgorithm");
   public:
       using KeyValue = std::pair<std::string, std::string>;
       constexpr static std::size_t N_in = sizeof...(In);

       // generic constructor:  N -> 0
       DataHandleMixin(const std::string& name, ISvcLocator* pSvcLocator,
                     const std::array<KeyValue,N_in>& inputs )
       : BaseClass_t<Traits_>( name , pSvcLocator ),
         m_inputs( make_tuple_of_handles<decltype(m_inputs)>( this, inputs, Gaudi::DataHandle::Reader ) )
       {
         declare_tuple_of_properties( this, inputs, m_inputs );
         // make sure this algorithm is seen as reentrant by Gaudi
         BaseClass_t<Traits_>::setProperty("Cardinality", 0);
       }

       // special cases: forward to the generic case...
       // 1 -> 0
       DataHandleMixin(const std::string& name, ISvcLocator* locator,
                     const KeyValue& input )
       : DataHandleMixin( name, locator, std::array<KeyValue,1>{ input } )
       { }

       template <std::size_t N=0>
       const std::string& inputLocation() const { return std::get<N>(m_inputs).objKey(); }
       unsigned int inputLocationSize() const { return std::tuple_size<decltype(m_inputs)>::value; }

   protected:
       std::tuple<details::InputHandle_t<Traits_,In>...>  m_inputs;
   };

   template <typename... Out, typename Traits_>
   class DataHandleMixin<std::tuple<Out...>, void,Traits_> : public BaseClass_t<Traits_> {
       static_assert( std::is_base_of<GaudiAlgorithm, BaseClass_t<Traits_>>::value,
                      "BaseClass must inherit from GaudiAlgorithm");
   public:
       using KeyValue = std::pair<std::string, std::string>;
       constexpr static std::size_t N_out = sizeof...(Out);

       // generic constructor:  0 -> N
       DataHandleMixin(const std::string& name, ISvcLocator* pSvcLocator,
                     const std::array<KeyValue,N_out>& outputs)
       : BaseClass_t<Traits_>( name , pSvcLocator ),
         m_outputs( make_tuple_of_handles<decltype(m_outputs)>( this, outputs, Gaudi::DataHandle::Writer ) )
       {
         declare_tuple_of_properties( this, outputs, m_outputs );
         // make sure this algorithm is seen as reentrant by Gaudi
         BaseClass_t<Traits_>::setProperty("Cardinality", 0);
       }

       // 0 -> 1
       DataHandleMixin(const std::string& name, ISvcLocator* locator,
                       const KeyValue& output)
       : DataHandleMixin( name, locator,  std::array<KeyValue,1>{ output } )
       { }

       template <std::size_t N=0>
       const std::string& outputLocation() const { return std::get<N>(m_outputs).objKey(); }
       unsigned int outputLocationSize() const { return std::tuple_size<decltype(m_outputs)>::value; }

   protected:
       std::tuple<details::OutputHandle_t<Traits_,Out>...> m_outputs;
   };

   /////////////////
   template <typename Fun, typename Container, typename... Args >
   constexpr void apply(const Fun&, Container&, Args... )
   { static_assert(sizeof...(Args)==0,"Args should not be used!");}

   // TODO/FIXME: overload ambiguity if output container type == input container type
   template <typename Fun, typename Container>
   auto apply(const Fun& fun, Container& c)  -> decltype(fun(c),void())
   { fun(c); }

   /////////////////

} } }

#endif
