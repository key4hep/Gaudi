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

namespace Gaudi::Functional {
  namespace details {
    // add a baseclass in case it isn't defined yet...
    template <typename Tr, typename Base = FixTESPath<AlgTool>>
    using add_base_t = std::conditional_t<Gaudi::cpp17::is_detected_v<detail2::BaseClass_t, Tr>, Tr,
                                          Traits::use_<Tr, BaseClass_t<Base>>>;

    template <typename Signature, typename Traits>
    struct ToolBinder;

    template <typename IFace, typename... Args, typename Traits>
    class ToolBinder<Gaudi::Interface::BoxedInterface<IFace>( Args const&... ), Traits>
        : public extends<details::BaseClass_t<Traits>, Gaudi::Interface::IBinder<IFace>> {

      constexpr static std::size_t N = sizeof...( Args );

      template <typename IArgs, std::size_t... I>
      ToolBinder( std::string type, std::string name, const IInterface* parent, IArgs&& args,
                  std::index_sequence<I...> )
          : extends<details::BaseClass_t<Traits>, Gaudi::Interface::IBinder<IFace>>{ std::move( type ),
                                                                                     std::move( name ), parent }
          , m_handles{ std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( args ) )... } {}

      std::tuple<details::InputHandle_t<Traits, Args>...> m_handles;

    public:
      using KeyValue = std::pair<std::string, std::string>;
      ToolBinder( std::string type, std::string name, const IInterface* parent,
                  Gaudi::Functional::details::RepeatValues_<KeyValue, N> const& inputs )
          : ToolBinder{ std::move( type ), std::move( name ), parent, inputs, std::make_index_sequence<N>{} } {}

      virtual Gaudi::Interface::BoxedInterface<IFace> bind( const Args&... args ) const = 0;

      Gaudi::Interface::BoxedInterface<IFace> operator()( EventContext const& ctx ) const override final {
        return std::apply(
            [&]( auto const&... arg ) {
              using namespace details;
              return this->bind( get( arg, *this, ctx )... );
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
    };
  } // namespace details

  template <typename Signature, typename Traits_ = Traits::use_<Traits::BaseClass_t<AlgTool>>>
  using ToolBinder = details::ToolBinder<Signature, Traits_>;

} // namespace Gaudi::Functional
