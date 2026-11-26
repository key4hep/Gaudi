/***********************************************************************************\
* (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/MergingTransformer.h>
#include <Gaudi/Interfaces/IFileSvc.h>
#include <Gaudi/Property.h>
#include <Gaudi/details/BranchWrapper.h>
#include <Gaudi/details/NTupleTypes.h>
#include <GaudiKernel/AnyDataWrapper.h>
#include <GaudiKernel/ConcurrencyFlags.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/System.h>
#include <ROOT/REntry.hxx>
#include <ROOT/RField.hxx>
#include <ROOT/RFieldToken.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleFillStatus.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RVersion.hxx>
#include <TDirectory.h>
#include <TFile.h>
#include <TTree.h>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_set>
#include <vector>

namespace {
  // Extract the name from a path in the TES string by returning the last part after a slash
  auto getNameFromLoc( std::string_view loc ) {
    auto lastSlashPos = loc.find_last_of( '/' );
    return std::string{ lastSlashPos != loc.npos ? loc.substr( lastSlashPos + 1 ) : loc };
  }

  std::mutex& fileMutex( const TFile& file ) {
    static std::mutex                         registryMutex;
    static std::map<const TFile*, std::mutex> mutexes;
    std::scoped_lock                          lock{ registryMutex };
    return mutexes[&file];
  }

  // any std::vector<T> read as std::vector<std::byte>, size() is in bytes then
  std::span<const std::byte> vectorBytes( const void* vector ) {
    const auto& bytes = *static_cast<const std::vector<std::byte>*>( vector );
    return { bytes.data(), bytes.size() };
  }

  // every fill context buffers a whole cluster, so halve the cluster size until they fit in the budget
  // TODO: the 1 MiB is arbitrary, could be a property if someone needs smaller clusters
  ROOT::RNTupleWriteOptions writeOptions( const ROOT::RNTupleModel& model, std::size_t budgetPerContext,
                                          std::uint32_t compression ) {
    auto withCluster = [compression]( std::size_t zipped ) {
      ROOT::RNTupleWriteOptions options;
      options.SetCompression( compression );
      options.SetApproxZippedClusterSize( zipped );
      options.SetMaxUnzippedClusterSize( 10 * zipped );
      return options;
    };
    auto fits = [&]( std::size_t zipped ) {
      return model.EstimateWriteMemoryUsage( withCluster( zipped ) ) <= budgetPerContext;
    };
    const std::size_t minimum = 1 << 20;
    std::size_t       zipped  = ROOT::RNTupleWriteOptions{}.GetApproxZippedClusterSize();
    while ( zipped > minimum && !fits( zipped ) ) zipped /= 2;
    return withCluster( std::max( zipped, minimum ) );
  }

  // either of them can be 0 when the job is not using the Hive scheduler
  std::size_t expectedConcurrency() {
    const std::size_t threads = Gaudi::Concurrency::ConcurrencyFlags::numThreads();
    const std::size_t slots   = Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents();
    if ( threads > 0 && slots > 0 ) return std::min( threads, slots );
    return std::max<std::size_t>( { 1, threads, slots } );
  }
} // namespace

namespace Gaudi::NTuple {

#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 38, 0 )
  using RNTupleFillContext    = ROOT::Experimental::RNTupleFillContext;
  using RNTupleParallelWriter = ROOT::Experimental::RNTupleParallelWriter;
#else
  using RNTupleFillContext    = ROOT::RNTupleFillContext;
  using RNTupleParallelWriter = ROOT::RNTupleParallelWriter;
#endif

  struct FieldSpec {
    std::string name;
    std::string type;
  };

  struct InconsistentEvent : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  struct Payload {
    const void*                address;
    std::type_index            type;
    std::optional<std::size_t> size;
  };

  Payload payloadOf( const DataObject& object ) {
    if ( auto* wrapper = dynamic_cast<const AnyDataWrapperBase*>( &object ) ) {
      auto data = wrapper->payload();
      return { data, data.type(), wrapper->size() };
    }
    return { &object, typeid( object ), std::nullopt };
  }

  // Layouts: how the inputs of one event become entries

  class EventLayout {
  public:
    struct Options {};

    struct Properties {
      explicit Properties( Gaudi::Algorithm* ) {}
      Options options() const { return {}; }
    };

    EventLayout( std::vector<FieldSpec> inputs, const Options& = {} ) : m_fields{ std::move( inputs ) } {}

    const std::vector<FieldSpec>& fields() const { return m_fields; }

    template <typename Sink>
    void write( std::span<const Payload> payloads, const Sink& sink ) const {
      std::vector<const void*> row( payloads.size() );
      std::ranges::transform( payloads, row.begin(), &Payload::address );
      sink.fill( 1, [&]( std::size_t ) { return std::span<const void* const>{ row }; } );
    }

  private:
    std::vector<FieldSpec> m_fields;
  };

  /// One entry per vector element. Inputs are std::vector<T> of equal length, except the event fields
  /// which go unchanged into every entry of the event.
  class ElementLayout {
  public:
    struct Options {
      std::vector<std::string> eventFields;
    };

    struct Properties {
      explicit Properties( Gaudi::Algorithm* owner )
          : eventFields{ owner, "EventFields", {}, "Fields repeated as a whole in every entry of the event" } {}
      Options options() const { return { eventFields.value() }; }

      Gaudi::Property<std::vector<std::string>> eventFields;
    };

    ElementLayout( std::vector<FieldSpec> inputs, const Options& options ) : m_fields{ std::move( inputs ) } {
      const std::set<std::string> eventFields( options.eventFields.begin(), options.eventFields.end() );
      for ( const auto& name : eventFields ) {
        if ( std::ranges::none_of( m_fields, [&]( const auto& field ) { return field.name == name; } ) ) {
          throw std::invalid_argument(
              std::format( "'{}' is listed in EventFields but is not one of the fields", name ) );
        }
      }
      if ( eventFields.size() == m_fields.size() ) {
        throw std::invalid_argument( "all fields are event fields, nothing to write per element" );
      }
      for ( auto& field : m_fields ) {
        if ( eventFields.contains( field.name ) ) {
          m_elementSizes.push_back( 0 );
        } else {
          auto element = Gaudi::details::vectorElementOf( field.type );
          field.type   = std::move( element.type );
          m_elementSizes.push_back( element.size );
        }
      }
    }

    const std::vector<FieldSpec>& fields() const { return m_fields; }

    template <typename Sink>
    void write( std::span<const Payload> payloads, const Sink& sink ) const {
      std::vector<const std::byte*> elements( payloads.size(), nullptr );
      std::optional<std::size_t>    length;
      std::size_t                   reference = 0;
      for ( std::size_t f = 0; f < payloads.size(); ++f ) {
        if ( isEventField( f ) ) continue;
        auto bytes = vectorBytes( payloads[f].address );
        auto count = payloads[f].size;
        if ( !count || bytes.size() != *count * m_elementSizes[f] ) {
          throw InconsistentEvent(
              std::format( "field '{}' does not hold a consistent std::vector", m_fields[f].name ) );
        }
        if ( !length ) {
          length    = count;
          reference = f;
        } else if ( *count != *length ) {
          throw InconsistentEvent( std::format( "field '{}' has {} elements but field '{}' has {}", m_fields[f].name,
                                                *count, m_fields[reference].name, *length ) );
        }
        elements[f] = bytes.data();
      }

      std::vector<const void*> row( payloads.size() );
      for ( std::size_t f = 0; f < payloads.size(); ++f ) {
        if ( isEventField( f ) ) row[f] = payloads[f].address;
      }
      sink.fill( *length, [&]( std::size_t i ) {
        for ( std::size_t f = 0; f < payloads.size(); ++f ) {
          if ( !isEventField( f ) ) row[f] = elements[f] + i * m_elementSizes[f];
        }
        return std::span<const void* const>{ row };
      } );
    }

  private:
    bool isEventField( std::size_t field ) const { return m_elementSizes[field] == 0; }

    std::vector<FieldSpec>   m_fields;
    std::vector<std::size_t> m_elementSizes; // 0 for an event field
  };

  // Sinks: where the entries are written

  /// TTree output, fills are serialized with the file mutex
  class TTreeSink {
  public:
    struct Options {};

    struct Properties {
      explicit Properties( Gaudi::Algorithm* ) {}
      Options options() const { return {}; }
    };

    TTreeSink( TFile& file, const std::string& name, std::span<const FieldSpec> fields, const Options& = {} )
        : m_fileMutex{ fileMutex( file ) } {
      std::scoped_lock lock{ m_fileMutex };
      m_tree = new TTree( name.c_str(), "Gaudi::NTuple generic writer", 99, &file ); // the file owns it
      m_branches.reserve( fields.size() );
      for ( const auto& field : fields ) m_branches.emplace_back( m_tree, field.type, field.name, field.name, name );
    }

    TTreeSink( const TTreeSink& )            = delete;
    TTreeSink& operator=( const TTreeSink& ) = delete;

    template <typename AddressesOf>
    void fill( std::size_t nEntries, AddressesOf&& addresses ) const {
      std::scoped_lock lock{ m_fileMutex };
      try {
        for ( std::size_t i = 0; i < nEntries; ++i ) {
          auto values = addresses( i );
          for ( std::size_t f = 0; f < values.size(); ++f ) m_branches[f].setDataPtr( values[f] );
          if ( m_tree->Fill() < 0 ) {
            throw std::runtime_error( std::format( "failed to fill TTree {}", m_tree->GetName() ) );
          }
        }
      } catch ( ... ) {
        m_incomplete = true;
        throw;
      }
    }

    void close() {
      std::scoped_lock     lock{ m_fileMutex };
      TDirectory::TContext context{ m_tree->GetDirectory() };
      m_tree->ResetBranchAddresses(); // they still point into the last event
      // kOverwrite since AutoSave may have written a cycle already
      if ( m_tree->Write( nullptr, TObject::kOverwrite ) <= 0 ) {
        throw std::runtime_error( std::format( "could not write TTree {}", m_tree->GetName() ) );
      }
      if ( m_incomplete ) throw std::runtime_error( "a fill failed, the TTree is incomplete" );
    }

    std::string summary() const {
      return std::format( "TTree {} with {} branches", m_tree->GetName(), m_branches.size() );
    }

    // Getters
    size_t getBranchWrappersSize() const { return m_branches.size(); }

    std::unordered_set<std::string> getBranchesClassNames() const {
      std::unordered_set<std::string> classNames;
      for ( auto& wrapper : m_branches ) { classNames.insert( wrapper.getClassName() ); }
      return classNames;
    }

  private:
    std::mutex&                                        m_fileMutex;
    TTree*                                             m_tree = nullptr;
    mutable std::vector<Gaudi::details::BranchWrapper> m_branches;
    mutable bool                                       m_incomplete = false;
  };

  /// RNTuple output, filled in parallel through a pool of fill contexts.
  class RNTupleSink {
  public:
    struct Options {
      std::size_t memoryBudget = 512 << 20;
      std::size_t concurrency  = 1;
    };

    struct Properties {
      explicit Properties( Gaudi::Algorithm* owner )
          : memoryBudget{ owner, "MemoryBudget", 512, "Approximate memory (MiB) for buffered output" } {}
      Options options() const { return { memoryBudget.value() << 20, expectedConcurrency() }; }

      Gaudi::Property<std::size_t> memoryBudget;
    };

    RNTupleSink( TFile& file, const std::string& name, std::span<const FieldSpec> fields, const Options& options )
        : m_fileMutex{ fileMutex( file ) } {
      auto model = ROOT::RNTupleModel::CreateBare();
      for ( const auto& field : fields ) {
        auto created = ROOT::RFieldBase::Create( field.name, field.type );
        if ( !created ) {
          throw std::invalid_argument( std::format( "cannot create field '{}' of type '{}': {}", field.name, field.type,
                                                    created.GetError()->GetReport() ) );
        }
        model->AddField( created.Unwrap() );
        m_fieldNames.push_back( field.name );
      }
      auto writeOpts = writeOptions( *model, options.memoryBudget / std::max<std::size_t>( 1, options.concurrency ),
                                     file.GetCompressionSettings() );
      m_clusterSize  = writeOpts.GetApproxZippedClusterSize();
      std::scoped_lock lock{ m_fileMutex };
      m_writer = RNTupleParallelWriter::Append( std::move( model ), name, file, writeOpts );
    }

    RNTupleSink( const RNTupleSink& )            = delete;
    RNTupleSink& operator=( const RNTupleSink& ) = delete;

    template <typename AddressesOf>
    void fill( std::size_t nEntries, AddressesOf&& addresses ) const {
      auto  lease   = acquire();
      auto& context = *lease.context;
      for ( std::size_t i = 0; i < nEntries; ++i ) {
        auto values = addresses( i );
        for ( std::size_t f = 0; f < values.size(); ++f ) {
          context.entry->BindRawPtr( context.tokens[f], const_cast<void*>( values[f] ) );
        }
        ROOT::RNTupleFillStatus status;
        context.fill->FillNoFlush( *context.entry, status );
        if ( status.ShouldFlushCluster() ) {
          context.fill->FlushColumns(); // compression outside of the lock
          std::scoped_lock lock{ m_fileMutex };
          context.fill->FlushCluster();
        }
      }
      lease.completed = true;
    }

    // commit explicitly, if we leave it to the destructors I/O errors are only printed
    void close() {
      std::scoped_lock lock{ m_fileMutex };
      for ( auto& context : m_idle ) context->fill->FlushCluster();
      m_idle.clear();
      m_writer->CommitDataset();
      m_writer.reset();
      if ( m_incomplete ) throw std::runtime_error( "RNTuple is incomplete, a fill failed during the event loop" );
    }

    std::string summary() const {
      return std::format( "RNTuple with {} fields, {} fill contexts, clusters of {:.1f} MiB", m_fieldNames.size(),
                          m_created, m_clusterSize / double( 1 << 20 ) );
    }

  private:
    struct Context {
      std::shared_ptr<RNTupleFillContext> fill;
      std::unique_ptr<ROOT::REntry>       entry;
      std::vector<ROOT::RFieldToken>      tokens;
    };

    struct Lease {
      const RNTupleSink&       sink;
      std::unique_ptr<Context> context;
      bool                     completed = false;
      ~Lease() {
        if ( completed ) {
          std::scoped_lock lock{ sink.m_poolMutex };
          sink.m_idle.push_back( std::move( context ) ); // can't throw, reserved in acquire()
          return;
        }
        // destroying it flushes to the file
        sink.m_incomplete = true;
        std::scoped_lock lock{ sink.m_fileMutex };
        context.reset();
      }
    };

    Lease acquire() const {
      {
        std::scoped_lock lock{ m_poolMutex };
        if ( !m_idle.empty() ) {
          auto context = std::move( m_idle.back() );
          m_idle.pop_back();
          return { *this, std::move( context ) };
        }
        m_idle.reserve( ++m_created );
      }
      auto context   = std::make_unique<Context>();
      context->fill  = m_writer->CreateFillContext();
      context->entry = context->fill->GetModel().CreateBareEntry();
      for ( const auto& name : m_fieldNames ) context->tokens.push_back( context->entry->GetToken( name ) );
      return { *this, std::move( context ) };
    }

    std::mutex&                                   m_fileMutex;
    std::vector<std::string>                      m_fieldNames;
    std::size_t                                   m_clusterSize = 0;
    std::unique_ptr<RNTupleParallelWriter>        m_writer;
    mutable std::mutex                            m_poolMutex;
    mutable std::vector<std::unique_ptr<Context>> m_idle;
    mutable std::size_t                           m_created = 0;
    mutable std::atomic<bool>                     m_incomplete{ false };
  };

  /**
   * @class GenericWriter
   * @brief Writes data of any type from N locations in the event store to a TTree or RNTuple.
   * Layout decides how the inputs of an event become entries, Sink writes them to the FileSvc file OutputFile.
   * Writers of the same file share its mutex, other components should not write to it during event loop.
   **/
  template <typename Layout, typename Sink>
  class GenericWriter final
      : public Gaudi::Functional::MergingConsumer<void( const Gaudi::Functional::vector_of_const_<DataObject>& )> {
    using base_t = Gaudi::Functional::MergingConsumer<void( const Gaudi::Functional::vector_of_const_<DataObject>& )>;

  public:
    GenericWriter( const std::string& name, ISvcLocator* svcLoc )
        : base_t( name, svcLoc, base_t::KeyValues{ "Inputs", {} } ) {}

    StatusCode initialize() override {
      return base_t::initialize().andThen( [this]() -> StatusCode {
        try {
          auto inputs = declaredInputs();
          m_types.clear();
          for ( const auto& input : inputs ) m_types.push_back( Gaudi::details::typeIndexOf( input.type ) );
          m_layout.emplace( std::move( inputs ), m_layoutProperties.options() );

          auto fileSvc = service<Gaudi::Interfaces::IFileSvc>( "FileSvc" );
          if ( !fileSvc ) throw std::runtime_error( "Failed to retrieve FileSvc." );
          m_file = fileSvc->getFile( m_fileId );
          if ( !m_file ) throw std::runtime_error( std::format( "Failed to retrieve TFile {}.", m_fileId.value() ) );

          const auto ntupleName = m_ntupleName.empty() ? name() : m_ntupleName.value();
          m_sink.emplace( *m_file, ntupleName, m_layout->fields(), m_sinkProperties.options() );
          info() << "Writing " << m_sink->summary() << endmsg;
        } catch ( const std::exception& e ) {
          error() << e.what() << endmsg;
          return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
      } );
    }

    void operator()( const Gaudi::Functional::vector_of_const_<DataObject>& inputs ) const override {
      std::vector<Payload> payloads;
      payloads.reserve( inputs.size() );
      for ( std::size_t i = 0; i < inputs.size(); ++i ) {
        const auto& payload = payloads.emplace_back( payloadOf( inputs[i] ) );
        if ( payload.type != m_types[i] ) {
          throw GaudiException( std::format( "Type mismatch for {}: got {}, but FieldTypes says {}.",
                                             inputLocation( i ), System::typeinfoName( payload.type.name() ),
                                             m_fieldTypes[i] ),
                                name(), StatusCode::FAILURE );
        }
      }
      try {
        m_layout->write( std::span<const Payload>{ payloads }, *m_sink );
      } catch ( const InconsistentEvent& e ) {
        throw GaudiException( std::format( "event skipped: {}", e.what() ), name(), StatusCode::RECOVERABLE );
      }
    }

    StatusCode finalize() override {
      StatusCode status = StatusCode::SUCCESS;
      if ( m_sink ) {
        try {
          m_sink->close();
          info() << "Wrote " << m_sink->summary() << endmsg;
        } catch ( const std::exception& e ) {
          error() << e.what() << endmsg;
          status = StatusCode::FAILURE;
        }
        m_sink.reset();
      }
      m_layout.reset();
      m_file.reset();
      return base_t::finalize().andThen( [&] { return status; } );
    }

  private:
    std::vector<FieldSpec> declaredInputs() const {
      const std::size_t n = inputLocationSize();
      if ( n == 0 ) throw std::invalid_argument( "No inputs specified. Please set the Inputs property." );
      if ( m_fieldTypes.size() != n ) {
        throw std::invalid_argument(
            std::format( "FieldTypes has {} entries but there are {} inputs", m_fieldTypes.size(), n ) );
      }
      if ( !m_fieldNames.empty() && m_fieldNames.size() != n ) {
        throw std::invalid_argument(
            std::format( "FieldNames has {} entries but there are {} inputs", m_fieldNames.size(), n ) );
      }
      std::vector<FieldSpec> inputs;
      std::set<std::string>  names;
      for ( std::size_t i = 0; i < n; ++i ) {
        auto& input = inputs.emplace_back(
            m_fieldNames.empty() ? getNameFromLoc( inputLocation( i ) ) : m_fieldNames[i], m_fieldTypes[i] );
        if ( input.name.empty() )
          throw std::invalid_argument( std::format( "empty field name for input {}", inputLocation( i ) ) );
        if ( !names.insert( input.name ).second ) {
          throw std::invalid_argument( std::format( "field name '{}' is used more than once", input.name ) );
        }
      }
      return inputs;
    }

    Gaudi::Property<std::string> m_fileId{ this, "OutputFile", "NTuple", "Identifier of the FileSvc file to write to" };
    Gaudi::Property<std::string> m_ntupleName{ this, "NTupleName", "",
                                               "Name of the ntuple (defaults to the algorithm name)" };
    Gaudi::Property<std::vector<std::string>> m_fieldNames{
        this, "FieldNames", {}, "Field names aligned with 'Inputs' (defaults to the last segment of each location)" };
    Gaudi::Property<std::vector<std::string>> m_fieldTypes{
        this, "FieldTypes", {}, "C++ type names of the objects at 'Inputs', aligned with them" };

    typename Layout::Properties  m_layoutProperties{ this };
    typename Sink::Properties    m_sinkProperties{ this };
    std::vector<std::type_index> m_types;
    std::optional<Layout>        m_layout;
    std::shared_ptr<TFile>       m_file;
    std::optional<Sink>          m_sink;
  };

  using GenericTTreeWriter          = GenericWriter<EventLayout, TTreeSink>;
  using GenericRNTupleWriter        = GenericWriter<EventLayout, RNTupleSink>;
  using GenericTTreeElementWriter   = GenericWriter<ElementLayout, TTreeSink>;
  using GenericRNTupleElementWriter = GenericWriter<ElementLayout, RNTupleSink>;

} // namespace Gaudi::NTuple

