#ifndef GAUDIKERNEL_DATASTREAMTOOL_H
#define GAUDIKERNEL_DATASTREAMTOOL_H 1

// Include files

#include <vector>

// from Gaudi
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataStreamTool.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Service.h"

class IIncidentSvc;

/** @class DataStreamTool DataStreamTool.h GaudiKernel/DataStreamTool.h
 *
 *
 *  @author Andres Felipe Osorio Oliveros
 *  @author Marco Clemencic
 *
 *
 *  @date   2006-09-21
 *
 */

class GAUDI_API DataStreamTool : public extends<AlgTool, IDataStreamTool>
{
public:
  typedef std::vector<EventSelectorDataStream*> Streams;
  typedef std::vector<Gaudi::Property<std::string>> Properties;

  /// Standard constructor
  using extends::extends;

  StatusCode initialize() override;

  StatusCode finalize() override;

  /// Initialize newly opened stream
  StatusCode initializeStream( EventSelectorDataStream* ) override;

  /// Finalize no longer needed stream
  StatusCode finalizeStream( EventSelectorDataStream* ) override;

  StatusCode getNextStream( const EventSelectorDataStream*&, size_type& ) override;

  StatusCode getPreviousStream( const EventSelectorDataStream*&, size_type& ) override;

  StatusCode addStream( const std::string& ) override;

  StatusCode addStreams( const StreamSpecs& ) override;

  StatusCode eraseStream( const std::string& ) override;
  /// Retrieve stream by name
  EventSelectorDataStream* getStream( const std::string& ) override;

  EventSelectorDataStream* getStream( size_type ) override;

  EventSelectorDataStream* lastStream() override;

  size_type size() override { return m_streams.size(); };

  StatusCode clear() override;

  virtual Streams& getStreams() { return m_streams; };

  virtual Streams::iterator beginOfStreams() { return m_streams.begin(); };

  virtual Streams::iterator endOfStreams() { return m_streams.end(); };

protected:
  StatusCode createSelector( const std::string&, const std::string&, IEvtSelector*& ) override;

  StatusCode createStream( const std::string&, const std::string&, EventSelectorDataStream*& ) override;

  /// Connect single stream by reference
  StatusCode connectStream( EventSelectorDataStream* );
  /// Connect single stream by name
  StatusCode connectStream( const std::string& );

  size_type m_streamID = 0;

  size_type m_streamCount = 0;

  Streams m_streams;

  StreamSpecs m_streamSpecs;

  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc = nullptr;

  Streams::iterator getStreamIterator( const std::string& );

private:
  /// Fake copy constructor (never implemented).
  DataStreamTool( const DataStreamTool& ) = delete;
  /// Fake assignment operator (never implemented).
  DataStreamTool& operator=( const DataStreamTool& ) = delete;
};
#endif // GAUDIKERNEL_DATASTREAMTOOL_H
