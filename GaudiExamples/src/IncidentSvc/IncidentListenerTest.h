#ifndef GAUDIEXAMPLES_INCIDENTLISTENERTEST_H_
#define GAUDIEXAMPLES_INCIDENTLISTENERTEST_H_

#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/SmartIF.h"

class ISvcLocator;
class IMessageSvc;
class IIncidentSvc;

/** @class IncidentListenerTest IncidentListenerTest.h
 *
 */
class IncidentListenerTest : public implements<IIncidentListener>
{

public:
  /// Constructor
  IncidentListenerTest( const std::string& name, ISvcLocator* svcloc, long shots = -1 );

  /// Destructor
  ~IncidentListenerTest() override;

  /// Reimplements from IIncidentListener
  void handle( const Incident& incident ) override;

private:
  std::string m_name;
  long m_shots;
  SmartIF<IMessageSvc> m_msgSvc;
  SmartIF<IIncidentSvc> m_incSvc;
};

#endif /*GAUDIEXAMPLES_INCIDENTLISTENERTEST_H_*/
