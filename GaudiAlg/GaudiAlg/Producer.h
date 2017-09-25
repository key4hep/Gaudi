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
      StatusCode execute() override final { return invoke( std::index_sequence_for<Out...>{} ); }

      // ... instead, they must implement the following operator
      virtual std::tuple<Out...> operator()() const = 0;

    private:
      template <std::size_t... O>
      StatusCode invoke( std::index_sequence<O...> )
      {
        using details::as_const;
        using details::put;
        try {
          std::initializer_list<int>{
              ( put( std::get<O>( this->m_outputs ), std::get<O>( as_const( *this )() ) ), 0 )...};
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
        return StatusCode::SUCCESS;
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
          this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
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
