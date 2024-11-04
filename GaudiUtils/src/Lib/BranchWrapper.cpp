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
#include <TTree.h>
#include <fmt/format.h>
#include <map>
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
                                const std::string& branchName, const std::string& location, const std::string& algName )
      : m_className( className ), m_branchName( branchName ), m_location( location ), m_algName( algName ) {
    auto leafListTag = getLeafListForType( m_className );
    if ( leafListTag ) {
      // Create a branch for fundamental types using the leaflist
      m_branch         = tree->Branch( m_branchName.c_str(), &m_dataBuffer,
                                       ( fmt::format( "{}/{}", m_className, leafListTag.value() ) ).c_str() );
      setBranchAddress = []( gsl::not_null<TBranch*> br, const void** wrappedDataPtr ) {
        br->SetAddress( const_cast<void*>( *wrappedDataPtr ) );
      };

    } else if ( TClass::GetClass( m_className.c_str() ) ) {
      // Create a branch for object types using the classname string
      m_branch         = tree->Branch( m_branchName.c_str(), m_className.c_str(), &m_dataBuffer );
      setBranchAddress = []( gsl::not_null<TBranch*> br, const void** wrappedDataPtr ) {
        br->SetAddress( wrappedDataPtr );
      };

    } else {
      throw GaudiException( fmt::format( "Cannot create branch {} for unknown class: {}. Provide a dictionary please.",
                                         m_branchName, m_className ),
                            m_algName, StatusCode::FAILURE );
    }

    if ( !m_branch ) {
      throw GaudiException( fmt::format( "Failed to create branch {} for type {}.", m_branchName, m_className ),
                            m_algName, StatusCode::FAILURE );
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

  const std::string& BranchWrapper::getLocation() const { return m_location; }

  const std::string& BranchWrapper::getClassName() const { return m_className; }
} // namespace Gaudi::details
