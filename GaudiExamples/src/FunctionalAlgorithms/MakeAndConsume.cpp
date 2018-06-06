#include <GaudiAlg/Consumer.h>
#include <GaudiAlg/Producer.h>
#include <GaudiAlg/Transformer.h>
#include <GaudiKernel/KeyedContainer.h>
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

    struct VectorDataProducer final : Gaudi::Functional::Producer<std::vector<int>(), BaseClass_t> {

      VectorDataProducer( const std::string& name, ISvcLocator* svcLoc )
          : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyVector" ) )
      {
      }

      std::vector<int> operator()() const override
      {
        info() << "executing VectorDataProducer, storing [3,3,3,3] into " << outputLocation() << endmsg;
        return {3, 3, 3, 3};
      }
    };

    DECLARE_COMPONENT( VectorDataProducer )

    using int_container = KeyedContainer<KeyedObject<int>, Containers::HashMap>;
    struct KeyedDataProducer final : Gaudi::Functional::Producer<int_container(), BaseClass_t> {

      KeyedDataProducer( const std::string& name, ISvcLocator* svcLoc )
          : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyKeyed" ) )
      {
      }

      int_container operator()() const override
      {
        int_container container;
        auto          a = new KeyedObject<int>{4};
        container.add( a );
        auto b = new KeyedObject<int>{5};
        container.add( b );
        info() << "executing KeyedDataProducer, storing [4,5] into " << outputLocation() << endmsg;
        return container;
      }
    };

    DECLARE_COMPONENT( KeyedDataProducer )
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
  }
}