DECLARE_COMPONENT_WITH_ID( Gaudi::NTuple::GenericTTreeWriter, "Gaudi::NTuple::GenericTTreeWriter" )
DECLARE_COMPONENT_WITH_ID( Gaudi::NTuple::GenericRNTupleWriter, "Gaudi::NTuple::GenericRNTupleWriter" )
DECLARE_COMPONENT_WITH_ID( Gaudi::NTuple::GenericTTreeElementWriter, "Gaudi::NTuple::GenericTTreeElementWriter" )
DECLARE_COMPONENT_WITH_ID( Gaudi::NTuple::GenericRNTupleElementWriter, "Gaudi::NTuple::GenericRNTupleElementWriter" )

// former name of the TTree writer
DECLARE_COMPONENT_WITH_ID( Gaudi::NTuple::GenericTTreeWriter, "Gaudi::NTuple::GenericWriter" )

#ifdef UNIT_TESTS

#  define BOOST_TEST_MODULE test_GenericNTupleWriter
#  include <ROOT/RNTuple.hxx>
#  include <ROOT/RNTupleReader.hxx>
#  include <TMemFile.h>
#  include <boost/test/unit_test.hpp>
#  include <tuple>
#  include <type_traits>

/**
 * @class MockISvcLocator
 * @brief Mock implementation of ISvcLocator interface for unit testing.
 */
