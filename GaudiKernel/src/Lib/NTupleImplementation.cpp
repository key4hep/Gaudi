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
  TupleImp::TupleImp ( std::string title )
  : m_isBooked(false),
    m_title(std::move(title)),
    m_pSelector(0),
    m_ntupleSvc(0),
    m_cnvSvc(0)
  {
  }

  /// Standard Destructor
  TupleImp::~TupleImp ()   {
    for (auto &i : m_items) i->release();
  }

  /// Attach selector
  StatusCode TupleImp::attachSelector(ISelectStatement* sel)  {
    if ( sel         ) sel->addRef();
    if ( m_pSelector ) m_pSelector->release();
    m_pSelector = sel;
    return StatusCode::SUCCESS;
  }

  /// Access selector
  ISelectStatement* TupleImp::selector()   {
    return m_pSelector;
  }

  /// Reset N tuple to default values
  void TupleImp::reset ( )   {
    for (auto& i : m_items ) i->reset();
  }

  /// Locate a column of data to the N tuple (not type safe)
  INTupleItem* TupleImp::i_find ( const std::string& name )  const   {
    for (auto& i : m_items ) {
      if ( name == i->name() )   {
        return const_cast<INTupleItem*>(i);
      }
    }
    return nullptr;
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
    for (auto i = m_items.begin(); i != m_items.end(); i++) {
      if ( (*i) == item )   {
        m_items.erase(i);
        item->release();
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }
  /// Set N tuple data buffer
  char* TupleImp::setBuffer(std::unique_ptr<char[]>&& buff)  {
    m_buffer = std::move(buff);
    return m_buffer.get();
  }
  /// Set N tuple data buffer
  char* TupleImp::setBuffer(char* buff)  {
    m_buffer.reset( buff );
    return m_buffer.get();
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
