#include "GaudiAlg/Consumer.h"
#include "GaudiAlg/MergingTransformer.h"
#include "GaudiAlg/Producer.h"
#include "GaudiAlg/ScalarTransformer.h"
#include "GaudiAlg/Transformer.h"
#include "GaudiKernel/KeyedContainer.h"
#include <cmath>
#include <numeric>

namespace Gaudi::Examples {

  // using LegacyBaseClass_t = Gaudi::Functional::Traits::BaseClass_t<::Algorithm>;
  using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

  struct CountingConsumer final : Gaudi::Functional::Consumer<void(), BaseClass_t> {
    using Gaudi::Functional::Consumer<void(), BaseClass_t>::Consumer;
    mutable Gaudi::Accumulators::MsgCounter<MSG::ERROR>   m_err{this, "This is not an error...", 3};
    mutable Gaudi::Accumulators::MsgCounter<MSG::WARNING> m_warn{this, "This is not a warning...", 2};
    mutable Gaudi::Accumulators::MsgCounter<MSG::INFO>    m_info{this, "This is not info...", 1};

    void operator()() const override {
      always() << "CountingConsumer: incrementing \"This is not an error\" twice" << endmsg;
      ++m_err;
      m_err += true;
      m_err += false; // should do nothing...
      always() << "CountingConsumer: incrementing \"This is not a warning\" twice" << endmsg;
      ++m_warn;
      m_warn += true;
      m_warn += false; // should do nothing...
      always() << "CountingConsumer: incrementing \"This is not info\" twice" << endmsg;
      ++m_info;
      m_info += true;
      m_info += false; // should do nothing...
    }
  };
  DECLARE_COMPONENT( CountingConsumer )

  struct IntDataProducer final : Gaudi::Functional::Producer<int(), BaseClass_t> {

    IntDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyInt" ) ) {}

    int operator()() const override {
      info() << "executing IntDataProducer, storing 7 into " << outputLocation() << endmsg;
      return 7;
    }
  };

  DECLARE_COMPONENT( IntDataProducer )

  struct VectorDataProducer final : Gaudi::Functional::Producer<std::vector<int>(), BaseClass_t> {

    VectorDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyVector" ) ) {}

    std::vector<int> operator()() const override {
      info() << "executing VectorDataProducer, storing [3,3,3,3] into " << outputLocation() << endmsg;
      return {3, 3, 3, 3};
    }
  };

  DECLARE_COMPONENT( VectorDataProducer )

  using int_container = KeyedContainer<KeyedObject<int>, Containers::HashMap>;
  struct KeyedDataProducer final : Gaudi::Functional::Producer<int_container(), BaseClass_t> {

    KeyedDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyKeyed" ) ) {}

    int_container operator()() const override {
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
        : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ) ) {}

