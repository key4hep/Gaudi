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
#pragma once

#include <Gaudi/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <TTree.h>
#include <gsl/pointers>
#include <string>

namespace Gaudi {
  namespace details {
    /**
     * @struct BranchWrapper
     * @brief Encapsulates a branch within a ROOT TTree, managing the data and interaction with the TTree.
     */
    struct BranchWrapper {

    private:
      void const* m_dataBuffer = nullptr; // Data buffer for branch creation
      TBranch*    m_branch     = nullptr; // TBranch object
      std::string m_className;            // Class name of the data
      std::string m_branchName;           // Branch name in the TTree
      std::string m_location;             // Location of the data in the data store
      std::string m_algName;              // Reference to the Gaudi algorithm using this wrapper
      void ( *setBranchAddress )( gsl::not_null<TBranch*>,
                                  const void** ); // Function pointer for the method used to set the address of the data
                                                  // buffer

    public:
      BranchWrapper( const gsl::not_null<TTree*> tree, const std::string& className, const std::string& branchName,
                     const std::string& location, const std::string& algName );

      void setDataPtr( void const* dataPtr );

      void setBranchData( const gsl::not_null<DataObject*> pObj );

      const std::string& getLocation() const;

      const std::string& getClassName() const;
    };

  } // namespace details
} // namespace Gaudi
