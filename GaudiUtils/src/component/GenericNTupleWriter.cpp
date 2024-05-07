/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/details/BranchWrapper.h>
#include <TFile.h>
#include <TTree.h>
#include <fmt/format.h>
#include <functional>
#include <gsl/pointers>
#include <mutex>
#include <string>
#include <vector>

namespace {
  // Extract the type name from a string, handles types marked as UNKNOWN_CLASS
  auto getTypeName( const std::string_view& dependency ) {
    auto unknownClassPos = dependency.find( "UNKNOWN_CLASS:" );
    return ( unknownClassPos != std::string::npos )
               ? std::string( dependency.substr( unknownClassPos + std::string( "UNKNOWN_CLASS:" ).length() ) )
               : std::string( dependency );
  }

  // Extract the name from a path in the TES string by returning the last part after a slash
  auto getNameFromLoc( const std::string_view& loc ) {
    auto lastSlashPos = loc.find_last_of( '/' );
    return ( lastSlashPos != std::string::npos ) ? std::string( loc.substr( lastSlashPos + 1 ) ) : std::string( loc );
  }
} // namespace

namespace Gaudi::NTuple {
  /**
   * @class GenericWriter
   * @brief A Gaudi algorithm for writing data of any type from N locations in the event store to a TTree.
   * This class allows users to write data from various locations within the event store into a ROOT TTree structure
   * dynamically creating branches leveraging ROOT's dynamic typing and I/O capabilities to support a range of data
   * types.
   **/

  class GenericWriter : public Gaudi::Algorithm {
  public:
    GenericWriter( const std::string& n, ISvcLocator* l )
        : Algorithm( n, l )
        , m_filename( this, "TreeFilename", "generic_ntuple_writer_tree.root",
                      "Filename where the NTuple writer alg writes the ttree." ) {}

    // Initialize the algorithm, set up the ROOT file and a TTree branch for each input location
    StatusCode initialize() override {
      return Gaudi::Algorithm::initialize().andThen( [this]() {
        const auto& extraInputs = extraInputDeps();
        if ( extraInputs.empty() ) {
          throw GaudiException(
              "No extra inputs locations specified. Please define extra inputs for the NTuple writer.", name(),
              StatusCode::FAILURE );
        }

        auto tempFile = std::make_unique<TFile>( m_filename.value().c_str(), "RECREATE" );
        if ( !tempFile || tempFile->IsZombie() ) {
          throw GaudiException(
              fmt::format( "Failed to open file '{}'. Check file path and permissions.", m_filename.value() ), name(),
              StatusCode::FAILURE );
        }

        m_tree = std::make_unique<TTree>( "GenericWriterTree", "Tree of GenericWriter Algorithm" ).release();
        if ( !m_tree ) {
          throw GaudiException( "Failed to create TTree. Ensure sufficient resources and permissions.", name(),
                                StatusCode::FAILURE );
        }

        createBranches( extraInputs );

        m_eventSvc = eventSvc();
        if ( !m_eventSvc ) {
          throw GaudiException(
              "Failed to retrieve the event service. Ensure the Event Service is properly configured and available.",
              name(), StatusCode::FAILURE );
        }

        m_file = std::move( tempFile );

        return StatusCode::SUCCESS;
      } );
    }

    // Execute the algorithm for each event, retrieving data from the event store and writing it to the TTree
    StatusCode execute( const EventContext& ) const override {
      std::lock_guard<std::mutex> lock( m_mtx );

      DataObject* pObj = nullptr;
      for ( auto& wrapper : m_branchWrappers ) {
        auto sc = m_eventSvc->retrieveObject( wrapper.getLocation(), pObj );

        if ( sc.isFailure() ) {
          throw GaudiException(
              fmt::format(
                  "Failed to retrieve object for location '{}'. Ensure the location is correct and the object exists.",
                  wrapper.getLocation() ),
              name(), StatusCode::FAILURE );
        }

        wrapper.setBranchData( pObj );
      }

      m_tree->Fill();

      return StatusCode::SUCCESS;
    }

    // Finalize the algorithm by writing the TTree to the file, resetting the branches and closing the file
    StatusCode finalize() override {
      m_file->cd();
      if ( m_tree->Write() <= 0 ) {
        throw GaudiException( "Failed to write TTree to ROOT file.", name(), StatusCode::FAILURE );
      }
      m_tree = nullptr;
      info() << "TTree written to ROOT file. File closed." << endmsg;

      return Gaudi::Algorithm::finalize();
    }

