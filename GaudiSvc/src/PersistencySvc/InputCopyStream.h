// $Id: InputCopyStream.h,v 1.2 2006/11/30 14:57:04 mato Exp $
#ifndef GAUDISVC_PERSISTENCYSVC_INPUTCOPYSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_INPUTCOPYSTREAM_H


// Required for inheritance
#include "OutputStream.h"

/** A small to stream Data I/O.
    Author:  M.Frank
    Version: 1.0
*/
class InputCopyStream : public OutputStream     {
  friend class AlgFactory<InputCopyStream>;
  friend class Factory<InputCopyStream,IAlgorithm* (std::string,ISvcLocator *)>;

public:
	/// Standard algorithm Constructor
	InputCopyStream(const std::string& name, ISvcLocator* pSvcLocator); 
  /// Standard Destructor
  virtual ~InputCopyStream();
  /// Collect leaves from input file
  virtual StatusCode collectLeaves(IRegistry* dir, int level);
  /// Collect all objects to be written tio the output stream
  virtual StatusCode collectObjects();

private:
  /// Allow optional items to be on TES instead of input file
  bool m_takeOptionalFromTES;
};

#endif // GAUDISVC_PERSISTENCYSVC_INPUTCOPYSTREAM_H