    void operator()( const int& input ) const override {
      info() << "executing IntDataConsumer, consuming " << input << " from " << inputLocation() << endmsg;
    }
  };

  DECLARE_COMPONENT( IntDataConsumer )

  struct IntToFloatData final : Gaudi::Functional::Transformer<float( const int& ), BaseClass_t> {

    IntToFloatData( const std::string& name, ISvcLocator* svcLoc )
        : Transformer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ),
                       KeyValue( "OutputLocation", "/Event/MyFloat" ) ) {}

    float operator()( const int& input ) const override {
      info() << "Converting: " << input << " from " << inputLocation() << " and storing it into " << outputLocation()
             << endmsg;
      return input;
    }
  };

  DECLARE_COMPONENT( IntToFloatData )

  struct IntIntToFloatFloatData final
      : Gaudi::Functional::MultiTransformer<std::tuple<float, float>( const int&, const int& ), BaseClass_t> {
    IntIntToFloatFloatData( const std::string& name, ISvcLocator* svcLoc )
        : MultiTransformer(
              name, svcLoc,
              {KeyValue( "InputLocation1", {"/Event/MyInt"} ), KeyValue( "InputLocation2", {"/Event/MyOtherInt"} )},
              {KeyValue( "OutputLocation1", {"/Event/MyMultiFloat1"} ),
               KeyValue( "OutputLocation2", {"/Event/MyMultiFloat2"} )} ) {}

    std::tuple<float, float> operator()( const int& input1, const int& input2 ) const override {
      info() << "Number of inputs : " << inputLocationSize() << ", number of outputs : " << outputLocationSize()
             << endmsg;
      info() << "Converting " << input1 << " from " << inputLocation<0>() << " and " << input2 << " from "
             << inputLocation<1>() << endmsg;
      info() << "Storing results into " << outputLocation<0>() << " and " << outputLocation<1>() << endmsg;
      return std::tuple<float, float>{input1, input2};
    }
  };

  DECLARE_COMPONENT( IntIntToFloatFloatData )

  /** @brief Concatenates a list of input vectors into a single output vector.
   */
  struct IntVectorsToIntVector final
      : public Gaudi::Functional::MergingTransformer<
            std::vector<int>( const Gaudi::Functional::vector_of_const_<std::vector<int>>& ), BaseClass_t> {

    IntVectorsToIntVector( const std::string& name, ISvcLocator* svcLoc )
        : MergingTransformer( name, svcLoc, {"InputLocations", {}},
                              {"OutputLocation", "/Event/MyConcatenatedIntVector"} ) {}

    std::vector<int>
    operator()( const Gaudi::Functional::vector_of_const_<std::vector<int>>& intVectors ) const override {
      // Create a vector and pre-allocate enough space for the number of integers we have
      auto             nelements = std::accumulate( intVectors.begin(), intVectors.end(), 0,
                                        []( const auto a, const auto b ) { return a + b.size(); } );
      std::vector<int> out;
      out.reserve( nelements );
      // Concatenate the input vectors to form the output
      for ( const auto& intVector : intVectors ) {
        info() << "Concatening vector " << intVector << endmsg;
        out.insert( out.end(), intVector.begin(), intVector.end() );
      }
      info() << "Storing output vector " << out << " to " << outputLocation() << endmsg;
      return out;
    }
  };

  DECLARE_COMPONENT( IntVectorsToIntVector )

  struct FloatDataConsumer final : Gaudi::Functional::Consumer<void( const float& ), BaseClass_t> {

    FloatDataConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyFloat" ) ) {}

    void operator()( const float& input ) const override {
      info() << "executing FloatDataConsumer: " << input << endmsg;
    }
  };

  DECLARE_COMPONENT( FloatDataConsumer )

  struct ContextConsumer final : Gaudi::Functional::Consumer<void( const EventContext& ), BaseClass_t> {

    using Gaudi::Functional::Consumer<void( const EventContext& ), BaseClass_t>::Consumer;

    void operator()( const EventContext& ctx ) const override {
      info() << "executing ContextConsumer, got " << ctx << endmsg;
    }
  };

  DECLARE_COMPONENT( ContextConsumer )

  struct ContextIntConsumer final : Gaudi::Functional::Consumer<void( const EventContext&, const int& ), BaseClass_t> {

    ContextIntConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ) ) {}

    void operator()( const EventContext& ctx, const int& i ) const override {
      info() << "executing ContextIntConsumer, got context = " << ctx << ", int = " << i << endmsg;
    }
  };

  DECLARE_COMPONENT( ContextIntConsumer )

  struct VectorDoubleProducer final : Gaudi::Functional::Producer<std::vector<double>(), BaseClass_t> {

    VectorDoubleProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyVectorOfDoubles" ) ) {}

    std::vector<double> operator()() const override {
      info() << "storing vector<double> into " << outputLocation() << endmsg;
      return {12.34, 56.78, 90.12, 34.56, 78.90};
    }
  };

  DECLARE_COMPONENT( VectorDoubleProducer )

  struct FrExpTransformer final
      : Gaudi::Functional::MultiScalarTransformer<
            FrExpTransformer, std::tuple<std::vector<double>, std::vector<int>>( const std::vector<double>& ),
            BaseClass_t> {
    FrExpTransformer( const std::string& name, ISvcLocator* svcLoc )
        : MultiScalarTransformer( name, svcLoc, KeyValue{"InputDoubles", {"/Event/MyVectorOfDoubles"}},
                                  {KeyValue{"OutputFractions", {"/Event/MyVectorOfFractions"}},
                                   KeyValue{"OutputIntegers", {"/Event/MyVectorOfIntegers"}}} ) {}

    using MultiScalarTransformer::operator();

    std::tuple<double, int> operator()( const double& d ) const {
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
                                   KeyValue{"OutputIntegers", {"/Event/OptMyVectorOfIntegers"}}} ) {}

    using MultiScalarTransformer::operator();

    boost::optional<std::tuple<double, int>> operator()( const double& d ) const {
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
      : Gaudi::Functional::ScalarTransformer<
            LdExpTransformer, std::vector<double>( const std::vector<double>&, const std::vector<int>& ), BaseClass_t> {
    LdExpTransformer( const std::string& name, ISvcLocator* svcLoc )
        : ScalarTransformer( name, svcLoc,
                             {KeyValue{"InputFractions", {"/Event/MyVectorOfFractions"}},
                              KeyValue{"InputIntegers", {"/Event/MyVectorOfIntegers"}}},
                             {KeyValue{"OutputDoubles", {"/Event/MyNewVectorOfDoubles"}}} ) {}

    using ScalarTransformer::operator();

    double operator()( double frac, int i ) const {
      double d = std::ldexp( frac, i );
      info() << "Converting " << i << ", " << frac << " -> " << d << endmsg;
      return d;
    }
  };
  DECLARE_COMPONENT( LdExpTransformer )

  struct OptLdExpTransformer final
      : Gaudi::Functional::ScalarTransformer<OptLdExpTransformer,
                                             std::vector<double>( const std::vector<double>&, const std::vector<int>& ),
                                             BaseClass_t> {
    OptLdExpTransformer( const std::string& name, ISvcLocator* svcLoc )
        : ScalarTransformer( name, svcLoc,
                             {KeyValue{"InputFractions", {"/Event/MyVectorOfFractions"}},
                              KeyValue{"InputIntegers", {"/Event/MyVectorOfIntegers"}}},
                             {KeyValue{"OutputDoubles", {"/Event/MyOptVectorOfDoubles"}}} ) {}

    using ScalarTransformer::operator();

    boost::optional<double> operator()( const double& frac, const int& i ) const {
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

  struct VoidConsumer final : Gaudi::Functional::Consumer<void(), BaseClass_t> {

    using Consumer::Consumer;

    void operator()() const override { info() << "executing VoidConsumer" << endmsg; }
  };

  DECLARE_COMPONENT( VoidConsumer )
} // namespace Gaudi::Examples
