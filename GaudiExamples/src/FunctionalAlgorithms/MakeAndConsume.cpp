#include <GaudiAlg/Consumer.h>
#include <GaudiAlg/Producer.h>
#include <GaudiAlg/Transformer.h>
#include <GaudiKernel/MsgStream.h>

namespace Gaudi
{
  namespace Examples
  {

    using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Algorithm>;

    struct IntDataProducer final : Gaudi::Functional::Producer<int(), BaseClass_t> {

      IntDataProducer( const std::string& name, ISvcLocator* svcLoc )
          : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyInt" ) )
      {
      }

      int operator()() const override
      {
        info() << "executing IntDataProducer, storing 7 into " << outputLocation() << endmsg;
        return 7;
      }
    };

    DECLARE_COMPONENT( IntDataProducer )

    struct IntDataConsumer final : Gaudi::Functional::Consumer<void( const int& ), BaseClass_t> {

      IntDataConsumer( const std::string& name, ISvcLocator* svcLoc )
          : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ) )
      {
      }

      void operator()( const int& input ) const override
      {
        info() << "executing IntDataConsumer, consuming " << input << " from " << inputLocation() << endmsg;
      }
    };

    DECLARE_COMPONENT( IntDataConsumer )

    struct IntToFloatData final : Gaudi::Functional::Transformer<float( const int& ), BaseClass_t> {

      IntToFloatData( const std::string& name, ISvcLocator* svcLoc )
          : Transformer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ),
                         KeyValue( "OutputLocation", "/Event/MyFloat" ) )
      {
      }

      float operator()( const int& input ) const override
      {
        info() << "Converting: " << input << " from " << inputLocation() << " and storing it into " << outputLocation()
               << endmsg;
        return input;
      }
    };

    DECLARE_COMPONENT( IntToFloatData )

    class IntIntToFloatFloatData final
        : public Gaudi::Functional::MultiTransformer<std::tuple<float, float>( const int&, const int& ), BaseClass_t>
    {
    public:
      IntIntToFloatFloatData( const std::string& name, ISvcLocator* svcLoc )
          : MultiTransformer( name, svcLoc, {KeyValue( "InputLocation1", {"/Event/MyInt"} ),
                                             KeyValue( "InputLocation2", {"/Event/MyOtherInt"} )},
                              {KeyValue( "OutputLocation1", {"/Event/MyMultiFloat1"} ),
                               KeyValue( "OutputLocation2", {"/Event/MyMultiFloat2"} )} )
      {
      }

      std::tuple<float, float> operator()( const int& input1, const int& input2 ) const override
      {
        info() << "Number of inputs : " << inputLocationSize() << ", number of outputs : " << outputLocationSize()
               << endmsg;
        info() << "Converting " << input1 << " from " << inputLocation<0>() << " and " << input2 << " from "
               << inputLocation<1>() << endmsg;
        info() << "Storing results into " << outputLocation<0>() << " and " << outputLocation<1>() << endmsg;
        return std::tuple<float, float>{input1, input2};
      }
    };

    DECLARE_COMPONENT( IntIntToFloatFloatData )

    struct FloatDataConsumer final : Gaudi::Functional::Consumer<void( const float& ), BaseClass_t> {

      FloatDataConsumer( const std::string& name, ISvcLocator* svcLoc )
          : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyFloat" ) )
      {
      }

      void operator()( const float& input ) const override
      {
        info() << "executing FloatDataConsumer: " << input << endmsg;
      }
    };

    DECLARE_COMPONENT( FloatDataConsumer )

    struct ContextConsumer final : Gaudi::Functional::Consumer<void( const EventContext& )> {

      using Gaudi::Functional::Consumer<void( const EventContext& )>::Consumer;

      void operator()( const EventContext& ctx ) const override
      {
        info() << "executing ContextConsumer, got " << ctx << endmsg;
      }
    };

    DECLARE_COMPONENT( ContextConsumer )

    struct ContextIntConsumer final : Gaudi::Functional::Consumer<void( const EventContext&, const int& )> {

      ContextIntConsumer( const std::string& name, ISvcLocator* svcLoc )
          : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ) )
      {
      }

      void operator()( const EventContext& ctx, const int& i ) const override
      {
        info() << "executing ContextIntConsumer, got context = " << ctx << ", int = " << i << endmsg;
      }
    };

    DECLARE_COMPONENT( ContextIntConsumer )
  }
}