class MockISvcLocator : public ISvcLocator {
public:
  virtual const std::list<IService*>& getServices() const override {
    static std::list<IService*> dummyServices;
    return dummyServices;
  }

  virtual bool existsService( std::string_view ) const override { return false; }

  virtual SmartIF<IService>& service( const Gaudi::Utils::TypeNameString&, const bool ) override {
    static SmartIF<IService> dummyService;
    return dummyService;
  }

  virtual unsigned long addRef() const override { return 1; }

  virtual unsigned long release() const override { return 1; }

  virtual StatusCode queryInterface( const InterfaceID&, void** ) override { return StatusCode::FAILURE; }

  virtual std::vector<std::string> getInterfaceNames() const override { return {}; }

  virtual void const* i_cast( const InterfaceID& ) const override { return nullptr; }

  virtual unsigned long refCount() const override { return 1; }

  virtual unsigned long decRef() const override { return 1; }
};

// Utility function tests
BOOST_AUTO_TEST_CASE( testGetNameFromLoc ) {
  // Test name extraction from location strings
  BOOST_CHECK_EQUAL( getNameFromLoc( "/Event/MyAlg/MyData" ), "MyData" );
  BOOST_CHECK_EQUAL( getNameFromLoc( "MyAlg/MyData" ), "MyData" );
  BOOST_CHECK_EQUAL( getNameFromLoc( "MyData" ), "MyData" );
  BOOST_CHECK_EQUAL( getNameFromLoc( "" ), "" );
}

