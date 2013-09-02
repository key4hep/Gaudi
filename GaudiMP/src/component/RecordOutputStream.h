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
class RecordOutputStream: public GaudiAlgorithm {
public:
  /// Standard constructor
  RecordOutputStream(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~RecordOutputStream(); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  /// Return the path in the Transient Store used to record the triggered
  /// instances.
  static inline const std::string locationRoot() {
    return "TriggeredOutputStreams";
  }
protected:
private:
  /// Name of the OuputStream that should be called when this algorithm is
  /// triggered.
  std::string m_streamName;

  /// location of the DataObject flag used to record that this algorithm was
  /// called
  std::string m_flagLocation;
};

#endif // COMPONENT_RECORDOUTPUTSTREAM_H
