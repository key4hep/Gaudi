#ifndef MERGING_TRANSFORMER_H
#define MERGING_TRANSFORMER_H

#include <functional>
#include <string>
#include <vector>

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi::Functional {

  using details::vector_of_const_;

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    class MergingTransformer;

    ////// Many of the same -> 1
    template <typename Out, typename In, typename Traits_>
    class MergingTransformer<Out( const vector_of_const_<In>& ), Traits_, true>
        : public details::DataHandleMixin<std::tuple<Out>, void, Traits_> {
      using base_class = details::DataHandleMixin<std::tuple<Out>, void, Traits_>;

    public:
      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;

      MergingTransformer( const std::string& name, ISvcLocator* locator, const KeyValues& inputs,
                          const KeyValue& output )
          : base_class( name, locator, output )
          , m_inputLocations{this, inputs.first, inputs.second,
                             [=]( Gaudi::Details::PropertyBase& ) {
                               this->m_inputs = details::make_vector_of_handles<decltype( this->m_inputs )>(
                                   this, m_inputLocations );
                               if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                                              // optional flag... so do it
                                                              // explicitly here...
                                 std::for_each( this->m_inputs.begin(), this->m_inputs.end(),
                                                []( auto& h ) { h.setOptional( true ); } );
                               }
                             },
                             Gaudi::Details::Property::ImmediatelyInvokeHandler{true}} {}

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n]; }
      unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        vector_of_const_<In> ins;
        ins.reserve( m_inputs.size() );
        std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ),
                        details::details2::get_from_handle<In>{} );
        try {
          details::put( std::get<0>( this->m_outputs ), std::as_const( *this )( std::as_const( ins ) ) );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      virtual Out operator()( const vector_of_const_<In>& inputs ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = details::InputHandle_t<Traits_, typename std::remove_pointer<T>::type>;
      std::vector<InputHandle_t<In>>            m_inputs;         //   and make the handles properties instead...
      Gaudi::Property<std::vector<std::string>> m_inputLocations; // TODO/FIXME: remove this duplication...
      // TODO/FIXME: replace vector of string property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
    };

    template <typename Out, typename In, typename Traits_>
    class MergingTransformer<Out( const vector_of_const_<In>& ), Traits_, false>
        : public details::DataHandleMixin<std::tuple<Out>, void, Traits_> {
      using base_class = details::DataHandleMixin<std::tuple<Out>, void, Traits_>;

    public:
      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;

      MergingTransformer( const std::string& name, ISvcLocator* locator, const KeyValues& inputs,
                          const KeyValue& output )
          : base_class( name, locator, output )
          , m_inputLocations{this, inputs.first, inputs.second,
                             [=]( Gaudi::Details::PropertyBase& ) {
                               this->m_inputs = details::make_vector_of_handles<decltype( this->m_inputs )>(
                                   this, m_inputLocations );
                               if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                                              // optional flag... so do it
                                                              // explicitly here...
                                 std::for_each( this->m_inputs.begin(), this->m_inputs.end(),
                                                []( auto& h ) { h.setOptional( true ); } );
                               }
                             },
                             Gaudi::Details::Property::ImmediatelyInvokeHandler{true}} {}

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n]; }
      unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ) const override final {
        vector_of_const_<In> ins;
        ins.reserve( m_inputs.size() );
        std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ),
                        details::details2::get_from_handle<In>{} );
        try {
          details::put( std::get<0>( this->m_outputs ), ( *this )( std::as_const( ins ) ) );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      virtual Out operator()( const vector_of_const_<In>& inputs ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = details::InputHandle_t<Traits_, typename std::remove_pointer<T>::type>;
      std::vector<InputHandle_t<In>>            m_inputs;         //   and make the handles properties instead...
      Gaudi::Property<std::vector<std::string>> m_inputLocations; // TODO/FIXME: remove this duplication...
      // TODO/FIXME: replace vector of string property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MergingTransformer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