using Writers = std::tuple<Gaudi::NTuple::GenericTTreeWriter, Gaudi::NTuple::GenericRNTupleWriter,
                           Gaudi::NTuple::GenericTTreeElementWriter, Gaudi::NTuple::GenericRNTupleElementWriter>;

// Test instantiation of the GenericWriter
BOOST_AUTO_TEST_CASE_TEMPLATE( testInit, Writer, Writers ) {
  MockISvcLocator mockLocator;
  Writer          writer( "test_writer", &mockLocator );

  // Verify we can instanciate a writer
  BOOST_CHECK_EQUAL( writer.name(), "test_writer" );
}

// Test that initialization fails when no inputs are provided
BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateBranches_EmptyInputs, Writer, Writers ) {
  MockISvcLocator mockLocator;
  Writer          writer( "test_writer", &mockLocator );

  // Expect a failure when no inputs are provided
  BOOST_CHECK_EQUAL( writer.initialize(), StatusCode::FAILURE );
}

using Sinks = std::tuple<Gaudi::NTuple::TTreeSink, Gaudi::NTuple::RNTupleSink>;

// Verify that the sink made one branch per field
template <typename Sink>
void checkBranches( Sink& sink, TFile& file, const std::vector<Gaudi::NTuple::FieldSpec>& fields ) {
  if constexpr ( std::is_same_v<Sink, Gaudi::NTuple::TTreeSink> ) {
    std::unordered_set<std::string> expectedTypes;
    for ( const auto& field : fields ) expectedTypes.insert( field.type );

    // Verify that the branch wrappers' class names match the expected types
    BOOST_CHECK_EQUAL( sink.getBranchWrappersSize(), expectedTypes.size() );
    BOOST_CHECK( expectedTypes == sink.getBranchesClassNames() );
  } else {
    // the RNTuple is only in the file once the sink is closed
    sink.close();
    auto                     reader = ROOT::RNTupleReader::Open( *file.Get<ROOT::RNTuple>( "testNTuple" ) );
    std::vector<std::string> names, expectedNames;
    for ( auto* field : reader->GetModel().GetConstFieldZero().GetConstSubfields() ) {
      names.push_back( field->GetFieldName() );
    }
    for ( const auto& field : fields ) expectedNames.push_back( field.name );
    BOOST_CHECK( names == expectedNames );
  }
}

