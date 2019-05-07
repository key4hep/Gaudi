#ifndef SPLITTING_TRANSFORMER_H
#define SPLITTING_TRANSFORMER_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi::Functional {

  template <typename Container>
  using vector_of_ = std::vector<Container>;
  template <typename Container>
  using vector_of_optional_ = std::vector<std::optional<Container>>;

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    class SplittingTransformer;

    ////// N -> Many of the same one (value of Many not known at compile time, but known at configuration time)
    template <typename Out, typename... In, typename Traits_>
    class SplittingTransformer<vector_of_<Out>( const In&... ), Traits_, true>
        : public details::DataHandleMixin<void, std::tuple<In...>, Traits_> {
      using base_class = details::DataHandleMixin<void, std::tuple<In...>, Traits_>;

    public:
      constexpr static std::size_t N = base_class::N_in;
      using KeyValue                 = typename base_class::KeyValue;
      using KeyValues                = typename base_class::KeyValues;

      SplittingTransformer( const std::string& name, ISvcLocator* locator, const std::array<KeyValue, N>& inputs,
                            const KeyValues& outputs )
          : base_class( name, locator, inputs )
          , m_outputLocations(
                this, outputs.first, outputs.second,
                [=]( Gaudi::Details::PropertyBase& ) {
                  this->m_outputs =
                      details::make_vector_of_handles<decltype( this->m_outputs )>( this, m_outputLocations );
                  if constexpr ( details::is_optional_v<Out> ) { // handle constructor does not (yet) allow to
                                                                 // set optional flag... so
                                                                 // do it explicitly here...
                    std::for_each( this->m_outputs.begin(), this->m_outputs.end(),
                                   []( auto& h ) { h.setOptional( true ); } );
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true} ) {}

      SplittingTransformer( const std::string& name, ISvcLocator* locator, const KeyValue& input,
                            const KeyValues& output )
          : SplittingTransformer( name, locator, std::array<KeyValue, 1>{input}, output ) {
        static_assert( N == 1, "single input argument requires single input signature" );
      }

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outputLocations.value()[n]; }
      unsigned int       outputLocationSize() const { return m_outputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
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
      std::vector<OutputHandle<Out>>            m_outputs;
      Gaudi::Property<std::vector<std::string>> m_outputLocations; // TODO/FIXME  for now: use a call-back to update the
                                                                   // actual handles!
    };

    template <typename Out, typename... In, typename Traits_>
    class SplittingTransformer<vector_of_<Out>( const In&... ), Traits_, false>
        : public details::DataHandleMixin<void, std::tuple<In...>, Traits_> {
      using base_class = details::DataHandleMixin<void, std::tuple<In...>, Traits_>;

    public:
      constexpr static std::size_t N = base_class::N_in;
      using KeyValue                 = typename base_class::KeyValue;
      using KeyValues                = typename base_class::KeyValues;

      SplittingTransformer( const std::string& name, ISvcLocator* locator, const std::array<KeyValue, N>& inputs,
                            const KeyValues& outputs )
          : base_class( name, locator, inputs )
          , m_outputLocations(
                this, outputs.first, outputs.second,
                [=]( Gaudi::Details::PropertyBase& ) {
                  this->m_outputs =
                      details::make_vector_of_handles<decltype( this->m_outputs )>( this, m_outputLocations );
                  if constexpr ( details::is_optional_v<Out> ) { // handle constructor does not (yet) allow to
                                                                 // set optional flag... so
                                                                 // do it explicitly here...
                    std::for_each( this->m_outputs.begin(), this->m_outputs.end(),
                                   []( auto& h ) { h.setOptional( true ); } );
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{true} ) {}

      SplittingTransformer( const std::string& name, ISvcLocator* locator, const KeyValue& input,
                            const KeyValues& output )
          : SplittingTransformer( name, locator, std::array<KeyValue, 1>{input}, output ) {
        static_assert( N == 1, "single input argument requires single input signature" );
      }

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outputLocations.value()[n]; }
      unsigned int       outputLocationSize() const { return m_outputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          auto out = details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs );
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
      std::vector<OutputHandle<Out>>            m_outputs;
      Gaudi::Property<std::vector<std::string>> m_outputLocations; // TODO/FIXME  for now: use a call-back to update the
                                                                   // actual handles!
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using SplittingTransformer = details::SplittingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
