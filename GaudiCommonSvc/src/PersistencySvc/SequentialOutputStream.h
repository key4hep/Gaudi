#ifndef GAUDISVC_PERSISTENCYSVC_SEQUENTIALOUTPUTSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_SEQUENTIALOUTPUTSTREAM_H

#include <string>
// Required for inheritance
#include "OutputStream.h"

/** @class SequentialOutputStream SequentialOutputStream.h
 * Extension of OutputStream to write run records after last event
 *
 * @author  M.Frank
 * @version 1.0
 */
class SequentialOutputStream : public OutputStream
{

protected:
  /// OutputStream override: Select the different objects and write them to file
  StatusCode writeObjects() override;

public:
  using OutputStream::OutputStream;

  /// Standard Destructor
  ~SequentialOutputStream() override = default;

  StatusCode execute() override;

private:
  Gaudi::Property<unsigned int> m_eventsPerFile{this, "EventsPerFile", std::numeric_limits<unsigned int>::max()};
  Gaudi::Property<bool>         m_numericFilename{this, "NumericFilename", false};
  Gaudi::Property<unsigned int> m_nNumbersAdded{this, "NumbersAdded", 6};

  // Data members
  unsigned int m_events = 0;
  unsigned int m_iFile  = 1;

  // Helper Methods
  void makeFilename();
};

#endif // GAUDISVC_PERSISTENCYSVC_SEQUENTIALOUTPUTSTREAM_H
