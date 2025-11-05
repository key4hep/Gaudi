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
#include <Gaudi/details/BranchWrapper.h>
#include <TMemFile.h>
#include <TTree.h>
#include <format>
#include <map>
#include <memory>
#include <string>

namespace {
  // Mapping from C++ type names to ROOT branch types for branch creation
  const std::map<std::string, std::string, std::less<>> typeMap = {
      { "char", "B" },      { "unsigned char", "b" },      { "short", "S" }, { "unsigned short", "s" },
      { "int", "I" },       { "unsigned int", "i" },       { "float", "F" }, { "double", "D" },
      { "long long", "L" }, { "unsigned long long", "l" }, { "long", "G" },  { "unsigned long", "g" },
      { "bool", "O" } };

  // Helper function to find the ROOT branch type corresponding to a C++ type
  std::optional<std::string> getLeafListForType( const std::string_view& typeName ) {
    auto it = typeMap.find( typeName );
    return ( it != typeMap.end() ) ? std::optional<std::string>{ it->second } : std::nullopt;
  }
} // namespace

namespace Gaudi::details {

  BranchWrapper::BranchWrapper( const gsl::not_null<TTree*> tree, const std::string& className,
                                const std::string& branchName, const std::string& location, const std::string& algName,
                                unsigned int bufferSize, unsigned int splitLevel )
      : m_className( className ), m_location( location ) {
    auto leafListTag = getLeafListForType( m_className );
    if ( leafListTag ) {
      // Create a branch for fundamental types using the leaflist
      m_branch         = tree->Branch( branchName.c_str(), &m_dataBuffer,
                                       ( std::format( "{}/{}", m_className, leafListTag.value() ) ).c_str(), bufferSize );
      setBranchAddress = []( gsl::not_null<TBranch*> br, const void** wrappedDataPtr ) {
        br->SetAddress( const_cast<void*>( *wrappedDataPtr ) );
      };

    } else if ( TClass::GetClass( m_className.c_str() ) ) {
      // Create a branch for object types using the classname string
      m_branch         = tree->Branch( branchName.c_str(), m_className.c_str(), &m_dataBuffer, bufferSize, splitLevel );
      setBranchAddress = []( gsl::not_null<TBranch*> br, const void** wrappedDataPtr ) {
        br->SetAddress( wrappedDataPtr );
      };

    } else {
      throw GaudiException( std::format( "Cannot create branch {} for unknown class: {}. Provide a dictionary please.",
                                         branchName, m_className ),
                            algName, StatusCode::FAILURE );
    }

    if ( !m_branch ) {
      throw GaudiException( std::format( "Failed to create branch {} for type {}.", branchName, m_className ), algName,
                            StatusCode::FAILURE );
    }
  }

  // Set the data pointer for the branch from a given address
  // Used by Gaudi::NTuple::Writer
  void BranchWrapper::setDataPtr( void const* dataPtr ) {
    m_dataBuffer = dataPtr;
    setBranchAddress( m_branch, &m_dataBuffer );
  }

  // Set the data for the branch from a given DataObject
  // Used by Gaudi::NTuple::GenericWriter
  void BranchWrapper::setBranchData( const gsl::not_null<DataObject*> pObj ) {
    auto baseWrapper = dynamic_cast<AnyDataWrapperBase*>( pObj.get() );
    m_dataBuffer     = baseWrapper ? baseWrapper->payload() : pObj.get();
    setBranchAddress( m_branch, &m_dataBuffer );
  }

  unsigned int BranchWrapper::computeOptimalBufferSize( unsigned int minBufferSize, unsigned int maxBufferSize,
                                                        unsigned int approxEventsPerBasket, unsigned int splitLevel ) {
    // simply try to write one item in a dummey branch of a dummy tree im memory and measure size
    auto dummy_file  = std::make_unique<TMemFile>( "dummy.root", "CREATE" );
    auto dummy_tree  = std::make_unique<TTree>( "DummyTree", "DummyTree", splitLevel, dummy_file->GetDirectory( "/" ) );
    auto leafListTag = getLeafListForType( m_className );
    TBranch* dummy_branch{ nullptr };
    if ( leafListTag ) {
      // Create a branch for fundamental types using the leaflist
      dummy_branch =
          dummy_tree->Branch( "DummyBranch", &m_dataBuffer,
                              ( fmt::format( "{}/{}", m_className, leafListTag.value() ) ).c_str(), minBufferSize );
    } else if ( TClass::GetClass( m_className.c_str() ) ) {
      // Create a branch for object types using the classname string
      dummy_branch = dummy_tree->Branch( "DummyBranch", m_className.c_str(), &m_dataBuffer, minBufferSize, splitLevel );
    } // no else as unknown className would have raised an exception at constructor level
    int nWritten = dummy_branch->Fill();
    if ( nWritten >= 0 ) {
      unsigned int newBasketSize = nWritten * approxEventsPerBasket;
      // Ensure that newBasketSize doesn't wrap around
      if ( std::numeric_limits<Int_t>::max() / approxEventsPerBasket < (unsigned int)nWritten ) {
        newBasketSize = std::numeric_limits<Int_t>::max();
      }
      return std::min( maxBufferSize, std::max( minBufferSize, newBasketSize ) );
    }
    return minBufferSize;
  }

  void BranchWrapper::padEntries() {
    if ( !m_branch ) return;
    auto nEvents  = m_branch->GetTree()->GetEntries();
    auto nEntries = m_branch->GetEntries();
    if ( nEntries < nEvents ) {
      m_branch->SetAddress( nullptr );
      for ( auto i = nEntries; i < nEvents; i++ ) { m_branch->Fill(); }
    }
  }

} // namespace Gaudi::details
