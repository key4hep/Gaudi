#ifndef COMPONENT_RECORDOUTPUTSTREAM_H
#define COMPONENT_RECORDOUTPUTSTREAM_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class RecordOutputStream
  *
  * Simple class that adds an entry to the Transient Event Store to record that
  * the processing flow triggered its execution.
  *
  * The main use case is to replace OutputStream instances appearing in the
  * normal processing flow of workers in GaudiMP jobs. The information is then
  * passed to the writer process where an instance of @c ReplayOutputStream will
  * trigger the actual writing.
  *
  * @author Marco Clemencic
  * @date 30/08/2013
  */
class RecordOutputStream : public GaudiAlgorithm
{
public:
  using GaudiAlgorithm::GaudiAlgorithm;

  ~RecordOutputStream() override = default; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

  /// Return the path in the Transient Store used to record the triggered
  /// instances.
  static inline const std::string locationRoot() { return "TriggeredOutputStreams"; }
protected:
private:
  Gaudi::Property<std::string> m_streamName{
      this, "OutputStreamName", {}, "Name of the OutputStream instance should be triggered."};

  /// location of the DataObject flag used to record that this algorithm was
  /// called
  std::string m_flagLocation;
};

#endif // COMPONENT_RECORDOUTPUTSTREAM_H
