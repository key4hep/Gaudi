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
#pragma once

#include <Gaudi/Property.h>
#include <GaudiKernel/DataHandle.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IDataHandleHolder.h>

#include <algorithm>
#include <unordered_set>

namespace {
  template <typename Container>
  std::vector<Gaudi::DataHandle*> handles( Container& c, Gaudi::DataHandle::Mode mode ) {
    std::vector<Gaudi::DataHandle*> h;
    std::copy_if( std::begin( c ), std::end( c ), std::back_inserter( h ),
                  [&]( const Gaudi::DataHandle* hndl ) -> bool { return hndl->mode() & mode; } );
    return h;
  }
} // namespace

template <class BASE>
class GAUDI_API DataHandleHolderBase : public extends<BASE, IDataHandleHolder> {
public:
  using extends<BASE, IDataHandleHolder>::extends;

  std::vector<Gaudi::DataHandle*> inputHandles() const override {
    return handles( m_handles, Gaudi::DataHandle::Reader );
  }
  std::vector<Gaudi::DataHandle*> outputHandles() const override {
    return handles( m_handles, Gaudi::DataHandle::Writer );
  }

  virtual const DataObjIDColl& extraInputDeps() const override { return m_extInputDataObjs; }
  virtual const DataObjIDColl& extraOutputDeps() const override { return m_extOutputDataObjs; }

  void declare( Gaudi::DataHandle& handle ) override {
    if ( !handle.owner() ) handle.setOwner( this );

    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to declare foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }

    m_handles.insert( &handle );
  }

  void renounce( Gaudi::DataHandle& handle ) override {
    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to renounce foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }
    m_handles.erase( &handle );
  }

  bool renounceInput( const DataObjID& id ) override {
    bool renounced = false;
    // TODO: C++20: use std::erase_if
    for ( auto i = m_handles.begin(), last = m_handles.end(); i != last; ) {
      if ( ( *i )->mode() == Gaudi::DataHandle::Reader && ( *i )->fullKey() == id ) {
        i         = m_handles.erase( i );
        renounced = true;
      } else {
        ++i;
      }
    }
    if ( auto elm = m_inputDataObjs.find( id ); elm != m_inputDataObjs.end() ) {
      m_inputDataObjs.erase( elm );
      renounced = true;
    }
    return renounced;
  }

  const DataObjIDColl& inputDataObjs() const override { return m_inputDataObjs; }
  const DataObjIDColl& outputDataObjs() const override { return m_outputDataObjs; }

  void addDependency( const DataObjID& id, const Gaudi::DataHandle::Mode& mode ) override {
    if ( mode & Gaudi::DataHandle::Reader ) m_inputDataObjs.emplace( id );
    if ( mode & Gaudi::DataHandle::Writer ) m_outputDataObjs.emplace( id );
  }

protected:
  /// initializes all handles - called by the sysInitialize method
  /// of any descendant of this
  void initDataHandleHolder() {
    for ( auto h : m_handles ) h->init();
  }

  DataObjIDColl m_inputDataObjs, m_outputDataObjs;

private:
  std::unordered_set<Gaudi::DataHandle*> m_handles;

  Gaudi::Property<DataObjIDColl> m_extInputDataObjs{ this, "ExtraInputs", DataObjIDColl{} };
  Gaudi::Property<DataObjIDColl> m_extOutputDataObjs{ this, "ExtraOutputs", DataObjIDColl{} };
};
