/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/DataObject.h>
#include <TBranch.h>
#include <gsl/pointers>
#include <memory>

/// Helper for type agnostic reading of data in branches
///
/// Use like this:
/// ```
/// auto f = TFile::Open("file.root");
/// auto t = f->Get<TTree>("tree");
/// BranchReadHelper b(t->GetBranch("branch"));
/// for (int i = 0; i < t->GetEntries(); ++i) {
///     t->GetEntry(i);
///     std::unique_ptr<DataObject> obj = b.adoptValue();
///     ...
/// }
/// ```

namespace Gaudi::details {

  class BranchReadHelper final {
  public:
    explicit BranchReadHelper( gsl::not_null<TBranch*> b );
    BranchReadHelper( const BranchReadHelper& other )            = delete;
    BranchReadHelper& operator=( const BranchReadHelper& other ) = delete;
    BranchReadHelper( BranchReadHelper&& other ) noexcept;
    BranchReadHelper& operator=( BranchReadHelper&& other ) noexcept;

    std::unique_ptr<DataObject> adoptValue();

    ~BranchReadHelper();

  private:
    TBranch* m_branch{ nullptr };
    void*    m_store{ nullptr };

    DataObject* ( *m_extractor )( void* ) = nullptr;
  };

} // namespace Gaudi::details