// Test branch creation with an invalid type
BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateBranches_InvalidType, Sink, Sinks ) {
  TMemFile                              file( "test.root", "RECREATE" );
  std::vector<Gaudi::NTuple::FieldSpec> fields{ { "loc", "InvalidType" } };

  // Expect an exception when an invalid type is provided
  if constexpr ( std::is_same_v<Sink, Gaudi::NTuple::TTreeSink> ) {
    BOOST_CHECK_EXCEPTION( Sink( file, "testNTuple", fields, {} ), GaudiException, []( const GaudiException& e ) {
      return e.message() == "Cannot create branch loc for unknown class: InvalidType. Provide a dictionary please.";
    } );
  } else {
    // the message comes from ROOT
    BOOST_CHECK_THROW( Sink( file, "testNTuple", fields, {} ), std::invalid_argument );
  }
}

// Test branch creation for fundamental types
BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateBranches_BasicTypes, Sink, Sinks ) {
  TMemFile                              file( "test.root", "RECREATE" );
  std::vector<Gaudi::NTuple::FieldSpec> fields{ { "loc1", "int" }, { "loc2", "double" }, { "loc3", "std::string" } };
  Sink                                  sink( file, "testNTuple", fields, {} );

  checkBranches( sink, file, fields );
}

// Test branch creation for ROOT-known non-fundamental types
BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateBranches_ROOTKnownTypes, Sink, Sinks ) {
  TMemFile                              file( "test.root", "RECREATE" );
  std::vector<Gaudi::NTuple::FieldSpec> fields{ { "vectorDoubleLoc", "std::vector<double>" },
                                                { "xyzVectorLoc", "ROOT::Math::XYZVector" } };
  // TH1D can't be stored natively in an RNTuple
  if constexpr ( std::is_same_v<Sink, Gaudi::NTuple::TTreeSink> ) fields.push_back( { "hist1DLoc", "TH1D" } );
  Sink sink( file, "testNTuple", fields, {} );

  checkBranches( sink, file, fields );
}

#endif
