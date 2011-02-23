// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/NTupleImplementation.cpp,v 1.7 2006/12/10 20:29:18 leggett Exp $
//====================================================================
//  NTuple class implementation
//--------------------------------------------------------------------
//
//  Package    : Gaudi/NTupleSvc ( The LHCb Offline System)
//  Author     : M.Frank
//
//  +---------+----------------------------------------------+--------+
//  |    Date |                 Comment                      | Who    |
//  +---------+----------------------------------------------+--------+
//  | 21/10/99| Initial version.                             | MF     |
//  +---------+----------------------------------------------+--------+
//====================================================================
#define GAUDI_NTUPLEIMP_CPP 1

// Framework include files
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/ISelectStatement.h"
#include "GaudiKernel/NTupleImplementation.h"

/*
*/
namespace NTuple   {
  /// Standard Constructor
  TupleImp::TupleImp ( const std::string& title )
  : m_isBooked(false),
    m_title(title),
    m_pSelector(0),
    m_buffer(0),
    m_ntupleSvc(0),
    m_cnvSvc(0)
  {
  }

  /// Standard Destructor
  TupleImp::~TupleImp ()   {
    for (ItemContainer::iterator i = m_items.begin(); i != m_items.end(); i++) {
      (*i)->release();
    }
    m_items.erase(m_items.begin(), m_items.end());
    delete  [] m_buffer;
    m_buffer = 0;
  }

  /// Attach selector
  StatusCode TupleImp::attachSelector(ISelectStatement* sel)  {
    if ( 0 != sel         ) sel->addRef();
    if ( 0 != m_pSelector ) m_pSelector->release();
    m_pSelector = sel;
    return StatusCode::SUCCESS;
  }

  /// Access selector
  ISelectStatement* TupleImp::selector()   {
    return m_pSelector;
  }

  /// Reset N tuple to default values
  void TupleImp::reset ( )   {
    for (ItemContainer::iterator i = m_items.begin(); i != m_items.end(); i++) {
      (*i)->reset();
    }
  }

  /// Locate a column of data to the N tuple (not type safe)
  INTupleItem* TupleImp::i_find ( const std::string& name )  const   {
    for (ItemContainer::const_iterator i = m_items.begin();
         i != m_items.end();
         i++) {
      if ( name == (*i)->name() )   {
        INTupleItem* it = const_cast<INTupleItem*>(*i);
        return it;
      }
    }
    return 0;
  }

  /// Add an item row to the N tuple
  StatusCode TupleImp::add(INTupleItem* item)   {
    if ( 0 != item )    {
      INTupleItem* i = i_find(item->name());
      if ( 0 == i )   {
        m_items.push_back( item );
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }

  /// Remove a column from the N-tuple
  StatusCode TupleImp::remove ( const std::string& name )    {
    INTupleItem* i = i_find(name);
    return (i == 0) ?  StatusCode(StatusCode::FAILURE) : remove(i);
  }

  /// Remove a column from the N-tuple
  StatusCode TupleImp::remove ( INTupleItem* item )    {
    for (ItemContainer::iterator i = m_items.begin(); i != m_items.end(); i++) {
      if ( (*i) == item )   {
        m_items.erase(i);
        item->release();
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }
  /// Set N tuple data buffer
  void TupleImp::setBuffer(char* buff)  {
    if ( 0 != m_buffer ) delete m_buffer;
    m_buffer = buff;
  }
  /// Write record of the NTuple
  StatusCode TupleImp::write()    {
    return m_ntupleSvc->writeRecord(this);
  }
  /// Write record of the NTuple
  StatusCode TupleImp::writeRecord()    {
    return m_ntupleSvc->writeRecord(this);
  }
  /// Read record of the NTuple
  StatusCode TupleImp::read()    {
    return m_ntupleSvc->readRecord(this);
  }
  /// Read record of the NTuple
  StatusCode TupleImp::readRecord()    {
    return m_ntupleSvc->readRecord(this);
  }
  /// Save the NTuple
  StatusCode TupleImp::save()    {
    return m_ntupleSvc->save(this);
  }
}    // end namespace NTuple
