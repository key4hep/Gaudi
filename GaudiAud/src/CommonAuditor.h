/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIAUD_COMMONAUDITOR_H
#define GAUDIAUD_COMMONAUDITOR_H

#include "GaudiKernel/Auditor.h"

/// Base class with common functionalities shared by few auditor implementations.
/// @author Marco Clemencic
class CommonAuditor : public Auditor {
public:
  /** Constructor
      @param name    The algorithm object's name
      @param svcloc  A pointer to a service location service */
  using Auditor::Auditor;

  /// \name "before" Auditor hooks
  /// The default behavior is to fall back on the version accepting 2 strings,
  /// which must be implemented in the derived class.
  //@{
  void before( StandardEventType evt, const std::string& caller ) override;
  void before( StandardEventType evt, INamedInterface* caller ) override;
  void before( CustomEventTypeRef evt, const std::string& caller ) override;
  void before( CustomEventTypeRef evt, INamedInterface* caller ) override;
  //@}

  /// \name "after" Auditor hooks
  /// The default behavior is to fall back on the version accepting 2 strings,
  /// which must be implemented in the derived class.
  //@{
  void after( StandardEventType evt, const std::string& caller, const StatusCode& sc ) override;
  void after( StandardEventType evt, INamedInterface* caller, const StatusCode& sc ) override;
  void after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc ) override;
  void after( CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc ) override;
  //@}

protected:
  /// catch all "before" method, implemented in the derived class
  virtual void i_before( CustomEventTypeRef evt, const std::string& caller ) = 0;
  /// catch all "after" method, implemented in the derived class
  virtual void i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc ) = 0;

  /// Check if we are requested to audit the passed event type.
  inline bool i_auditEventType( const std::string& evt ) {
    // Note: there is no way to extract from a Property type the type returned by
    // value().
    const std::vector<std::string>& v = m_types.value();
    // we need to return true is the list is empty or when the list does't
    // start by "none" and the list contain the event we got.
    return v.empty() || ( ( v[0] != "none" ) && ( find( v.begin(), v.end(), evt ) != v.end() ) );
  }

private:
  void deprecated_property( Gaudi::Details::PropertyBase& p ) {
    if ( m_customTypes.size() > 0 ) {
      if ( m_types.size() > 0 ) {
        error() << p.name() << " is deprecated, but both " << m_customTypes.name() << " and " << m_types.name()
                << " used." << endmsg;
        throw GaudiException( "Property CustomEventTypes is deprecated, but both CustomEventTypes and EventTypes used",
                              this->name(), StatusCode::FAILURE );
      } else {
        warning() << p.name() << " " << p.documentation() << endmsg;
        m_types = m_customTypes;
      }
    }
  };

  Gaudi::Property<std::vector<std::string>> m_types{
      this, "EventTypes", {}, "list of event types to audit ([]=all, ['none']=none)"};
  Gaudi::Property<std::vector<std::string>> m_customTypes{
      this, "CustomEventTypes", {}, &CommonAuditor::deprecated_property, "[[deprecated]] use EventTypes instead"};
};

#endif // GAUDIAUD_COMMONAUDITOR_H
