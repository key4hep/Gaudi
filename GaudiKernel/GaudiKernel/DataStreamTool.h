// $Id: DataStreamTool.h,v 1.3 2007/01/15 16:48:21 hmd Exp $
#ifndef GAUDIKERNEL_DATASTREAMTOOL_H
#define GAUDIKERNEL_DATASTREAMTOOL_H 1

// Include files

#include <vector>

// from Gaudi
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IDataStreamTool.h"
#include "GaudiKernel/AlgTool.h"

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


class GAUDI_API DataStreamTool: public extends1<AlgTool, IDataStreamTool> {
public:

  typedef std::vector<EventSelectorDataStream*>            Streams;
  typedef std::vector<StringProperty>            Properties;

  /// Standard constructor
  DataStreamTool( const std::string& type,
                  const std::string& name,
                  const IInterface* parent);

  virtual ~DataStreamTool( ); ///< Destructor

  virtual StatusCode initialize();

  virtual StatusCode addStream(const std::string &);

  virtual StatusCode addStreams(const StreamSpecs &);

  virtual StatusCode eraseStream( const std::string& );

  virtual StatusCode finalize();

  /// Initialize newly opened stream
  virtual StatusCode initializeStream( EventSelectorDataStream* );

  /// Finalize no longer needed stream
  virtual StatusCode finalizeStream( EventSelectorDataStream* );

  virtual StatusCode getNextStream( const EventSelectorDataStream* &, size_type & );

  virtual StatusCode getPreviousStream( const EventSelectorDataStream* &, size_type & );

  virtual Streams& getStreams() { return m_streams; };

  virtual EventSelectorDataStream* lastStream();

  virtual Streams::iterator beginOfStreams() {return m_streams.begin(); };

  virtual Streams::iterator endOfStreams() {return m_streams.end(); };

  /// Retrieve stream by name
  virtual EventSelectorDataStream* getStream( const std::string& );

  virtual EventSelectorDataStream* getStream( size_type );

  virtual size_type size() { return m_streams.size(); };

  virtual StatusCode clear();

protected:

  virtual StatusCode createSelector( const std::string& , const std::string& ,
                                     IEvtSelector*& );

  virtual StatusCode createStream(const std::string&, const std::string&,
                                  EventSelectorDataStream*& );

  /// Connect single stream by reference
  StatusCode connectStream( EventSelectorDataStream *);
  /// Connect single stream by name
  StatusCode connectStream( const std::string & );

  size_type m_streamID;

  size_type m_streamCount;

  Streams m_streams;

  StreamSpecs m_streamSpecs;

  /// Reference to the incident service
  SmartIF<IIncidentSvc>     m_incidentSvc;

  bool m_reconfigure;

  Streams::iterator getStreamIterator ( const std::string& );

private:
  /// Fake copy constructor (never implemented).
  DataStreamTool(const DataStreamTool&);
  /// Fake assignment operator (never implemented).
  DataStreamTool& operator= (const DataStreamTool&);

};
#endif // GAUDIKERNEL_DATASTREAMTOOL_H
