#ifndef SPLITTING_TRANSFORMER_H
#define SPLITTING_TRANSFORMER_H

#include <functional>
#include <string>
#include <vector>

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/apply.h"

namespace Gaudi
{
  namespace Functional
  {

    template <typename Signature, typename Traits_ = Traits::useDefaults>
    class SplittingTransformer;

    template <typename Container>
    using vector_of_ = std::vector<Container>;
    template <typename Container>
    using vector_of_optional_ = std::vector<boost::optional<Container>>;

    ////// N -> Many of the same one (value of Many not known at compile time, but known at configuration time)
    template <typename Out, typename... In, typename Traits_>
    class SplittingTransformer<vector_of_<Out>( const In&... ), Traits_>
        : public details::DataHandleMixin<void, std::tuple<In...>, Traits_>
    {
      using base_class = details::DataHandleMixin<void, std::tuple<In...>, Traits_>;

    public:
      constexpr static std::size_t N = sizeof...( In );
      using KeyValue                 = std::pair<std::string, std::string>;
      using KeyValues                = std::pair<std::string, std::vector<std::string>>;

      SplittingTransformer( const std::string& name, ISvcLocator* locator, const std::array<KeyValue, N>& inputs,
                            const KeyValues& output );

      SplittingTransformer( const std::string& name, ISvcLocator* locator, const KeyValue& input,
                            const KeyValues& output )
          : SplittingTransformer( name, locator, std::array<KeyValue, 1>{input}, output )
      {
        static_assert( N == 1, "single input argument requires single input signature" );
      }

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outputLocations[n]; }
      unsigned int                                    outputLocationSize() const { return m_outputLocations.size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final
      {
        try {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          using details::as_const;
          auto out = details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs );
          if ( out.size() != m_outputs.size() ) {
            throw GaudiException( "Error during transform: expected " + std::to_string( m_outputs.size() ) +
                                      " containers, got " + std::to_string( out.size() ) + " instead",
                                  this->name(), StatusCode::FAILURE );
          }
          for ( unsigned i = 0; i != out.size(); ++i ) details::put( m_outputs[i], std::move( out[i] ) );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an boost::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const In&... ) const = 0;

    private:
      template <typename T>
      using OutputHandle = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
      std::vector<std::string> m_outputLocations; // TODO/FIXME  for now: use a call-back to update the actual handles!
      std::vector<OutputHandle<Out>> m_outputs;
    };

    template <typename Out, typename... In, typename Traits_>
    SplittingTransformer<vector_of_<Out>( const In&... ), Traits_>::SplittingTransformer(
        const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N>& inputs,
        const KeyValues& outputs )
        : base_class( name, pSvcLocator, inputs ), m_outputLocations( outputs.second )
    {
      auto p = this->declareProperty( outputs.first, m_outputLocations );
      p->declareUpdateHandler( [=]( Gaudi::Details::PropertyBase& ) {
        this->m_outputs = details::make_vector_of_handles<decltype( this->m_outputs )>( this, m_outputLocations );
        if ( details::is_optional<Out>::value ) { // handle constructor does not (yet) allow to set optional flag... so
                                                  // do it explicitly here...
          std::for_each( this->m_outputs.begin(), this->m_outputs.end(), []( auto& h ) { h.setOptional( true ); } );
        }
      } );
      p->useUpdateHandler(); // invoke now, to be sure the input handles are synced with the property...
    }
  }
}

#endif
