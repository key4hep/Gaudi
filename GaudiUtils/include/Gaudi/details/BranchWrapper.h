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
      void*                   m_dataBuffer = nullptr; // Data buffer for branch creation
      AnyDataWrapperBase*     baseWrapper  = nullptr; // Pointer to the base wrapper for type-safe access to data
      TBranch*                m_branch     = nullptr; // TBranch object
      std::string             m_className;            // Class name of the data
      std::string             m_branchName;           // Branch name in the TTree
      std::string             m_location;             // Location of the data in the data store
      const Gaudi::Algorithm& m_algRef;               // Reference to the Gaudi algorithm using this wrapper
      void ( *setBranchAddress )( gsl::not_null<TBranch*>,
                                  void** ); // Function pointer for the method used to set the address of the data
                                            // buffer

    public:
      BranchWrapper( const gsl::not_null<TTree*> tree, const std::string& className, const std::string& branchName,
                     const std::string& location, const Gaudi::Algorithm& algRef );

      void setDataPtr( void* dataPtr );

      void setBranchData( const gsl::not_null<DataObject*> pObj );

      std::string getLocation() const;

      std::string getClassName() const;
    };

  } // namespace details
} // namespace Gaudi
