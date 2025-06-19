/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Interfaces/IFileSvc.h>
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
  auto getTypeName( std::string_view dependency ) {
    auto unknownClassPos = dependency.find( "UNKNOWN_CLASS:" );
    return ( unknownClassPos != std::string::npos )
               ? std::string( dependency.substr( unknownClassPos + std::string( "UNKNOWN_CLASS:" ).length() ) )
               : std::string( dependency );
  }

  // Extract the name from a path in the TES string by returning the last part after a slash
  auto getNameFromLoc( std::string_view loc ) {
    auto lastSlashPos = loc.find_last_of( '/' );
    return std::string{ lastSlashPos != loc.npos ? loc.substr( lastSlashPos + 1 ) : loc };
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
        : Algorithm( n, l ), m_fileId( this, "OutputFile", "NTuple", "Identifier for the TFile to write to." ) {}

    // Initialize the algorithm, set up the ROOT file and a TTree branch for each input location
    StatusCode initialize() override {
      return Gaudi::Algorithm::initialize().andThen( [this]() {
        if ( m_ntupleTname.empty() ) m_ntupleTname = name();

        const auto& extraInputs = extraInputDeps();
        if ( extraInputs.empty() ) {
          error() << "No extra inputs locations specified. Please define extra inputs for the NTuple writer." << endmsg;
          return StatusCode::FAILURE;
        }

        m_fileSvc = service<Gaudi::Interfaces::IFileSvc>( "FileSvc" );
        if ( !m_fileSvc ) {
          error() << "Failed to retrieve FileSvc." << endmsg;
          return StatusCode::FAILURE;
        }

        m_file = m_fileSvc->getFile( m_fileId );
        if ( !m_file ) {
          error() << "Failed to retrieve TFile." << endmsg;
          return StatusCode::FAILURE;
        }

        m_tree = std::make_unique<TTree>( m_ntupleTname.value().c_str(), "Tree of GenericWriter Algorithm" ).release();

        createBranches( extraInputs );

        // eventSvc guarantees that the pointer is not null
        m_eventSvc = eventSvc();

        return StatusCode::SUCCESS;
      } );
    }

    // Execute the algorithm for each event, retrieving data from the event store and writing it to the TTree
    StatusCode execute( const EventContext& ) const override {
      std::vector<DataObject*> pObjs( m_branchWrappers.size(), nullptr );

      for ( std::size_t i = 0; const auto& wrapper : m_branchWrappers ) {
        m_eventSvc->retrieveObject( wrapper.getLocation(), pObjs[i++] )
            .orThrow( fmt::format( "Failed to retrieve object for location '{}'. Ensure the location is correct and "
                                   "the object exists.",
                                   wrapper.getLocation() ),
                      name() );
      }

      {
        std::scoped_lock lock{ m_mtx };
        for ( std::size_t i = 0; auto& wrapper : m_branchWrappers ) { wrapper.setBranchData( pObjs[i++] ); }
        m_tree->Fill();
      }

      return StatusCode::SUCCESS;
    }

    // Finalize the algorithm by writing the TTree to the file, resetting the branches and closing the file
    StatusCode finalize() override {
      m_file->cd();
      if ( m_tree->Write() <= 0 ) {
        error() << "Failed to write TTree to ROOT file." << endmsg;
        return StatusCode::FAILURE;
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
        m_branchWrappers.emplace_back( m_tree, typeName, branchName, dep.key(), name() );
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
    Gaudi::Property<std::string> m_fileId; // Property to hold the the identifier of where the TTree will be saved
    Gaudi::Property<std::string> m_ntupleTname{ this, "NTupleName", "",
                                                "Name of the TTree" }; // Property to hold the name of the TTree
    std::shared_ptr<TFile>       m_file     = nullptr;                 // Smart pointer to the ROOT TFile object
    TTree*                       m_tree     = nullptr;                 // Pointer to the ROOT TTree object
    IDataProviderSvc*            m_eventSvc = nullptr; // Pointer to the event service interface for data retrieval
    Gaudi::Interfaces::IFileSvc* m_fileSvc  = nullptr;
    mutable std::mutex           m_mtx; // Mutex for thread-safe operations on the GenericWriter object
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

  virtual StatusCode queryInterface( const InterfaceID&, void** ) override { return StatusCode::SUCCESS; }

  virtual std::vector<std::string> getInterfaceNames() const override { return {}; }

  virtual void const* i_cast( const InterfaceID& ) const override { return nullptr; }

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

  // Expect an failure when no dependencies are provided
  BOOST_CHECK_EQUAL( writer.initialize(), StatusCode::FAILURE );
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

#endif
