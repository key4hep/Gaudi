// $Id: RecordStream.h,v 1.1 2008/12/10 18:37:36 marcocle Exp $
#ifndef GAUDISVC_PERSISTENCYSVC_RECORDSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_RECORDSTREAM_H

// Required for inheritance
#include "OutputStream.h"

/** @class RecordStream RecordStream.h
  * Extension of OutputStream to write run records after last event
  *
  * @author  M.Frank
  * @version 1.0
  */
class RecordStream : public OutputStream     {
public:
  /// Standard algorithm Constructor
  RecordStream(const std::string&, ISvcLocator*);
  /// Standard Destructor
  virtual ~RecordStream() {}
  /// Algorithm overload: finalization
  virtual StatusCode finalize();
  /// Runrecords do not get written for each event: Event processing hence dummy....
  virtual StatusCode execute() {  return StatusCode::SUCCESS; }
};

#endif // GAUDISVC_PERSISTENCYSVC_RECORDSTREAM_H
