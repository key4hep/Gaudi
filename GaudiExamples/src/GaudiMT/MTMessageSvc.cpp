// $Id: MTMessageSvc.cpp,v 1.4 2006/04/20 16:34:57 hmd Exp $

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Message.h"
#include "MTMessageSvc.h"

#include <sstream>
#include <iostream>

#include <pthread.h>
extern pthread_mutex_t coutmutex;

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(MTMessageSvc)

// Constructor
MTMessageSvc::MTMessageSvc( const std::string& name, ISvcLocator* svcloc )
  : base_class( name, svcloc ) {
  m_defaultStream = &std::cout;
  m_outputLevel   = MSG::NIL;
  declareProperty( "Format",      m_defaultFormat = "% F%18W%S%7W%R%T %0W%M");

  // Special properties to control output level of individual sources
  declareProperty( "setVerbose",  m_thresholdProp[MSG::VERBOSE] );
  declareProperty( "setDebug",    m_thresholdProp[MSG::DEBUG] );
  declareProperty( "setInfo",     m_thresholdProp[MSG::INFO] );
  declareProperty( "setWarning",  m_thresholdProp[MSG::WARNING] );
  declareProperty( "setError",    m_thresholdProp[MSG::ERROR] );
  declareProperty( "setFatal",    m_thresholdProp[MSG::FATAL] );
  declareProperty( "setAlways",   m_thresholdProp[MSG::ALWAYS] );
}


