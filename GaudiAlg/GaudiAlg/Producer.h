#ifndef GAUDI_FUNCTIONAL_PRODUCER_H
#define GAUDI_FUNCTIONAL_PRODUCER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/apply.h"
#include <utility>

namespace Gaudi
{
  namespace Functional
  {

    template <typename Signature, typename Traits_ = Traits::useDefaults>
    class Producer;

    template <typename... Out, typename Traits_>
    class Producer<std::tuple<Out...>(), Traits_> : public details::DataHandleMixin<std::tuple<Out...>, void, Traits_>
    {
    public:
      using details::DataHandleMixin<std::tuple<Out...>, void, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final
      {
        try {
          Gaudi::apply(
              [&]( auto&... ohandle ) {
                Gaudi::apply(
                    [&ohandle...]( auto&&... data ) {
#if __cplusplus < 201703L
                      (void)std::initializer_list<int>{
                          ( details::put( ohandle, std::forward<decltype( data )>( data ) ), 0 )...};
#else
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
#endif
                    },
                    details::as_const( *this )() );
              },
              this->m_outputs );
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
        return StatusCode::SUCCESS;
      }

      // ... instead, they must implement the following operator
      virtual std::tuple<Out...> operator()() const = 0;
    };

    template <typename Out, typename Traits_>
    class Producer<Out(), Traits_> : public details::DataHandleMixin<std::tuple<Out>, void, Traits_>
    {
    public:
      using details::DataHandleMixin<std::tuple<Out>, void, Traits_>::DataHandleMixin;
      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final
      {
        try {
          details::put( std::get<0>( this->m_outputs ), details::as_const( *this )() );
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
        return StatusCode::SUCCESS;
      }

      // ... instead, they must implement the following operator
      virtual Out operator()() const = 0;
    };
  }
}

#endif
