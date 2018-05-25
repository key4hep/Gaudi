#include "CommonAuditor.h"
#include "GaudiKernel/GaudiException.h"

CommonAuditor::CommonAuditor( const std::string& name, ISvcLocator* svcloc ) : Auditor( name, svcloc )
{
  auto deprecated_property = [this]( Gaudi::Details::PropertyBase& p ) {
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
  m_customTypes.declareUpdateHandler( deprecated_property );
}

void CommonAuditor::before( StandardEventType evt, INamedInterface* caller )
{
  if ( caller ) before( toStr( evt ), caller->name() );
}
void CommonAuditor::before( StandardEventType evt, const std::string& caller ) { before( toStr( evt ), caller ); }
void CommonAuditor::before( CustomEventTypeRef evt, INamedInterface* caller )
{
  if ( caller ) before( evt, caller->name() );
}
void CommonAuditor::before( CustomEventTypeRef evt, const std::string& caller )
{
  if ( i_auditEventType( evt ) ) i_before( evt, caller );
}

void CommonAuditor::after( StandardEventType evt, INamedInterface* caller, const StatusCode& sc )
{
  if ( caller ) after( toStr( evt ), caller->name(), sc );
}
void CommonAuditor::after( StandardEventType evt, const std::string& caller, const StatusCode& sc )
{
  after( toStr( evt ), caller, sc );
}
void CommonAuditor::after( CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc )
{
  if ( caller ) after( evt, caller->name(), sc );
}
void CommonAuditor::after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc )
{
  if ( i_auditEventType( evt ) ) i_after( evt, caller, sc );
}
