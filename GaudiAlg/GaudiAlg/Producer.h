#ifndef GAUDI_FUNCTIONAL_PRODUCER_H
#define GAUDI_FUNCTIONAL_PRODUCER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
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
          invoke( std::index_sequence_for<Out...>{} );
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
        return StatusCode::SUCCESS;
      }

      // ... instead, they must implement the following operator
      virtual std::tuple<Out...> operator()() const = 0;

    private:
      template <std::size_t... O>
      void invoke( std::index_sequence<O...> )
      {
        std::initializer_list<int>{
            ( details::put( std::get<O>( this->m_outputs ), std::get<O>( details::as_const( *this )() ) ), 0 )...};
      }
    };

    template <typename Out, typename Traits_>
    class Producer<Out(), Traits_> : public details::DataHandleMixin<std::tuple<Out>, void, Traits_>
    {
    public:
      using details::DataHandleMixin<std::tuple<Out>, void, Traits_>::DataHandleMixin;
      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final
      {
        using details::as_const;
        using details::put;
        try {
          put( std::get<0>( this->m_outputs ), as_const( *this )() );
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
