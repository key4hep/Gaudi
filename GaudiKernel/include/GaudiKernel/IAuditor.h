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

#include <Gaudi/IAuditor.h>
#include <GaudiKernel/INamedInterface.h>
#include <array>
#include <string>

/** @class IAuditor IAuditor.h GaudiKernel/IAuditor.h

    The IAuditor is the interface implemented by the Auditor base class.
    Concrete auditors, derived from the Auditor base class are controlled
    via this interface.

    This version of the interface is deprecated and Gaudi::IAuditor should now be used.

    @author Marjorie Shapiro, LBNL
    @author Marco Clemencic <marco.clemencic@cern.ch>
*/
class GAUDI_API IAuditor : virtual public Gaudi::IAuditor {
public:
  /// InterfaceID
  DeclareInterfaceID( IAuditor, 4, 0 );

  /// Defines the standard (= used by the framework) auditable event types.
  enum StandardEventType { Initialize, ReInitialize, Execute, Finalize, Start, Stop, ReStart };
  /// Simple mapping function from IAuditor::StandardEventType to string.
  friend const char* toStr( IAuditor::StandardEventType e ) {
    constexpr std::array<const char*, IAuditor::StandardEventType::ReStart + 1> s_tbl = {
        { "Initialize", "ReInitialize", "Execute", "Finalize", "Start", "Stop", "ReStart" } };
    return e <= IAuditor::StandardEventType::ReStart ? s_tbl[e] : nullptr;
  }
  friend std::ostream& operator<<( std::ostream& s, IAuditor::StandardEventType e ) { return s << toStr( e ); }

  /// Type used to allow users to specify a custom event to be audit.
  /// Examples of custom events are callbacks (see
  /// <a href="https://savannah.cern.ch/patch/index.php?1725">patch #1725</a>).
  typedef std::string CustomEventType;
  /// Used in function calls for optimization purposes.
  typedef const CustomEventType& CustomEventTypeRef;

  /// iherit new interface
  using Gaudi::IAuditor::after;
  using Gaudi::IAuditor::before;

  /// Audit the start of a standard "event".
  virtual void before( StandardEventType, INamedInterface* ) = 0;
  /// Audit the start of a standard "event" for callers that do not implement INamedInterface.
  virtual void before( StandardEventType, const std::string& ) = 0;

  /// Audit the start of a custom "event".
  virtual void before( CustomEventTypeRef, INamedInterface* ) = 0;
  /// Audit the start of a custom "event" for callers that do not implement INamedInterface.
  virtual void before( CustomEventTypeRef, const std::string& ) = 0;

  /// Audit the end of a standard "event".
  virtual void after( StandardEventType, INamedInterface*, const StatusCode& sc = StatusCode::SUCCESS ) = 0;
  /// Audit the end of a standard "event" for callers that do not implement INamedInterface.
  virtual void after( StandardEventType, const std::string&, const StatusCode& sc = StatusCode::SUCCESS ) = 0;

  /// Audit the end of a custom "event".
  virtual void after( CustomEventTypeRef, INamedInterface*, const StatusCode& sc = StatusCode::SUCCESS ) = 0;
  /// Audit the end of a custom "event" for callers that do not implement INamedInterface.
  virtual void after( CustomEventTypeRef, const std::string&, const StatusCode& sc = StatusCode::SUCCESS ) = 0;
};
