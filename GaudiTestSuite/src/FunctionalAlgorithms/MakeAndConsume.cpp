/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Consumer.h>
#include <Gaudi/Functional/MergingTransformer.h>
#include <Gaudi/Functional/Producer.h>
#include <Gaudi/Functional/ScalarTransformer.h>
#include <Gaudi/Functional/SplittingMergingTransformer.h>
#include <Gaudi/Functional/ToolBinder.h>
#include <Gaudi/Functional/Transformer.h>
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IBinder.h>
#include <GaudiKernel/KeyedContainer.h>
#include <GaudiKernel/SharedObjectsContainer.h>
#include <cmath>
#include <numeric>
#include <optional>

namespace Gaudi::TestSuite {

  struct IMyTool : extend_interfaces<IAlgTool> {
    DeclareInterfaceID( IMyTool, 1, 0 );
    virtual void operator()() const = 0;
  };

  struct MyExampleTool : extends<AlgTool, IMyTool> {
    using extends::extends;
    void                         operator()() const override { always() << m_message.value() << endmsg; }
    Gaudi::Property<std::string> m_message{ this, "Message", "Boring Default Message" };
  };
  DECLARE_COMPONENT( MyExampleTool )

  struct MyConsumerTool final : Gaudi::Functional::ToolBinder<Gaudi::Interface::Bind::Box<IMyTool>( const int& )> {
    MyConsumerTool( std::string type, std::string name, const IInterface* parent )
        : ToolBinder{ std::move( type ), std::move( name ), parent, KeyValue{ "MyInt", "/Event/MyInt" },
                      construct<BoundInstance>( this ) } {}

    class BoundInstance final : public Gaudi::Interface::Bind::Stub<IMyTool> {
      MyConsumerTool const* parent;
      int                   i;

    public:
      BoundInstance( MyConsumerTool const* parent, const int& i ) : parent{ parent }, i{ i } {}
      void operator()() const override {
        parent->always() << "BoundInstance - got: " << i << " from " << parent->inputLocation<int>() << endmsg;
      }
    };
  };
  DECLARE_COMPONENT( MyConsumerTool )

  using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

  struct ToolConsumer final : Gaudi::Functional::Consumer<void( IMyTool const& ), BaseClass_t> {
    ToolConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, KeyValue{ "MyTool", "MyExampleTool" } ) {}