    // Create branches for each input location specified in the configuration
    void createBranches( const DataObjIDColl& extraInputs ) {
      m_branchWrappers.reserve( m_branchWrappers.size() + extraInputs.size() );
      for ( const auto& dep : extraInputs ) {
        auto typeName   = getTypeName( dep.className() );
        auto branchName = getNameFromLoc( dep.key() );
        m_branchWrappers.emplace_back( m_tree, typeName, branchName, dep.key(), *this );
      }
    }

    // Getters, Setters
    size_t getBranchWrappersSize() const { return m_branchWrappers.size(); }

    std::unordered_set<std::string> getBranchesClassNames() const {
      std::unordered_set<std::string> classNames;
      for ( auto& wrapper : m_branchWrappers ) { classNames.insert( wrapper.getClassName() ); }
      return classNames;
    }

    void setTree( TTree* tree ) { m_tree = tree; }

  private:
    Gaudi::Property<std::string> m_filename;           // Property to hold the filename where the TTree will be saved
    std::unique_ptr<TFile>       m_file     = nullptr; // Smart pointer to the ROOT TFile object
    TTree*                       m_tree     = nullptr; // Pointer to the ROOT TTree object
    IDataProviderSvc*            m_eventSvc = nullptr; // Pointer to the event service interface for data retrieval
    mutable std::mutex           m_mtx;                // Mutex for thread-safe operations on the GenericWriter object
    mutable std::vector<Gaudi::details::BranchWrapper> m_branchWrappers{}; // Container for all BranchWrapper instances
  };

  DECLARE_COMPONENT( GenericWriter )
} // namespace Gaudi::NTuple

#ifdef UNIT_TESTS

#  define BOOST_TEST_MODULE test_GenericNTupleWriter
#  include <boost/test/unit_test.hpp>

/**
 * @class MockISvcLocator
 * @brief Mock implementation of ISvcLocator interface for unit testing.
 */
class MockISvcLocator : public ISvcLocator {
public:
  virtual StatusCode getService( const Gaudi::Utils::TypeNameString&, const InterfaceID&, IInterface*& ) override {
    return StatusCode::SUCCESS;
  }

  virtual StatusCode getService( const Gaudi::Utils::TypeNameString&, IService*&, const bool ) override {
    return StatusCode::SUCCESS;
  }

  virtual const std::list<IService*>& getServices() const override {
    static std::list<IService*> dummyServices;
    return dummyServices;
  }

  virtual bool existsService( std::string_view ) const override { return false; }

  virtual SmartIF<IService>& service( const Gaudi::Utils::TypeNameString&, const bool ) override {
    static SmartIF<IService> dummyService;
    return dummyService;
  }

  virtual unsigned long addRef() override { return 0; }

  virtual unsigned long release() override { return 0; }

  virtual StatusCode queryInterface( const InterfaceID&, void** ) override { return StatusCode::SUCCESS; }

  virtual std::vector<std::string> getInterfaceNames() const override { return {}; }

  virtual void* i_cast( const InterfaceID& ) const override { return nullptr; }

  virtual unsigned long refCount() const override { return 1; }
};

// Utility function tests
BOOST_AUTO_TEST_CASE( testGetTypeName ) {
  // Test type name extraction from various formats
  BOOST_CHECK_EQUAL( getTypeName( "MyClass" ), "MyClass" );
  BOOST_CHECK_EQUAL( getTypeName( "std::vector<double>" ), "std::vector<double>" );
  BOOST_CHECK_EQUAL( getTypeName( "UNKNOWN_CLASS:MyCustomClass" ), "MyCustomClass" );
}

BOOST_AUTO_TEST_CASE( testGetNameFromLoc ) {
  // Test name extraction from location strings
  BOOST_CHECK_EQUAL( getNameFromLoc( "/Event/MyAlg/MyData" ), "MyData" );
  BOOST_CHECK_EQUAL( getNameFromLoc( "MyAlg/MyData" ), "MyData" );
  BOOST_CHECK_EQUAL( getNameFromLoc( "MyData" ), "MyData" );
  BOOST_CHECK_EQUAL( getNameFromLoc( "" ), "" );
}

