#ifndef GAUDI_FUNCTIONAL_PRODUCER_H
#define GAUDI_FUNCTIONAL_PRODUCER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    struct Producer;

    template <typename... Out, typename Traits_>
    struct Producer<std::tuple<Out...>(), Traits_, true> : DataHandleMixin<std::tuple<Out...>, void, Traits_> {
      using DataHandleMixin<std::tuple<Out...>, void, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        try {
          std::apply(
              [&]( auto&... ohandle ) {
                GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
                std::apply( [&ohandle...](
                                auto&&... data ) { ( put( ohandle, std::forward<decltype( data )>( data ) ), ... ); },
                            std::as_const( *this )() );
                GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
              },
              this->m_outputs );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual std::tuple<Out...> operator()() const = 0;
    };

    template <typename... Out, typename Traits_>
    struct Producer<std::tuple<Out...>(), Traits_, false> : DataHandleMixin<std::tuple<Out...>, void, Traits_> {
      using DataHandleMixin<std::tuple<Out...>, void, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ) const override final {
        try {
          std::apply(
              GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN

                  [&]( auto&... ohandle ) {
                    std::apply(
                        [&ohandle...]( auto&&... data ) {
                          ( put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                        },
                        ( *this )() );
                  },
              GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END

              this->m_outputs );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual std::tuple<Out...> operator()() const = 0;
    };

    template <typename Out, typename Traits_>
    struct Producer<Out(), Traits_, true> : public DataHandleMixin<std::tuple<Out>, void, Traits_> {
      using DataHandleMixin<std::tuple<Out>, void, Traits_>::DataHandleMixin;
      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        try {
          put( std::get<0>( this->m_outputs ), std::as_const( *this )() );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual Out operator()() const = 0;
    };

    template <typename Out, typename Traits_>
    struct Producer<Out(), Traits_, false> : public DataHandleMixin<std::tuple<Out>, void, Traits_> {
      using DataHandleMixin<std::tuple<Out>, void, Traits_>::DataHandleMixin;
      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ) const override final {
        try {
          put( std::get<0>( this->m_outputs ), ( *this )() );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual Out operator()() const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using Producer = details::Producer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
