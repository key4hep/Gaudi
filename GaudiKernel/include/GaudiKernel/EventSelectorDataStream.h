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
//====================================================================
//	EventSelectorDataStream.h
//--------------------------------------------------------------------
//
//	Package    : EventSelectorDataStream  (LHCb Event Selector Package)
//
//	Author     : M.Frank
//      Created    : 4/10/00
//	Changes    : R. Lambert 2009-09-04
//
//====================================================================
#pragma once

// Include files
#include <Gaudi/Property.h>
#include <GaudiKernel/IEvtSelector.h>

// STL include files
#include <iostream>
#include <vector>

// Forward declarations
class MsgStream;
class ISvcLocator;
class EventSelectorDataStream;

/** Definition of class EventSelectorDataStream

    Small class which eases the management of multiple
    input streams for the event selector.

    History:
    +---------+----------------------------------------------+------------+
    |    Date |                 Comment                      |    Who     |
    +---------+----------------------------------------------+------------+
    | 3/10/00 | Initial version                              | M.Frank    |
    +---------+----------------------------------------------+------------+
    | 4/09/09 | Added m_dbName and dbName()                  | R. Lambert |
    +---------+----------------------------------------------+------------+
   @author Markus Frank
   @author R. Lambert
   @version 1.0
*/
class GAUDI_API EventSelectorDataStream : public implements<IInterface> {
  /// Output friend for MsgStream
  friend MsgStream& operator<<( MsgStream& s, const EventSelectorDataStream& obj );
  /// Output friend for standard I/O
  friend std::ostream& operator<<( std::ostream& s, const EventSelectorDataStream& obj );

public:
  typedef std::vector<Gaudi::Property<std::string>> Properties;

protected:
  /// Name
  std::string m_name;
  /// Definition string
  std::string m_definition;
  /// Criteria
  std::string m_criteria;
  /// String with name of the db as parsed
  std::string m_dbName;
  /// Event selector type
  std::string m_selectorType;
  /// Pointer to valid selector
  SmartIF<IEvtSelector> m_pSelector;
  /// Reference to service locator
  ISvcLocator* m_pSvcLocator;
  /// Properties vector
  Properties m_properties;
  /// Initialization state
  bool m_initialized = false;
  /// Standard Destructor
  virtual ~EventSelectorDataStream() = default;

public:
  /// Standard Constructor
  EventSelectorDataStream( std::string nam, std::string def, ISvcLocator* svcloc );
  /// Attach event selector object
  virtual void setSelector( IEvtSelector* pSelector );
  /// Parse input criteria
  virtual StatusCode initialize();
  /// Finalize stream and release resources
  virtual StatusCode finalize();
  /// Allow access to individual properties by name
  Gaudi::Property<std::string>* property( const std::string& nam );
  /// Allow access to individual properties by name (CONST)
  const Gaudi::Property<std::string>* property( const std::string& nam ) const;
  /// Access properties
  const Properties& properties() { return m_properties; }
  /// Retrieve stream name
  const std::string& name() const { return m_name; }
  /// Retrieve stream criteria
  const std::string& criteria() const { return m_criteria; }
  /// Retrieve stream dbName
  const std::string& dbName() const { return m_dbName; }
  /// Retrieve event selector type
  const std::string& selectorType() const { return m_selectorType; }
  /// Retrieve definition string
  const std::string& definition() const { return m_definition; }
  /// Retrieve event selector object
  IEvtSelector* selector() const { return m_pSelector.get(); }
  /// Check initialization status
  bool isInitialized() const { return m_initialized; }
};

/// Output friend for MsgStream
GAUDI_API MsgStream& operator<<( MsgStream& s, const EventSelectorDataStream& obj );
/// Output friend for standard I/O
GAUDI_API std::ostream& operator<<( std::ostream& s, const EventSelectorDataStream& obj );
