#include "GaudiAlg/Consumer.h"
#include "GaudiAlg/Producer.h"
#include "GaudiAlg/ScalarTransformer.h"
#include "GaudiAlg/Transformer.h"
#include "GaudiKernel/KeyedContainer.h"
#include <cmath>

namespace Gaudi
{
  namespace Examples
  {

    using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<::Algorithm>;

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

    struct IntIntToFloatFloatData final
        : Gaudi::Functional::MultiTransformer<std::tuple<float, float>( const int&, const int& ), BaseClass_t> {
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

    struct VectorDoubleProducer final : Gaudi::Functional::Producer<std::vector<double>(), BaseClass_t> {

      VectorDoubleProducer( const std::string& name, ISvcLocator* svcLoc )
          : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyVectorOfDoubles" ) )
      {
      }

      std::vector<double> operator()() const override
      {
        info() << "storing vector<double> into " << outputLocation() << endmsg;
        return {12.34, 56.78, 90.12, 34.56, 78.90};
      }
    };

    DECLARE_COMPONENT( VectorDoubleProducer )

    struct FrExpTransformer final
        : Gaudi::Functional::MultiScalarTransformer<FrExpTransformer, std::tuple<std::vector<double>, std::vector<int>>(
                                                                          const std::vector<double>& ),
                                                    BaseClass_t> {
      FrExpTransformer( const std::string& name, ISvcLocator* svcLoc )
          : MultiScalarTransformer( name, svcLoc, KeyValue{"InputDoubles", {"/Event/MyVectorOfDoubles"}},
                                    {KeyValue{"OutputFractions", {"/Event/MyVectorOfFractions"}},
                                     KeyValue{"OutputIntegers", {"/Event/MyVectorOfIntegers"}}} )
      {
      }

      using MultiScalarTransformer::operator();

      std::tuple<double, int> operator()( const double& d ) const
      {
        int    i;
        double frac = std::frexp( d, &i );
        info() << "Converting " << d << " -> " << frac << ", " << i << endmsg;
        return {frac, i};
      }
    };
    DECLARE_COMPONENT( FrExpTransformer )

    struct OptFrExpTransformer final
        : Gaudi::Functional::MultiScalarTransformer<
              OptFrExpTransformer, std::tuple<std::vector<double>, std::vector<int>>( const std::vector<double>& ),
              BaseClass_t> {
      OptFrExpTransformer( const std::string& name, ISvcLocator* svcLoc )
          : MultiScalarTransformer( name, svcLoc, KeyValue{"InputDoubles", {"/Event/MyVectorOfDoubles"}},
                                    {KeyValue{"OutputFractions", {"/Event/OptMyVectorOfFractions"}},
                                     KeyValue{"OutputIntegers", {"/Event/OptMyVectorOfIntegers"}}} )
      {
      }

      using MultiScalarTransformer::operator();

      boost::optional<std::tuple<double, int>> operator()( const double& d ) const
      {
        if ( d < 30. ) {
          info() << "Skipping " << d << endmsg;
          return {};
        }
        int    i;
        double frac = std::frexp( d, &i );
        info() << "Converting " << d << " -> " << frac << ", " << i << endmsg;
        return std::make_tuple( frac, i );
      }
    };
    DECLARE_COMPONENT( OptFrExpTransformer )

    struct LdExpTransformer final
        : Gaudi::Functional::ScalarTransformer<LdExpTransformer, std::vector<double>( const std::vector<double>&,
                                                                                      const std::vector<int>& ),
                                               BaseClass_t> {
      LdExpTransformer( const std::string& name, ISvcLocator* svcLoc )
          : ScalarTransformer( name, svcLoc, {KeyValue{"InputFractions", {"/Event/MyVectorOfFractions"}},
                                              KeyValue{"InputIntegers", {"/Event/MyVectorOfIntegers"}}},
                               {KeyValue{"OutputDoubles", {"/Event/MyNewVectorOfDoubles"}}} )
      {
      }

      using ScalarTransformer::operator();

      double operator()( double frac, int i ) const
      {
        double d = std::ldexp( frac, i );
        info() << "Converting " << i << ", " << frac << " -> " << d << endmsg;
        return d;
      }
    };
    DECLARE_COMPONENT( LdExpTransformer )

    struct OptLdExpTransformer final
        : Gaudi::Functional::ScalarTransformer<OptLdExpTransformer, std::vector<double>( const std::vector<double>&,
                                                                                         const std::vector<int>& ),
                                               BaseClass_t> {
      OptLdExpTransformer( const std::string& name, ISvcLocator* svcLoc )
          : ScalarTransformer( name, svcLoc, {KeyValue{"InputFractions", {"/Event/MyVectorOfFractions"}},
                                              KeyValue{"InputIntegers", {"/Event/MyVectorOfIntegers"}}},
                               {KeyValue{"OutputDoubles", {"/Event/MyOptVectorOfDoubles"}}} )
      {
      }

      using ScalarTransformer::operator();

      boost::optional<double> operator()( const double& frac, const int& i ) const
      {
        double d = std::ldexp( frac, i );
        if ( i > 6 ) {
          info() << "Skipping " << d << endmsg;
          return {};
        }
        info() << "Converting " << i << ", " << frac << " -> " << d << endmsg;
        return d;
      }
    };
    DECLARE_COMPONENT( OptLdExpTransformer )

    struct VoidConsumer final : Gaudi::Functional::Consumer<void()> {

      using Consumer::Consumer;

      void operator()() const override { info() << "executing VoidConsumer" << endmsg; }
    };

    DECLARE_COMPONENT( VoidConsumer )
  }
}
