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
#include <GaudiKernel/IDataSelector.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/ISelectStatement.h>
#include <GaudiKernel/NTupleImplementation.h>

namespace NTuple {
  TupleImp::TupleImp( std::string title ) : m_title( std::move( title ) ) {}
  TupleImp::~TupleImp() {
    for ( auto& i : m_items ) i->release();
  }

  /// Attach selector
  StatusCode TupleImp::attachSelector( ISelectStatement* sel ) {
    m_pSelector = sel;
    return StatusCode::SUCCESS;
  }

  /// Access selector
  ISelectStatement* TupleImp::selector() { return m_pSelector.get(); }

  /// Reset N tuple to default values
  void TupleImp::reset() {
    for ( auto& i : m_items ) i->reset();
  }

  /// Locate a column of data to the N tuple (not type safe)
  INTupleItem* TupleImp::i_find( const std::string& name ) const {
    auto i = std::find_if( std::begin( m_items ), std::end( m_items ),
                           [&]( ItemContainer::const_reference j ) { return j->name() == name; } );
    return i != std::end( m_items ) ? const_cast<INTupleItem*>( *i ) : nullptr;
  }

  /// Add an item row to the N tuple
  StatusCode TupleImp::add( INTupleItem* item ) {
    if ( item ) {
      INTupleItem* i = i_find( item->name() );
      if ( !i ) {
        m_items.push_back( item );
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }

  /// Remove a column from the N-tuple
  StatusCode TupleImp::remove( const std::string& name ) {
    INTupleItem* i = i_find( name );
    return i ? remove( i ) : StatusCode::FAILURE;
  }

  /// Remove a column from the N-tuple
  StatusCode TupleImp::remove( INTupleItem* item ) {
    auto i = std::find( std::begin( m_items ), std::end( m_items ), item );
    if ( i == std::end( m_items ) ) return StatusCode::FAILURE;
    m_items.erase( i );
    item->release();
    return StatusCode::SUCCESS;
  }
  /// Set N tuple data buffer
  char* TupleImp::setBuffer( std::unique_ptr<char[]>&& buff ) {
    m_buffer = std::move( buff );
    return m_buffer.get();
  }
  /// Set N tuple data buffer
  char* TupleImp::setBuffer( char* buff ) {
    m_buffer.reset( buff );
    return m_buffer.get();
  }
  /// Write record of the NTuple
  StatusCode TupleImp::write() { return m_ntupleSvc->writeRecord( this ); }
  /// Write record of the NTuple
  StatusCode TupleImp::writeRecord() { return m_ntupleSvc->writeRecord( this ); }
  /// Read record of the NTuple
  StatusCode TupleImp::read() { return m_ntupleSvc->readRecord( this ); }
  /// Read record of the NTuple
  StatusCode TupleImp::readRecord() { return m_ntupleSvc->readRecord( this ); }
  /// Save the NTuple
  StatusCode TupleImp::save() { return m_ntupleSvc->save( this ); }
} // namespace NTuple