    void operator()( IMyTool const& tool ) const override { tool(); }
  };
  DECLARE_COMPONENT( ToolConsumer )

  struct CountingConsumer final : Gaudi::Functional::Consumer<void(), BaseClass_t> {
    using Gaudi::Functional::Consumer<void(), BaseClass_t>::Consumer;
    mutable Gaudi::Accumulators::MsgCounter<MSG::ERROR>   m_err{ this, "This is not an error...", 3 };
    mutable Gaudi::Accumulators::MsgCounter<MSG::WARNING> m_warn{ this, "This is not a warning...", 2 };
    mutable Gaudi::Accumulators::MsgCounter<MSG::INFO>    m_info{ this, "This is not info...", 1 };

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
      info() << "executing IntDataProducer, storing " << m_value.value() << " into " << outputLocation() << endmsg;
      return m_value;
    }

    Gaudi::Property<int> m_value{ this, "Value", 7, "The integer value to produce." };
  };

  DECLARE_COMPONENT( IntDataProducer )

  struct VectorDataProducer final : Gaudi::Functional::Producer<std::vector<int>(), BaseClass_t> {

    Gaudi::Property<std::vector<int>> m_data{ this, "Data", { 3, 3, 3, 3 } };

    VectorDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyVector" ) ) {}

    std::vector<int> operator()() const override {
      info() << "executing VectorDataProducer, storing " << m_data.value() << " into " << outputLocation() << endmsg;
      return m_data;
    }
  };

  DECLARE_COMPONENT( VectorDataProducer )

  using int_container = KeyedContainer<KeyedObject<int>, Containers::HashMap>;
  struct KeyedDataProducer final : Gaudi::Functional::Producer<int_container(), BaseClass_t> {

    KeyedDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyKeyed" ) ) {}

    int_container operator()() const override {
      int_container container;
      auto          a = new KeyedObject<int>{ 4 };
      container.add( a );
      auto b = new KeyedObject<int>{ 5 };
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

  struct IntFloatToFloatData final : Gaudi::Functional::Transformer<float( const int&, const float& ), BaseClass_t> {

    IntFloatToFloatData( const std::string& name, ISvcLocator* svcLoc )
        : Transformer( name, svcLoc,
                       { KeyValue( "InputLocation", "/Event/MyInt" ), KeyValue{ "OtherInput", "/Event/MyOtherFloat" } },
                       KeyValue( "OutputLocation", "/Event/OtherFloat" ) ) {}

    float operator()( const int& in1, const float& in2 ) const override {
      info() << "Converting: " << in1 << " from " << inputLocation<int>() << " and " << in2 << " from "
             << inputLocation<float>() << " and storing it into " << outputLocation() << endmsg;
      return in1 + in2;
    }
  };

  DECLARE_COMPONENT( IntFloatToFloatData )

  struct IntIntToFloatFloatData final
      : Gaudi::Functional::MultiTransformer<std::tuple<float, float>( const int&, const int& ), BaseClass_t> {
    IntIntToFloatFloatData( const std::string& name, ISvcLocator* svcLoc )
        : MultiTransformer( name, svcLoc,
                            { KeyValue( "InputLocation1", { "/Event/MyInt" } ),
                              KeyValue( "InputLocation2", { "/Event/MyOtherInt" } ) },
                            { KeyValue( "OutputLocation1", { "/Event/MyMultiFloat1" } ),
                              KeyValue( "OutputLocation2", { "/Event/MyMultiFloat2" } ) } ) {}

    std::tuple<float, float> operator()( const int& input1, const int& input2 ) const override {
      info() << "Number of inputs : " << inputLocationSize() << ", number of outputs : " << outputLocationSize()
             << endmsg;
      info() << "Converting " << input1 << " from " << inputLocation<0>() << " and " << input2 << " from "
             << inputLocation<1>() << endmsg;
      info() << "Storing results into " << outputLocation<0>() << " and " << outputLocation<1>() << endmsg;
      return std::tuple<float, float>{ input1, input2 };
    }
  };

  DECLARE_COMPONENT( IntIntToFloatFloatData )

  /** @brief Concatenates a list of input vectors into a single output vector.
   */
  struct IntVectorsToIntVector final
      : public Gaudi::Functional::MergingTransformer<
            std::vector<int>( const Gaudi::Functional::vector_of_const_<std::vector<int>>& ), BaseClass_t> {

    IntVectorsToIntVector( const std::string& name, ISvcLocator* svcLoc )
        : MergingTransformer( name, svcLoc, { "InputLocations", {} },
                              { "OutputLocation", "/Event/MyConcatenatedIntVector" } ) {}

    std::vector<int>
    operator()( const Gaudi::Functional::vector_of_const_<std::vector<int>>& intVectors ) const override {
      // Create a vector and pre-allocate enough space for the number of integers we have
      auto             nelements = std::accumulate( intVectors.begin(), intVectors.end(), 0,
                                                    []( const auto a, const auto b ) { return a + b.size(); } );
      std::vector<int> out;
      out.reserve( nelements );
      // Concatenate the input vectors to form the output
      for ( auto& intVector : intVectors ) {
        info() << "Concatening vector " << intVector << endmsg;
        out.insert( out.end(), intVector.begin(), intVector.end() );
        // intVector.clear(); // should not be possible!!! and does indeed not compile ;-)
      }
      info() << "Storing output vector " << out << " to " << outputLocation() << endmsg;
      return out;
    }
  };

  DECLARE_COMPONENT( IntVectorsToIntVector )

  struct PIntVectorsToIntVector final
      : public Gaudi::Functional::MergingTransformer<
            std::vector<int>( const Gaudi::Functional::vector_of_const_<std::vector<int>*>& ), BaseClass_t> {

    PIntVectorsToIntVector( const std::string& name, ISvcLocator* svcLoc )
        : MergingTransformer( name, svcLoc, { "InputLocations", {} },
                              { "OutputLocation", "/Event/MyConcatenatedIntVector" } ) {}

    std::vector<int>
    operator()( const Gaudi::Functional::vector_of_const_<std::vector<int>*>& intVectors ) const override {
      // Create a vector and pre-allocate enough space for the number of integers we have
      auto             nelements = std::accumulate( intVectors.begin(), intVectors.end(), 0,
                                                    []( const auto a, const auto b ) { return a + ( b ? b->size() : 0 ); } );
      std::vector<int> out;
      out.reserve( nelements );
      // Concatenate the input vectors to form the output
      for ( auto& intVector : intVectors ) {
        info() << "Concatening vector " << intVector << endmsg;
        if ( intVector ) {
          out.insert( out.end(), intVector->begin(), intVector->end() );
          // intVector->clear(); // should not be possible!!! and does indeed not compile ;-)
        }
      }
      info() << "Storing output vector " << out << " to " << outputLocation() << endmsg;
      return out;
    }
  };

  DECLARE_COMPONENT( PIntVectorsToIntVector )

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

  struct ContextTransformer final : Gaudi::Functional::Transformer<int( const EventContext& ), BaseClass_t> {

    ContextTransformer( const std::string& name, ISvcLocator* svcLoc )
        : Transformer( name, svcLoc, KeyValue{ "OutputLoc", "/Event/SomeOtherInt" } ) {}

    int operator()( const EventContext& ctx ) const override {
      info() << "executing ContextConsumer, got " << ctx << endmsg;
      return 9;
    }
  };

  DECLARE_COMPONENT( ContextTransformer )

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
      return { 12.34, 56.78, 90.12, 34.56, 78.90 };
    }
  };

  DECLARE_COMPONENT( VectorDoubleProducer )

  struct FrExpTransformer final
      : Gaudi::Functional::MultiScalarTransformer<
            FrExpTransformer, std::tuple<std::vector<double>, std::vector<int>>( const std::vector<double>& ),
            BaseClass_t> {
    FrExpTransformer( const std::string& name, ISvcLocator* svcLoc )
        : MultiScalarTransformer( name, svcLoc, KeyValue{ "InputDoubles", { "/Event/MyVectorOfDoubles" } },
                                  { KeyValue{ "OutputFractions", { "/Event/MyVectorOfFractions" } },
                                    KeyValue{ "OutputIntegers", { "/Event/MyVectorOfIntegers" } } } ) {}

    using MultiScalarTransformer::operator();

    std::tuple<double, int> operator()( const double& d ) const {
      int    i;
      double frac = std::frexp( d, &i );
      info() << "Converting " << d << " -> " << frac << ", " << i << endmsg;
      return { frac, i };
    }
  };
  DECLARE_COMPONENT( FrExpTransformer )

  struct OptFrExpTransformer final
      : Gaudi::Functional::MultiScalarTransformer<
            OptFrExpTransformer, std::tuple<std::vector<double>, std::vector<int>>( const std::vector<double>& ),
            BaseClass_t> {
    OptFrExpTransformer( const std::string& name, ISvcLocator* svcLoc )
        : MultiScalarTransformer( name, svcLoc, KeyValue{ "InputDoubles", { "/Event/MyVectorOfDoubles" } },
                                  { KeyValue{ "OutputFractions", { "/Event/OptMyVectorOfFractions" } },
                                    KeyValue{ "OutputIntegers", { "/Event/OptMyVectorOfIntegers" } } } ) {}

    using MultiScalarTransformer::operator();

    std::optional<std::tuple<double, int>> operator()( const double& d ) const {
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
                             { KeyValue{ "InputFractions", { "/Event/MyVectorOfFractions" } },
                               KeyValue{ "InputIntegers", { "/Event/MyVectorOfIntegers" } } },
                             { KeyValue{ "OutputDoubles", { "/Event/MyNewVectorOfDoubles" } } } ) {}

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
                             { KeyValue{ "InputFractions", { "/Event/MyVectorOfFractions" } },
                               KeyValue{ "InputIntegers", { "/Event/MyVectorOfIntegers" } } },
                             { KeyValue{ "OutputDoubles", { "/Event/MyOptVectorOfDoubles" } } } ) {}

    using ScalarTransformer::operator();

    std::optional<double> operator()( const double& frac, const int& i ) const {
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

  struct S : public KeyedObject<int> {
    using KeyedObject::KeyedObject;
    int a;
    using ConstVector = std::vector<S const*>;
    using Selection   = SharedObjectsContainer<S>;
    using Container   = KeyedContainer<S, Containers::HashMap>;
  };

  struct SDataProducer final : Gaudi::Functional::Producer<S::Container(), BaseClass_t> {

    SDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyS" ) ) {}

    S::Container operator()() const override {
      S::Container out{};
      for ( int i = 0; i < j; ++i ) out.insert( new S{} );
      info() << "storing KeyedContainer of size " << out.size() << " into " << outputLocation() << endmsg;
      return out;
    }
    Gaudi::Property<int> j{ this, "j", 5 };
  };

  DECLARE_COMPONENT( SDataProducer )

  struct SRangesToIntVector final
      : public Gaudi::Functional::MergingTransformer<
            std::vector<int>( const Gaudi::Functional::vector_of_const_<Gaudi::Range_<std::vector<S const*>>>& ),
            BaseClass_t> {

    SRangesToIntVector( const std::string& name, ISvcLocator* svcLoc )
        : MergingTransformer( name, svcLoc, { "InputRanges", {} },
                              { "OutputLocation", "/Event/MyConcatenatedIntFromSVector" } ) {}

    std::vector<int> operator()(
        const Gaudi::Functional::vector_of_const_<Gaudi::Range_<std::vector<S const*>>>& SVectors ) const override {
      std::vector<int> out;
      // Concatenate the input vectors to form the output
      for ( const auto& SVector : SVectors ) {
        info() << "Concatening range of size " << SVector.size() << endmsg;
        for ( auto* s : SVector ) { out.push_back( s->a ); }
      }
      info() << "Storing output vector " << out << " to " << outputLocation() << endmsg;
      return out;
    }
  };
  DECLARE_COMPONENT( SRangesToIntVector )

  struct OptionalSRangesMerger final
      : public Gaudi::Functional::MergingTransformer<void( const Gaudi::Functional::vector_of_const_<
                                                           std::optional<Gaudi::NamedRange_<std::vector<S const*>>>>& ),
                                                     BaseClass_t> {

    OptionalSRangesMerger( const std::string& name, ISvcLocator* svcLoc )
        : MergingTransformer( name, svcLoc, { "InputRanges", {} } ) {}

    void
    operator()( const Gaudi::Functional::vector_of_const_<std::optional<Gaudi::NamedRange_<std::vector<S const*>>>>&
                    OptSVectors ) const override {
      // Loop over the optional ranges checking if the opt has a value
      for ( const auto& OptSVector : OptSVectors ) {
        if ( OptSVector.has_value() ) {
          auto SVector = OptSVector.value();
          info() << "Consuming vector of size: " << SVector.size() << endmsg;
        } else {
          info() << "Skipping empty optional range" << endmsg;
        }
      }
    }
  };
  DECLARE_COMPONENT( OptionalSRangesMerger )

  struct IntVectorsMerger final
      : public Gaudi::Functional::MergingTransformer<
            void( const Gaudi::Functional::vector_of_const_<std::vector<int>>& ), BaseClass_t> {

    IntVectorsMerger( const std::string& name, ISvcLocator* svcLoc )
        : MergingTransformer( name, svcLoc, { "InputLocations", {} } ) {}

    void operator()( const Gaudi::Functional::vector_of_const_<std::vector<int>>& intVectors ) const override {
      // Create a vector and pre-allocate enough space for the number of integers we have
      auto nelements = std::accumulate( intVectors.begin(), intVectors.end(), 0,
                                        []( const auto a, const auto b ) { return a + b.size(); } );
      info() << "sum of input sizes: " << nelements << endmsg;
      // Concatenate the input vectors to form the output
      for ( const auto& intVector : intVectors ) { info() << "Consuming vector " << intVector << endmsg; }
    }
  };

  DECLARE_COMPONENT( IntVectorsMerger )

  struct IntVectorsMergingConsumer final
      : public Gaudi::Functional::MergingConsumer<void( Gaudi::Functional::vector_of_const_<std::vector<int>> const& ),
                                                  BaseClass_t> {
    using Base =
        Gaudi::Functional::MergingConsumer<void( Gaudi::Functional::vector_of_const_<std::vector<int>> const& ),
                                           BaseClass_t>;

    IntVectorsMergingConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Base( name, svcLoc, { "InputLocations", {} } ) {}

    void operator()( Gaudi::Functional::vector_of_const_<std::vector<int>> const& intVectors ) const override {
      // Create a vector and pre-allocate enough space for the number of integers we have
      auto nelements = std::accumulate( intVectors.begin(), intVectors.end(), 0,
                                        []( const auto a, const auto b ) { return a + b.size(); } );
      info() << "sum of input sizes: " << nelements << endmsg;
      // Concatenate the input vectors to form the output
      for ( const auto& intVector : intVectors ) { info() << "Consuming vector " << intVector << endmsg; }
    }
  };

  DECLARE_COMPONENT( IntVectorsMergingConsumer )

  struct MyData {
    using ConstVector = std::vector<const MyData*>;
  };
  using MyDataRange = Gaudi::Range_<MyData::ConstVector>;

  struct RangeProducer : Gaudi::Functional::Producer<MyDataRange()> {

    RangeProducer( const std::string& name, ISvcLocator* pSvcLocator )
        : Producer( name, pSvcLocator, KeyValue{ "TrackLocation", "" } ){};

    MyDataRange operator()() const override { return {}; }
  };
  DECLARE_COMPONENT( RangeProducer )

  /** @brief Concatenates a list of input vectors into a single output vector.
   */
  struct TwoDMerger final : public Gaudi::Functional::MergingMultiTransformer<
                                std::tuple<std::vector<int>, std::vector<double>>(
                                    const Gaudi::Functional::vector_of_const_<std::vector<int>>&,
                                    const Gaudi::Functional::vector_of_const_<std::vector<double>>& ),
                                BaseClass_t> {

    TwoDMerger( const std::string& name, ISvcLocator* svcLoc )
        : MergingMultiTransformer{ name,
                                   svcLoc,
                                   { KeyValues{ "InputInts", {} }, KeyValues{ "InputDoubles", {} } },
                                   { KeyValue{ "OutputInts", "/Event/MySummedInts" },
                                     KeyValue{ "OutputDoubles", "/Event/MySummedDoubles" } } } {}

    std::tuple<std::vector<int>, std::vector<double>>
    operator()( const Gaudi::Functional::vector_of_const_<std::vector<int>>&    intVectors,
                const Gaudi::Functional::vector_of_const_<std::vector<double>>& doubleVectors ) const override {
      auto r         = std::tuple{ std::vector<int>{}, std::vector<double>{} };
      auto& [is, ds] = r;
      std::transform( begin( intVectors ), end( intVectors ), std::back_inserter( is ),
                      []( const std::vector<int>& vi ) { return std::accumulate( begin( vi ), end( vi ), 0 ); } );
      always() << " accumulated: " << is << endmsg;
      std::transform( begin( doubleVectors ), end( doubleVectors ), std::back_inserter( ds ),
                      []( const std::vector<double>& vd ) { return std::accumulate( begin( vd ), end( vd ), 0. ); } );
      always() << " accumulated: " << ds << endmsg;
      return r;
    }
  };

  DECLARE_COMPONENT( TwoDMerger )

  struct Foo {
    int i;

    Foo( int i ) : i{ i } {}
    Foo( Foo&& )                 = delete;
    Foo& operator=( Foo&& )      = delete;
    Foo( const Foo& )            = delete;
    Foo& operator=( const Foo& ) = delete;
    ~Foo(){};
  };

  struct ShrdPtrProducer final : Gaudi::Functional::Producer<std::shared_ptr<Foo>(), BaseClass_t> {

    ShrdPtrProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MySharedFoo" ) ) {}

    std::shared_ptr<Foo> operator()() const override {
      auto foo = std::make_shared<Foo>( m_value.value() );
      info() << "executing ShrdPtrProducer, storing shared_ptr<Foo> with payload at " << foo.get() << " and value  "
             << foo->i << " into " << outputLocation() << endmsg;
      return foo;
    }

    Gaudi::Property<int> m_value{ this, "Value", 7, "The integer value to produce." };
  };

  DECLARE_COMPONENT( ShrdPtrProducer )

  struct ShrdPtrConsumer final : Gaudi::Functional::Consumer<void( std::shared_ptr<Foo> const& ), BaseClass_t> {

    ShrdPtrConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MySharedFoo" ) ) {}

    void operator()( const std::shared_ptr<Foo>& foo ) const override {
      info() << "executing ShrdPtrConsumer, got shared_ptr<Foo> with payload at " << foo.get() << " with value  "
             << foo->i << " from " << inputLocation() << endmsg;
    }
  };

  DECLARE_COMPONENT( ShrdPtrConsumer )

  /** @brief transform a vector of vector of int to a vector of int, where the output vector of in is scattered into the
   * TES
   */
  struct IntVectorsToInts final
      : public Gaudi::Functional::SplittingMergingTransformer<
            std::vector<int>( const Gaudi::Functional::vector_of_const_<std::vector<int>>& ), BaseClass_t> {

    Gaudi::Property<std::vector<std::pair<int, int>>> m_mapping{ this, "Mapping", {} };

    IntVectorsToInts( const std::string& name, ISvcLocator* svcLoc )
        : SplittingMergingTransformer( name, svcLoc, { "InputLocations", {} }, { "OutputLocations", {} } ) {}

    std::vector<int>
    operator()( const Gaudi::Functional::vector_of_const_<std::vector<int>>& intVectors ) const override {
      int l = 0;
      for ( const auto& iv : intVectors ) { info() << "loaded " << iv << " from " << inputLocation( l++ ) << endmsg; }
      std::vector<int> out( outputLocationSize(), 0 );
      for ( const auto& [l, r] : m_mapping.value() ) {
        out[l] = std::accumulate( intVectors.at( r ).begin(), intVectors.at( r ).end(), out[l] );
      }
      l = 0;
      for ( const auto& o : out ) { info() << "storing " << o << " in " << outputLocation( l++ ) << endmsg; }
      return out;
    }
  };
  DECLARE_COMPONENT( IntVectorsToInts )

  struct Eventually {
    Gaudi::Algorithm const* parent              = nullptr;
    void ( *action )( Gaudi::Algorithm const* ) = nullptr;
    Eventually( Gaudi::Algorithm const* p, void ( *a )( Gaudi::Algorithm const* ) ) : parent{ p }, action{ a } {}
    Eventually( Eventually const& )            = delete;
    Eventually& operator=( Eventually const& ) = delete;
    Eventually( Eventually&& other )
        : parent{ std::exchange( other.parent, nullptr ) }, action{ std::exchange( other.action, nullptr ) } {}
    Eventually& operator=( Eventually&& other ) {
      parent = std::exchange( other.parent, nullptr );
      action = std::exchange( other.action, nullptr );
      return *this;
    }
    ~Eventually() {
      if ( action ) action( parent );
    }
  };

  struct OpaqueProducer final
      : Gaudi::Functional::Producer<
            Eventually(),
            Gaudi::Functional::Traits::use_<BaseClass_t, Gaudi::Functional::Traits::WriteOpaqueFor<Eventually>>> {

    OpaqueProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/Eventually" ) ) {}

    Eventually operator()() const override {
      always() << "creating Eventually" << endmsg;
      return Eventually{ this, []( Gaudi::Algorithm const* me ) {
                          me->always() << "My Eventually is about to be destroyed" << endmsg;
                        } };
    }
  };

  DECLARE_COMPONENT( OpaqueProducer )

  static_assert( std::ranges::forward_range<Gaudi::Functional::vector_of_const_<void*>> );
  static_assert( std::ranges::forward_range<Gaudi::Functional::vector_of_const_<int>> );
  static_assert( std::ranges::forward_range<Gaudi::Functional::vector_of_const_<std::vector<int>*>> );
  static_assert( std::ranges::forward_range<Gaudi::Functional::vector_of_const_<std::vector<int>>> );
  static_assert( std::same_as<typename Gaudi::Functional::vector_of_const_<std::vector<int>*>::value_type,
                              std::vector<int> const*> );
  static_assert( std::same_as<typename Gaudi::Functional::vector_of_const_<std::vector<int>>::value_type,
                              std::vector<int> const> );
  static_assert(
      std::same_as<typename Gaudi::Functional::vector_of_const_<Gaudi::Range_<std::vector<void*>>>::value_type,
                   Gaudi::Range_<std::vector<void*>> const> );

} // namespace Gaudi::TestSuite