/// Initialize Service
StatusCode MTMessageSvc::initialize() {
  StatusCode sc;
  sc = Service::initialize();
  if( sc.isFailure() ) return sc;
  // Release pointer to myself done in Service base class
  if( m_messageSvc ) {
    m_messageSvc->release();
    m_messageSvc = 0;
  }
  // Set my own properties
  setProperties();

  // Set the output level thresholds from properties
  for ( int lvl = MSG::VERBOSE; lvl < MSG::NUM_LEVELS; lvl++ ) {
    std::vector<std::string>& thresholds = m_thresholdProp[lvl];
    for ( std::vector<std::string>::iterator it = thresholds.begin();
          it != thresholds.end(); it++) {
      setOutputLevel( *it, lvl );
    }
  }
  return StatusCode::SUCCESS;
}
/// Finalize Service
StatusCode MTMessageSvc::finalize() {
  return StatusCode::SUCCESS;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: reportMessage
// Purpose: dispatches a message to the relevant streams.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::reportMessage( const Message& msg )    {
  int key = msg.getType();
  StreamMap::const_iterator first = m_streamMap.lower_bound( key );
  if ( first != m_streamMap.end() ) {
    StreamMap::const_iterator last = m_streamMap.upper_bound( key );
    while( first != last ) {
      std::ostream& stream = *( (*first).second.second );
      stream << msg << std::endl;
      first++;
    }
  }
  else if ( key >= outputLevel(msg.getSource()) )   {
    msg.setFormat(m_defaultFormat);
    pthread_mutex_lock(&coutmutex);
    (*m_defaultStream) << msg << std::endl << std::flush;
    pthread_mutex_unlock(&coutmutex);
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: reportMessage
// Purpose: dispatches a message to the relevant streams.
// ---------------------------------------------------------------------------
//
void MTMessageSvc::reportMessage (const char* source,
                                int type,
                                const char* message) {
  Message msg( source, type, message);
  reportMessage( msg );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: reportMessage
// Purpose: dispatches a message to the relevant streams.
// ---------------------------------------------------------------------------
//
void MTMessageSvc::reportMessage (const std::string& source,
                                int type,
                                const std::string& message) {
  Message msg( source, type, message);
  reportMessage( msg );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: sendMessage
// Purpose: finds a message for a given status code and dispatches it.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::reportMessage (const StatusCode& key,
                                const std::string& source)
{
  MessageMap::const_iterator first = m_messageMap.lower_bound( key );
  if ( first != m_messageMap.end() ) {
    MessageMap::const_iterator last = m_messageMap.upper_bound( key );
    while( first != last ) {
      Message msg = (*first).second;
      msg.setSource( source );
      std::ostringstream os1;
      os1 << "Status Code " << key.getCode() << std::ends;
      Message stat_code1( source, msg.getType(), os1.str() );
      reportMessage( stat_code1 );
      reportMessage( msg );
      first++;
    }
  }
  else {
    Message mesg = m_defaultMessage;
    mesg.setSource( source );
    std::ostringstream os2;
    os2 << "Status Code " << key.getCode() << std::ends;
    Message stat_code2( source,  mesg.getType(), os2.str() );
    reportMessage( stat_code2 );
    reportMessage( mesg );
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: insertStream
// Purpose: inserts a stream for a message type.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::insertStream (int key,
                               const std::string& name,
                               std::ostream *stream)
{
  typedef StreamMap::value_type value_type;
  m_streamMap.insert( value_type( key, NamedStream(name,stream) ) );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseStream
// Purpose: erases all the streams for all the message types.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseStream()
{
  m_streamMap.erase( m_streamMap.begin(), m_streamMap.end() );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseStream
// Purpose: erases all the streams for a message type.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseStream( int message_type )
{
  m_streamMap.erase( message_type );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseStream
// Purpose: erases one stream for a message type.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseStream( int key, std::ostream* stream )   {
  if ( 0 != stream )    {
    bool changed = true;
    while( changed ) {
      changed = false;
      StreamMap::iterator first = m_streamMap.lower_bound( key );
      StreamMap::iterator last = m_streamMap.upper_bound( key );
      while( first != last ) {
        if ( (*first).second.second == stream ) {
          m_streamMap.erase( first );
          changed = true;
          break;
        }
      }
    }
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseStream
// Purpose: erases one stream for all message types.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseStream( std::ostream* stream )    {
  if ( 0 != stream )    {
    bool changed = true;
    while( changed ) {
      changed = false;
      StreamMap::iterator first = m_streamMap.begin();
      while( first != m_streamMap.end() ) {
        if ( (*first).second.second == stream ) {
          m_streamMap.erase( first );
          changed = true;
          break;
        }
      }
    }
  }
}


//#############################################################################
// ---------------------------------------------------------------------------
// Routine: insertMessage
// Purpose: inserts a message for a status code.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::insertMessage( const StatusCode& key, const Message& msg )
{
  typedef MessageMap::value_type value_type;
  m_messageMap.insert( value_type( key, msg ) );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseMessage
// Purpose: erases all the messages for all the status codes.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseMessage()
{
  m_messageMap.erase( m_messageMap.begin(), m_messageMap.end() );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseMessage
// Purpose: erases all the messages for a status code.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseMessage( const StatusCode& key )
{
  m_messageMap.erase( key );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseMessage
// Purpose: erases one message for a status code.
// ---------------------------------------------------------------------------
//

void MTMessageSvc::eraseMessage( const StatusCode& key, const Message& msg )
{
  bool changed = true;
  while( changed ) {
    changed = false;
    MessageMap::iterator first = m_messageMap.lower_bound( key );
    MessageMap::iterator last = m_messageMap.upper_bound( key );
    while( first != last ) {
      const Message& message = (*first).second;
      if ( message == msg ) {
        m_messageMap.erase( first );
        changed = true;
        break;
      }
    }
  }
}

// ---------------------------------------------------------------------------
int MTMessageSvc::outputLevel()   const {
// ---------------------------------------------------------------------------
  return m_outputLevel;
}
// ---------------------------------------------------------------------------
int MTMessageSvc::outputLevel( const std::string& source )   const {
// ---------------------------------------------------------------------------
  ThresholdMap::const_iterator it;

  it = m_thresholdMap.find( source );
  if( it != m_thresholdMap.end() ) {
    return (*it).second;
  }
  else {
    return m_outputLevel;
  }
}

// ---------------------------------------------------------------------------
void MTMessageSvc::setOutputLevel(int new_level)    {
// ---------------------------------------------------------------------------
  m_outputLevel = new_level;
}

// ---------------------------------------------------------------------------
void MTMessageSvc::setOutputLevel(const std::string& source, int level)    {
// ---------------------------------------------------------------------------
  pthread_mutex_lock(&coutmutex);
  std::pair<ThresholdMap::iterator, bool> p;
  p = m_thresholdMap.insert(ThresholdMap::value_type( source, level) );
  if( p.second == false ) {
    // Already esisting an output level for that source. Erase an enter it again
    m_thresholdMap.erase ( p.first );
    m_thresholdMap.insert(ThresholdMap::value_type( source, level) );
  }
  pthread_mutex_unlock(&coutmutex);
}