// Test instantiation of the GenericWriter
BOOST_AUTO_TEST_CASE( testInit ) {
  MockISvcLocator              mockLocator;
  Gaudi::NTuple::GenericWriter writer( "test_writer", &mockLocator );

  // Verify we can instanciate a writer
  BOOST_CHECK_EQUAL( writer.name(), "test_writer" );
}

// Test branch creation with empty dependencies
BOOST_AUTO_TEST_CASE( testCreateBranches_EmptyDeps ) {
  MockISvcLocator              mockLocator;
  Gaudi::NTuple::GenericWriter writer( "test_writer", &mockLocator );

  // Expect an exception when no dependencies are provided
  BOOST_CHECK_EXCEPTION( writer.initialize().ignore(), GaudiException, []( const GaudiException& e ) {
    return e.message() == "No extra inputs locations specified. Please define extra inputs for the NTuple writer.";
  } );
}

// Test branch creation with an invalid type
BOOST_AUTO_TEST_CASE( testCreateBranches_InvalidType ) {
  MockISvcLocator              mockLocator;
  auto                         tree = std::make_unique<TTree>( "testTree", "test tree" );
  Gaudi::NTuple::GenericWriter writer( "test_writer", &mockLocator );
  writer.setTree( tree.get() );

  DataObjIDColl invalidDeps{ { "InvalidType", "loc" } };

  // Expect an exception when an invalid type is provided
  BOOST_CHECK_EXCEPTION( writer.createBranches( invalidDeps ), GaudiException, []( const GaudiException& e ) {
    return e.message() == "Cannot create branch loc for unknown class: InvalidType. Provide a dictionary please.";
  } );
}

// Test branch creation for fundamental types
BOOST_AUTO_TEST_CASE( testCreateBranches_BasicTypes ) {
  MockISvcLocator              mockLocator;
  auto                         tree = std::make_unique<TTree>( "testTree", "test tree" );
  Gaudi::NTuple::GenericWriter writer( "test_writer", &mockLocator );
  writer.setTree( tree.get() );

  DataObjIDColl                   dependencies{ { "int", "loc1" }, { "double", "loc2" }, { "std::string", "loc3" } };
  std::unordered_set<std::string> expectedTypes{ "int", "double", "std::string" };
  writer.createBranches( dependencies );

  // Verify that the branch wrappers' class names match the expected types
  BOOST_CHECK_EQUAL( writer.getBranchWrappersSize(), expectedTypes.size() );
  BOOST_CHECK( expectedTypes == writer.getBranchesClassNames() );
}

// Test branch creation for ROOT-known non-fundamental types
BOOST_AUTO_TEST_CASE( testCreateBranches_ROOTKnownTypes ) {
  MockISvcLocator              mockLocator;
  auto                         tree = std::make_unique<TTree>( "testTree", "test tree" );
  Gaudi::NTuple::GenericWriter writer( "test_writer", &mockLocator );
  writer.setTree( tree.get() );

  DataObjIDColl                   dependencies{ { "std::vector<double>", "vectorDoubleLoc" }, { "TH1D", "hist1DLoc" } };
  std::unordered_set<std::string> expectedTypes{ "std::vector<double>", "TH1D" };
  writer.createBranches( dependencies );

  // Verify that the branch wrappers' class names match the expected types
  BOOST_CHECK_EQUAL( writer.getBranchWrappersSize(), expectedTypes.size() );
  BOOST_CHECK( expectedTypes == writer.getBranchesClassNames() );
}

// Test for GaudiException when the file cannot be opened
BOOST_AUTO_TEST_CASE( testFileOpenException ) {
  MockISvcLocator              mockLocator;
  Gaudi::NTuple::GenericWriter writer( "test_writer", &mockLocator );
  DataObjIDColl                dependencies{ { "float", "loc" } };

  BOOST_CHECK( writer.setProperty( "ExtraInputs", dependencies ).isSuccess() );
  BOOST_CHECK( writer.setProperty( "TreeFilename", "/invalid/path/to/file.root" ).isSuccess() );
  BOOST_CHECK_EXCEPTION( writer.initialize().ignore(), GaudiException, []( const GaudiException& e ) {
    return e.message() == "Failed to open file '/invalid/path/to/file.root'. Check file path and permissions.";
  } );
}

#endif
