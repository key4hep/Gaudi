/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIHIVE_DATAOBJECTHANDLEBASE_H
#define GAUDIHIVE_DATAOBJECTHANDLEBASE_H

#include <mutex>

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"

//---------------------------------------------------------------------------

/** DataObjectHandleBase  GaudiKernel/DataObjectHandleBase.h
 *
 * Base class for handles to Data Objects in the Event Store, to simplify
 * access via Properties.
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class DataObjectHandleBase : public Gaudi::DataHandle {

public:
  DataObjectHandleBase( DataObjID k, Gaudi::DataHandle::Mode a, IDataHandleHolder* owner );
  DataObjectHandleBase( std::string k, Gaudi::DataHandle::Mode a, IDataHandleHolder* owner );

  ~DataObjectHandleBase() override;
  DataObjectHandleBase( const DataObjectHandleBase& ) = delete;
  DataObjectHandleBase( DataObjectHandleBase&& );
  DataObjectHandleBase& operator=( const DataObjectHandleBase& );

  /// Autodeclaring constructor with property name, mode, key and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, class K, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
  inline DataObjectHandleBase( OWNER* owner, Gaudi::DataHandle::Mode m, std::string name, K key = {},
                               std::string doc = "" )
      : DataObjectHandleBase( std::move(key), m, owner ) {
    auto p = owner->declareProperty( std::move( name ), *this, std::move( doc ) );
    p->template setOwnerType<OWNER>();
  }

  friend std::ostream& operator<<( std::ostream& str, const DataObjectHandleBase& d );

  /// Check if the data object declared is optional for the algorithm
  bool isOptional() const { return m_optional; }
  void setOptional( bool optional = true ) { m_optional = optional; }

  bool initialized() const { return m_init; }
  bool wasRead() const { return m_wasRead; }
  bool wasWritten() const { return m_wasWritten; }

  bool isValid() const;

protected:
  void setRead( bool wasRead = true ) { m_wasRead = wasRead; }
  void setWritten( bool wasWritten = true ) { m_wasWritten = wasWritten; }

  bool init() override;

  DataObject* fetch() const;

protected:
  SmartIF<IDataProviderSvc> m_EDS;
  SmartIF<IMessageSvc>      m_MS;

  bool m_init       = false;
  bool m_optional   = false;
  bool m_wasRead    = false;
  bool m_wasWritten = false;

  /**
   * Whether the search part of the fetch method (so dealing with alt names
   * was already executed or not. On subsequent calls (when this is true),
   * it will be skipped.
   */
  mutable bool m_searchDone = false; // TODO: optimize it so that it is std::any_of( objKey, ":" )

  /**
   * A Mutex protecting the calls to the search part of the fetch method,
   * so that we are sure that we only call it once
   */
  mutable std::mutex m_searchMutex;
};

#endif
