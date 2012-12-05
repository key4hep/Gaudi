#ifndef GAUDI_INCIDENT_H
#define GAUDI_INCIDENT_H

// Include files
#include "GaudiKernel/Kernel.h"
#include <string>

/** @class Incident Incident.h GaudiKernel/Incident.h
 *
 *  Base class for all Incidents (computing events).
 *
 *  @author P. Mato
 *  @date   2001/01/19
 */

class GAUDI_API Incident {

public:

  /// Default Constructor
  Incident ( const std::string& source, ///< Incident source (service or algorithm name)
             const std::string& type    ///< Incident type
             )
    : m_source ( source ),
      m_type   ( type   ) { }

  /// Destructor
  virtual ~Incident() { }

  /** Access to the incident type
   *
   *  @return string descriptor for the incident type
   */
  const std::string& type()  const { return m_type; }

  /** Access to the source of the incident
   *
   *  @return service or algorithm name that initiated the incident
   */
  const std::string& source() const { return m_source; }

private:

  std::string m_source; ///< Incident source
  std::string m_type;   ///< incident type

};

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
  const std::string BeginEvent = "BeginEvent"; ///< Processing of a new event has started
  const std::string EndEvent   = "EndEvent";   ///< Processing of the last event has finished
  const std::string BeginRun   = "BeginRun";   ///< Processing of a new run has started
  const std::string EndRun     = "EndRun";     ///< Processing of the last run has finished
  const std::string EndStream  = "EndStream";  ///< Processing of the stream has finished

  const std::string AbortEvent = "AbortEvent"; ///< Stop processing the current event and pass to te next one

  //Added by R. Lambert 2009-09-03, for summary services
  //define a preprocessor macro to allow backward-compatibility
#define GAUDI_FILE_INCIDENTS

  const std::string BeginOutputFile = "BeginOutputFile"; ///< a new output file has been created
  const std::string FailOutputFile = "FailOutputFile"; ///< could not create or write to this file
  const std::string WroteToOutputFile = "WroteToOutputFile"; ///< the output file was written to in this event
  const std::string EndOutputFile   = "EndOutputFile";   ///< an output file has been finished

  const std::string BeginInputFile = "BeginInputFile"; ///< a new input file has been started
  const std::string FailInputFile = "FailInputFile"; ///< could not open or read from this file
  const std::string EndInputFile   = "EndInputFile";   ///< an input file has been finished

  const std::string CorruptedInputFile = "CorruptedInputFile"; ///< the input file has shown a corruption

  /// Incident raised just before entering loop over the algorithms.
  const std::string BeginProcessing = "BeginProcessing";
  /// Incident raised just after the loop over the algorithms (note: before the execution of OutputStreams).
  const std::string EndProcessing = "EndProcessing";
}

#endif //GAUDI_INCIDENT_H

