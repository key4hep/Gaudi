/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiKernel/IBinder.h"

#define GAUDI_FUNCTIONAL_TOOL_BINDER_USES_CREATE

namespace Gaudi::Functional {
  namespace details {
    // add a baseclass in case it isn't defined yet...
    template <typename Tr, typename Base = FixTESPath<AlgTool>>
    using add_base_t = std::conditional_t<Gaudi::cpp17::is_detected_v<detail2::BaseClass_t, Tr>, Tr,
                                          Traits::use_<Tr, BaseClass_t<Base>>>;

    template <typename Signature, typename Traits>
    class ToolBinder;

    template <typename IFace, typename... Args, typename Traits>
    class ToolBinder<Gaudi::Interface::Bind::Box<IFace>( Args const&... ), Traits>
        : public extends<details::BaseClass_t<Traits, AlgTool>, Gaudi::Interface::Bind::IBinder<IFace>> {

      constexpr static std::size_t N = sizeof...( Args );

      template <typename IArgs, std::size_t... I>
      ToolBinder( std::string type, std::string name, const IInterface* parent, IArgs&& args,
                  Gaudi::Interface::Bind::Box<IFace> ( *creator )( void const*, Args const&... ),
                  std::index_sequence<I...> )
          : extends<details::BaseClass_t<Traits>, Gaudi::Interface::Bind::IBinder<IFace>>{ std::move( type ),
                                                                                           std::move( name ), parent }
          , m_handles{ std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( args ) )... }
          , m_creator{ creator } {}

      std::tuple<details::InputHandle_t<Traits, Args>...> m_handles;
      Gaudi::Interface::Bind::Box<IFace> ( *m_creator )( void const*, Args const&... );

    public:
      using KeyValue = std::pair<std::string, std::string>;
      ToolBinder( std::string type, std::string name, const IInterface* parent,
                  Gaudi::Functional::details::RepeatValues_<KeyValue, N> const& inputs,
                  Gaudi::Interface::Bind::Box<IFace> ( *creator )( void const*, Args const&... ) )
          : ToolBinder{ std::move( type ), std::move( name ), parent, inputs, creator, std::make_index_sequence<N>{} } {
      }

      Gaudi::Interface::Bind::Box<IFace> bind( EventContext const& ctx ) const final {
        return std::apply(
            [&]( auto const&... arg ) {
              using namespace details;
              return std::invoke( m_creator, this, get( arg, *this, ctx )... );
            },
            m_handles );
      }

      template <std::size_t N = 0>
      decltype( auto ) inputLocation() const {
        using namespace details;
        return getKey( std::get<N>( m_handles ) );
      }
      template <typename T>
      decltype( auto ) inputLocation() const {
        using namespace details;
        return getKey( std::get<InputHandle_t<Traits, std::decay_t<T>>>( m_handles ) );
      }

      // TODO: make this a callable instance?
      template <typename BoundInstance, typename Self>
      static auto construct( Self* ) {
        static_assert( std::is_base_of_v<ToolBinder, Self> );
        return +[]( void const* ptr, const Args&... args ) {
          return Gaudi::Interface::Bind::Box<IFace>{ std::in_place_type<BoundInstance>,
                                                     static_cast<std::add_const_t<Self>*>( ptr ), args... };
        };
      }
    };
  } // namespace details

  template <typename Signature, typename Traits_ = Traits::use_<Traits::BaseClass_t<AlgTool>>>
  using ToolBinder = details::ToolBinder<Signature, Traits_>;

} // namespace Gaudi::Functional
