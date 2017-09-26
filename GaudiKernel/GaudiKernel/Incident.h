#ifndef GAUDI_INCIDENT_H
#define GAUDI_INCIDENT_H

// Include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/Kernel.h"
#include <string>

/** @class Incident Incident.h GaudiKernel/Incident.h
 *
 *  Base class for all Incidents (computing events).
 *
 *  @author P. Mato
 *  @date   2001/01/19
 */

class GAUDI_API Incident
{

public:
  /// Default Constructor
  Incident( const std::string& source, ///< Incident source (service or algorithm name)
            const std::string& type    ///< Incident type
            );

  Incident( const std::string& source, ///< Incident source (service or algorithm name)
            const std::string& type,   ///< Incident type
            const EventContext& ctx    ///< EventContext
            )
      : m_source( source ), m_type( type ), m_ctx( ctx )
  {
  }

  /// Destructor
  virtual ~Incident() = default;

  /** Access to the incident type
   *
   *  @return string descriptor for the incident type
   */
  const std::string& type() const { return m_type; }

  /** Access to the source of the incident
   *
   *  @return service or algorithm name that initiated the incident
   */
  const std::string& source() const { return m_source; }

  /** Access to the EventContext of the source of the incident
   *
   *  @return EventContext of the component that initiated the incident
   */
  EventContext context() const { return m_ctx; }

private:
  std::string m_source; ///< Incident source
  std::string m_type;   ///< incident type
  EventContext m_ctx;   ///< Event Context when Incident created
};

#ifndef _inc_types_impl_
#define _inc_type_( x ) extern const std::string x
#else
#define _inc_type_( x )                                                                                                \
  extern const std::string x;                                                                                          \
  const std::string x { #x }
#endif
/** @namespace IncidentType
 *
 *  Namespace for pre-defined common incident types
 *
 *  @author P. Mato
 *  @date   2001/01/19
 *  @author R. Lambert
 *  @date   2009/09/03
 */
namespace IncidentType
{
  _inc_type_( BeginEvent ); ///< Processing of a new event has started
  _inc_type_( EndEvent );   ///< Processing of the last event has finished
  _inc_type_( BeginRun );   ///< Processing of a new run has started
  _inc_type_( EndRun );     ///< Processing of the last run has finished
  _inc_type_( EndStream );  ///< Processing of the stream has finished

  _inc_type_( AbortEvent ); ///< Stop processing the current event and pass to te next one

// Added by R. Lambert 2009-09-03, for summary services
// define a preprocessor macro to allow backward-compatibility
#define GAUDI_FILE_INCIDENTS

  _inc_type_( BeginOutputFile );   ///< a new output file has been created
  _inc_type_( FailOutputFile );    ///< could not create or write to this file
  _inc_type_( WroteToOutputFile ); ///< the output file was written to in this event
  _inc_type_( EndOutputFile );     ///< an output file has been finished

  _inc_type_( BeginInputFile ); ///< a new input file has been started
  _inc_type_( FailInputFile );  ///< could not open or read from this file
  _inc_type_( EndInputFile );   ///< an input file has been finished

  _inc_type_( CorruptedInputFile ); ///< the input file has shown a corruption

  /// Incident raised just before entering loop over the algorithms.
  _inc_type_( BeginProcessing );
  /// Incident raised just after the loop over the algorithms (note: before the execution of OutputStreams).
  _inc_type_( EndProcessing );

  /// ONLY For Services that need something after they've been finalized.
  /// Caveat Emptor: Don't use unless you're a Service or know you'll exist
  ///                after all services have been finalized!!!
  _inc_type_( SvcPostFinalize );
}
#undef _inc_type_

#endif // GAUDI_INCIDENT_H
