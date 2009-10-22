// $Id: $
#ifndef GAUDISVC_PERSISTENCYSVC_RUNRECORDSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_RUNRECORDSTREAM_H

// Required for inheritance
#include "OutputStream.h"

/** @class RunRecordStream RunRecordStream.h
  * Extension of OutputStream to write run records after last event
  *
  * @author  M.Frank
  * @version 1.0
  */
class RunRecordStream : public OutputStream     {
public:
  /// Standard algorithm Constructor
  RunRecordStream(const std::string& nam, ISvcLocator* svc) : OutputStream(nam,svc) {}
  /// Standard Destructor
  virtual ~RunRecordStream() {}
  /// Algorithm overload: finalization
  virtual StatusCode finalize();
  /// Runrecords do not get written for each event: Event processing hence dummy....
  virtual StatusCode execute() {  return StatusCode::SUCCESS; }
};

#endif // GAUDISVC_PERSISTENCYSVC_OUPUTFSRSTREAM_H
