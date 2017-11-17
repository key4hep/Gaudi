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
class RecordStream : public OutputStream
{
public:
  /// Standard algorithm Constructor
  RecordStream( const std::string&, ISvcLocator* );
  /// Runrecords do not get written for each event: Event processing hence dummy....
  StatusCode execute() override { return StatusCode::SUCCESS; }
  /// Algorithm overload: finalization
  StatusCode finalize() override;
};

#endif // GAUDISVC_PERSISTENCYSVC_RECORDSTREAM_H
