#ifdef _WIN32
// Avoid conflicts between windows and the message service.
#define NOMSG
#define NOGDI
#endif

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Message.h"
#include "GaudiKernel/xtoa.h"
#include "MessageSvc.h"

#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(MessageSvc)

static std::string levelNames[MSG::NUM_LEVELS];

// Constructor
MessageSvc::MessageSvc( const std::string& name, ISvcLocator* svcloc )
  : base_class( name, svcloc ) {
  m_defaultStream = &std::cout;
  m_outputLevel   = MSG::NIL;
  declareProperty( "Format",      m_defaultFormat = Message::getDefaultFormat() );
  declareProperty( "timeFormat",  m_defaultTimeFormat = Message::getDefaultTimeFormat() );
  declareProperty( "showStats",   m_stats = false );
  declareProperty( "statLevel",   m_statLevel = 0 );

  // Special properties to control output level of individual sources
  declareProperty( "setVerbose",  m_thresholdProp[MSG::VERBOSE] );
  declareProperty( "setDebug",    m_thresholdProp[MSG::DEBUG] );
  declareProperty( "setInfo",     m_thresholdProp[MSG::INFO] );
  declareProperty( "setWarning",  m_thresholdProp[MSG::WARNING] );
  declareProperty( "setError",    m_thresholdProp[MSG::ERROR] );
  declareProperty( "setFatal",    m_thresholdProp[MSG::FATAL] );
  declareProperty( "setAlways",   m_thresholdProp[MSG::ALWAYS] );

  declareProperty( "useColors",        m_color=false);
  m_color.declareUpdateHandler(&MessageSvc::initColors, this);

  declareProperty( "fatalColorCode",   m_logColors[MSG::FATAL] );
  declareProperty( "errorColorCode",   m_logColors[MSG::ERROR] );
  declareProperty( "warningColorCode", m_logColors[MSG::WARNING] );
  declareProperty( "infoColorCode",    m_logColors[MSG::INFO] );
  declareProperty( "debugColorCode",   m_logColors[MSG::DEBUG] );
  declareProperty( "verboseColorCode", m_logColors[MSG::VERBOSE] );
  declareProperty( "alwaysColorCode",  m_logColors[MSG::ALWAYS] );

  const int defaultLimit = 500;
  declareProperty( "fatalLimit",    m_msgLimit[MSG::FATAL]   = defaultLimit );
  declareProperty( "errorLimit",    m_msgLimit[MSG::ERROR]   = defaultLimit );
  declareProperty( "warningLimit",  m_msgLimit[MSG::WARNING] = defaultLimit );
  declareProperty( "infoLimit",     m_msgLimit[MSG::INFO]    = defaultLimit );
  declareProperty( "debugLimit",    m_msgLimit[MSG::DEBUG]   = defaultLimit );
  declareProperty( "verboseLimit",  m_msgLimit[MSG::VERBOSE] = defaultLimit );
  declareProperty( "alwaysLimit",   m_msgLimit[MSG::ALWAYS]  = 0 );

  declareProperty( "defaultLimit",  m_msgLimit[MSG::NIL]     = defaultLimit );

  declareProperty( "enableSuppression", m_suppress = false );
  declareProperty( "countInactive", m_inactCount = false )->declareUpdateHandler( &MessageSvc::setupInactCount, this );
#ifndef NDEBUG
  // initialize the MsgStream static flag.
  MsgStream::enableCountInactive(m_inactCount);
#endif

  declareProperty( "loggedStreams",
                   m_loggedStreamsName,
                   "MessageStream sources we want to dump into a logfile" );

  for (int ic=0; ic<MSG::NUM_LEVELS; ++ic) {
    m_logColors[ic].declareUpdateHandler(&MessageSvc::setupColors, this);
    m_msgLimit[ic].declareUpdateHandler(&MessageSvc::setupLimits, this);
    m_thresholdProp[ic].declareUpdateHandler(&MessageSvc::setupThreshold, this);
  }

  levelNames[0] = "NIL";
  levelNames[1] = "VERBOSE";
  levelNames[2] = "DEBUG";
  levelNames[3] = "INFO";
  levelNames[4] = "WARNING";
  levelNames[5] = "ERROR";
  levelNames[6] = "FATAL";
  levelNames[7] = "ALWAYS";

  for (int i=0; i<MSG::NUM_LEVELS; ++i) {
      m_msgCount[i] = 0;
  }

}

//#############################################################################

MessageSvc::~MessageSvc()
{
  // closing log-files, if any
  LoggedStreamsMap_t::iterator iStream   = m_loggedStreams.begin();
  LoggedStreamsMap_t::iterator endStream = m_loggedStreams.end();
  for ( ; iStream != endStream; ++iStream ) {
    delete iStream->second;
    iStream->second = 0;
  }
}
//#############################################################################


/// Initialize Service
StatusCode MessageSvc::initialize() {
  StatusCode sc;
  sc = Service::initialize();
  if( sc.isFailure() ) return sc;
  // Release pointer to myself done in Service base class
  //if( m_msgsvc.isValid() ) {
  //  m_msgsvc = 0;
  //}
  // Set my own properties
  sc = setProperties();
  if (sc.isFailure()) return sc;

#ifdef _WIN32
  m_color = false;
#endif

  m_colMap["black"]  = MSG::BLACK;
  m_colMap["red"]    = MSG::RED;
  m_colMap["green"]  = MSG::GREEN;
  m_colMap["yellow"] = MSG::YELLOW;
  m_colMap["blue"]   = MSG::BLUE;
  m_colMap["purple"] = MSG::PURPLE;
  m_colMap["cyan"]   = MSG::CYAN;
  m_colMap["white"]  = MSG::WHITE;

  // make sure the map of logged stream names is initialized
  setupLogStreams();

  return StatusCode::SUCCESS;
}

//#############################################################################

/// Reinitialize Service
StatusCode MessageSvc::reinitialize() {
  m_state = Gaudi::StateMachine::OFFLINE;
  return initialize();
}

//#############################################################################

void MessageSvc::initColors(Property& /*prop*/) {

  if (m_color == true) {

    if (m_logColors[MSG::FATAL].value().size() == 0) {
      vector<string> fatDef;
      fatDef.push_back( "[94;101;1m" );
      m_logColors[MSG::FATAL].set( fatDef );
    } else {
      MessageSvc::setupColors( m_logColors[MSG::FATAL] );
    }

    if (m_logColors[MSG::ERROR].value().size() == 0) {
      vector<string> errDef;
      errDef.push_back( "[97;101;1m" );
      m_logColors[MSG::ERROR].set( errDef );
    } else {
      MessageSvc::setupColors( m_logColors[MSG::ERROR] );
    }

    if (m_logColors[MSG::WARNING].value().size() == 0) {
      vector<string> warDef;
      warDef.push_back( "[93;1m" );
      m_logColors[MSG::WARNING].set( warDef );
    } else {
      MessageSvc::setupColors( m_logColors[MSG::WARNING] );
    }

  } else {

    // reset all color codes;
    for (int ic=0; ic<MSG::NUM_LEVELS; ++ic) {
      vector<string> def;
      m_logColors[ic].set( def );
    }

  }

}

//#############################################################################

void MessageSvc::setupColors(Property& prop) {

  if (! m_color) return;

  int ic;
  if (prop.name() == "fatalColorCode") {
    ic = MSG::FATAL;
  } else if (prop.name() == "errorColorCode") {
    ic = MSG::ERROR;
  } else if (prop.name() == "warningColorCode") {
    ic = MSG::WARNING;
  } else if (prop.name() == "infoColorCode") {
    ic = MSG::INFO;
  } else if (prop.name() == "debugColorCode") {
    ic = MSG::DEBUG;
  } else if (prop.name() == "verboseColorCode") {
    ic = MSG::VERBOSE;
  } else if (prop.name() == "alwaysColorCode") {
    ic = MSG::ALWAYS;
  } else {
    cout << "ERROR: Unknown message color parameter: " << prop.name()
         << endl;
    return;
  }

  string code;
  vector<string>::const_iterator itr;
  itr = m_logColors[ic].value().begin();

  if ( m_logColors[ic].value().size() == 1 ) {

    if (*itr == "") {
      code = "";
    } else if (itr->substr(0,1) == "[") {
      code = "\033" + *itr;
    } else {
      code = "\033[" + colTrans(*itr, 90) + ";1m";
    }

  } else if (m_logColors[ic].value().size() == 2) {
    vector<string>::const_iterator itr2 = itr + 1;

    code =  "\033[" + colTrans(*itr, 90) + ";"
      + colTrans(*itr2, 100) + ";1m";

  }

  m_logColorCodes[ic] = code;

}
//#############################################################################

void MessageSvc::setupLimits(Property& prop) {
  // Just report problems in the settings of the limits and unknown limit parameters
  if (prop.name() == "alwaysLimit") {
    IntegerProperty *p = dynamic_cast<IntegerProperty*>(&prop);
    if (p && p->value() != 0) {
      cout << "MessageSvc ERROR: cannot suppress ALWAYS messages" << endl;
      p->setValue(0);
    }
  } else if (prop.name() == "defaultLimit") {
    for (int i = MSG::VERBOSE; i< MSG::NUM_LEVELS; ++i) {
      if (i != MSG::ALWAYS) {
        m_msgLimit[i] = m_msgLimit[MSG::NIL].value();
      }
    }
  } else if (prop.name() != "fatalLimit" &&
             prop.name() != "errorLimit" &&
             prop.name() != "warningLimit" &&
             prop.name() == "infoLimit" &&
             prop.name() == "debugLimit" &&
             prop.name() == "verboseLimit") {
    cout << "MessageSvc ERROR: Unknown message limit parameter: "
         << prop.name() << endl;
    return;
  }
}
//#############################################################################

void MessageSvc::setupThreshold(Property& prop) {

  int ic = 0;
  if (prop.name() == "setFatal") {
    ic = MSG::FATAL;
  } else if (prop.name() == "setError") {
    ic = MSG::ERROR;
  } else if (prop.name() == "setWarning") {
    ic = MSG::WARNING;
  } else if (prop.name() == "setInfo") {
    ic = MSG::INFO;
  } else if (prop.name() == "setDebug") {
    ic = MSG::DEBUG;
  } else if (prop.name() == "setVerbose") {
    ic = MSG::VERBOSE;
  } else if (prop.name() == "setAlways") {
    ic = MSG::ALWAYS;
  } else {
    cerr << "MessageSvc ERROR: Unknown message threshold parameter: "
         << prop.name() << endl;
    return;
  }

  StringArrayProperty *sap = dynamic_cast<StringArrayProperty*>( &prop);
  if (sap == 0) {
    std::cerr << "could not dcast " << prop.name()
              << " to a StringArrayProperty (which it should be!)" << endl;
    return;
  } else {
    std::vector<std::string>::const_iterator itr;
    for ( itr =  sap->value().begin();
          itr != sap->value().end();
          ++itr) {
      setOutputLevel( *itr, ic );
    }
  }

}

//#############################################################################

#ifdef NDEBUG
void MessageSvc::setupInactCount(Property&) {}
#else
void MessageSvc::setupInactCount(Property&prop) {
  if (prop.name() == "countInactive") {
    BooleanProperty *p = dynamic_cast<BooleanProperty*>(&prop);
    if (p)
      MsgStream::enableCountInactive(p->value());
  }
}
#endif


//#############################################################################
/// Finalize Service
StatusCode MessageSvc::finalize() {

  m_suppress = false;

  {
    std::ostringstream os;

    if (m_stats) {
      os << "Summarizing all message counts" << endl;
    } else {
      os << "Listing sources of suppressed message: " << endl;
    }

    os << "=====================================================" << endl;
    os << " Message Source              |   Level |    Count" << endl;
    os << "-----------------------------+---------+-------------" << endl;


    bool found(false);

    std::map<std::string,MsgAry>::const_iterator itr;
    for (itr=m_sourceMap.begin(); itr!=m_sourceMap.end(); ++itr) {
      for (unsigned int ic = 0; ic < MSG::NUM_LEVELS; ++ic) {
        if ( (itr->second.msg[ic] >= m_msgLimit[ic] && m_msgLimit[ic] != 0 ) ||
            (m_stats && itr->second.msg[ic] > 0 && ic >= m_statLevel.value()) ) {
          os << " ";
          os.width(28);
          os.setf(ios_base::left,ios_base::adjustfield);
          os << itr->first;

          os << "|";

          os.width(8);
          os.setf(ios_base::right,ios_base::adjustfield);
          os << levelNames[ic];

          os << " |";

          os.width(9);
          os << itr->second.msg[ic];

          os << endl;

          found = true;
        }
      }
    }
    os << "=====================================================" << endl;

    if (found || m_stats) {
      cout << os.str();
    }
  }

#ifndef NDEBUG
  if (m_inactCount.value()) {

    std::ostringstream os;
    os << "Listing sources of Unprotected and Unseen messages\n";

    bool found(false);

    unsigned int ml(0);
    std::map<std::string,MsgAry>::const_iterator itr;
    for (itr=m_inactiveMap.begin(); itr!=m_inactiveMap.end(); ++itr) {
      for (unsigned int ic = 0; ic < MSG::NUM_LEVELS; ++ic) {
	if (itr->second.msg[ic] != 0) {
	  if (itr->first.length() > ml) { ml = itr->first.length(); }
	}
      }
    }

    for (unsigned int i=0; i<ml+25; ++i) {
      os << "=";
    }

    os << endl << " ";
    os.width(ml+2);
    os.setf(ios_base::left,ios_base::adjustfield);
    os << "Message Source";
    os.width(1);
    os << "|   Level |    Count" << endl;

    for (unsigned int i=0; i<ml+3; ++i) {
      os << "-";
    }
    os << "+---------+-----------" << endl;


    for (itr=m_inactiveMap.begin(); itr!=m_inactiveMap.end(); ++itr) {
      for (unsigned int ic = 0; ic < MSG::NUM_LEVELS; ++ic) {
	if (itr->second.msg[ic] != 0) {
	  os << " ";
	  os.width(ml+2);
	  os.setf(ios_base::left,ios_base::adjustfield);
	  os << itr->first;

	  os << "|";

	  os.width(8);
	  os.setf(ios_base::right,ios_base::adjustfield);
	  os << levelNames[ic];

	  os << " |";

	  os.width(9);
	  os << itr->second.msg[ic];

	  os << endl;

	  found = true;
	}
      }
    }
    for (unsigned int i=0; i<ml+25; ++i) {
      os << "=";
    }
    os << endl;

    if (found) {
      cout << os.str();
    }
  }
#endif

  return StatusCode::SUCCESS;
}

//#############################################################################
std::string MessageSvc::colTrans(std::string col, int offset) {
  ColorMap::const_iterator itr = m_colMap.find(col);
  int icol;
  if (itr != m_colMap.end()) {
    icol = offset + itr->second;
  } else {
    icol = offset + 8;
  }
  std::ostringstream os1;

  os1 << icol;

  return os1.str();

}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: reportMessage
// Purpose: dispatches a message to the relevant streams.
// ---------------------------------------------------------------------------
//

void MessageSvc::reportMessage( const Message& msg, int outputLevel )    {
  boost::recursive_mutex::scoped_lock lock(m_reportMutex);

  int key = msg.getType();

  m_msgCount[key] ++;

  const Message *cmsg = &msg;

  // processing logged streams
  if ( !m_loggedStreams.empty() ) {
    const LoggedStreamsMap_t::iterator iLog = m_loggedStreams.find( msg.getSource() );
    if ( m_loggedStreams.end() != iLog ) {
      (*iLog->second) << *cmsg << std::endl;
    }
  }

  if ( m_suppress.value() || m_stats.value() ) {

    // Increase the counter of 'key' type of messages for the source and
    // get the new value.
    const int nmsg = ++(m_sourceMap[msg.getSource()].msg[key]);

    if (m_suppress.value()) {

      if ( m_msgLimit[key] != 0 ) {
        if (nmsg == m_msgLimit[key]) {
          std::ostringstream txt;
          txt << levelNames[key] << " message limit ("
              << m_msgLimit[key].value()
              << ") reached for "
              << msg.getSource() + ". Suppressing further output.";
          cmsg = new Message(msg.getSource(), MSG::WARNING, txt.str());
          cmsg->setFormat(msg.getFormat());
        } else if (nmsg > m_msgLimit[key]) {
          return;
        }
      }
    }

  }

  StreamMap::const_iterator first = m_streamMap.lower_bound( key );
  if ( first != m_streamMap.end() ) {
    StreamMap::const_iterator last = m_streamMap.upper_bound( key );
    while( first != last ) {
      std::ostream& stream = *( (*first).second.second );
      stream << *cmsg << std::endl;
      first++;
    }
  }
  else if ( key >= outputLevel )   {
    msg.setFormat(m_defaultFormat);
    msg.setTimeFormat(m_defaultTimeFormat);
    if (!m_color) {
      (*m_defaultStream) << *cmsg << std::endl << std::flush;
    } else {
      (*m_defaultStream) <<  m_logColorCodes[key] << *cmsg << "\033[m"
                         << std::endl << std::flush;
    }
  }

  if (cmsg != &msg) { delete cmsg; }

}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: reportMessage
// Purpose: dispatches a message to the relevant streams.
// ---------------------------------------------------------------------------
//
void MessageSvc::reportMessage( const Message& msg )    {
  reportMessage(msg, outputLevel(msg.getSource()));
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: reportMessage
// Purpose: dispatches a message to the relevant streams.
// ---------------------------------------------------------------------------
//
void MessageSvc::reportMessage (const char* source,
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
void MessageSvc::reportMessage (const std::string& source,
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

void MessageSvc::reportMessage (const StatusCode& key,
                                const std::string& source)
{
  boost::recursive_mutex::scoped_lock lock(m_messageMapMutex);

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

void MessageSvc::insertStream (int key,
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

void MessageSvc::eraseStream()
{
  m_streamMap.erase( m_streamMap.begin(), m_streamMap.end() );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseStream
// Purpose: erases all the streams for a message type.
// ---------------------------------------------------------------------------
//

void MessageSvc::eraseStream( int message_type )
{
  m_streamMap.erase( message_type );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseStream
// Purpose: erases one stream for a message type.
// ---------------------------------------------------------------------------
//

void MessageSvc::eraseStream( int key, std::ostream* stream )   {
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

void MessageSvc::eraseStream( std::ostream* stream )    {
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

void MessageSvc::insertMessage( const StatusCode& key, const Message& msg )
{
  boost::recursive_mutex::scoped_lock lock(m_messageMapMutex);

  typedef MessageMap::value_type value_type;
  m_messageMap.insert( value_type( key, msg ) );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseMessage
// Purpose: erases all the messages for all the status codes.
// ---------------------------------------------------------------------------
//

void MessageSvc::eraseMessage()
{
  boost::recursive_mutex::scoped_lock lock(m_messageMapMutex);

  m_messageMap.erase( m_messageMap.begin(), m_messageMap.end() );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseMessage
// Purpose: erases all the messages for a status code.
// ---------------------------------------------------------------------------
//

void MessageSvc::eraseMessage( const StatusCode& key )
{
  boost::recursive_mutex::scoped_lock lock(m_messageMapMutex);

  m_messageMap.erase( key );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: eraseMessage
// Purpose: erases one message for a status code.
// ---------------------------------------------------------------------------
//

void MessageSvc::eraseMessage( const StatusCode& key, const Message& msg )
{
  boost::recursive_mutex::scoped_lock lock(m_messageMapMutex);

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
int MessageSvc::outputLevel()   const {
// ---------------------------------------------------------------------------
  return m_outputLevel;
}

// ---------------------------------------------------------------------------
int MessageSvc::outputLevel( const std::string& source )   const {
// ---------------------------------------------------------------------------
  boost::recursive_mutex::scoped_lock lock(m_thresholdMapMutex);

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
void MessageSvc::setOutputLevel(int new_level)    {
// ---------------------------------------------------------------------------
  m_outputLevel = new_level;
}

// ---------------------------------------------------------------------------
void MessageSvc::setOutputLevel(const std::string& source, int level)    {
// ---------------------------------------------------------------------------
  boost::recursive_mutex::scoped_lock lock(m_thresholdMapMutex);

  /*
  std::pair<ThresholdMap::iterator, bool> p;
  p = m_thresholdMap.insert(ThresholdMap::value_type( source, level) );
  if( p.second == false ) {
    // Already esisting an output level for that source. Erase and enter it again
    m_thresholdMap.erase ( p.first );
    m_thresholdMap.insert(ThresholdMap::value_type( source, level) );
  }
  */
  m_thresholdMap[source] = level;
}

// ---------------------------------------------------------------------------
std::string MessageSvc::getLogColor(int logLevel) const   {
// ---------------------------------------------------------------------------
  if (logLevel < MSG::NUM_LEVELS) {
    return m_logColorCodes[logLevel];
  } else {
    return "";
  }
}

// ---------------------------------------------------------------------------
int MessageSvc::messageCount( MSG::Level level) const   {

  return m_msgCount[level];

}

// ---------------------------------------------------------------------------
void
MessageSvc::incrInactiveCount(MSG::Level level, const std::string& source) {

  ++(m_inactiveMap[source].msg[level]);

}

// ---------------------------------------------------------------------------
void MessageSvc::setupLogStreams()
{
  // reset state
  for ( LoggedStreamsMap_t::iterator iLog = m_loggedStreams.begin();
        iLog != m_loggedStreams.end();
        ++iLog ) {
    delete iLog->second;
  }
  m_loggedStreams.clear();

  typedef std::map<std::string,std::string> StreamMap_t;
  const StreamMap_t& streamMap = m_loggedStreamsName;
  typedef StreamMap_t::const_iterator StreamMapIter;

  for ( StreamMapIter iProp = streamMap.begin(), iEnd = streamMap.end();
        iProp != iEnd;
        ++iProp ) {

    const std::string sourceName  = iProp->first;
    const std::string outFileName = iProp->second;

    std::set<std::string> outFileNames;
    for ( StreamMapIter jProp = streamMap.begin();
          jProp != iEnd;
          ++jProp ) {
      if ( jProp->first != iProp->first ) {
        outFileNames.insert( jProp->second );
      }
    }

    tee( sourceName, outFileName, outFileNames );

  }//> loop over property entries

  return;
}

// ---------------------------------------------------------------------------
void MessageSvc::tee( const std::string& sourceName,
                      const std::string& outFileName,
                      const std::set<std::string>& outFileNames )
{
  const std::ios_base::openmode openMode = std::ios_base::out |
                                           std::ios_base::trunc;

  LoggedStreamsMap_t::iterator iEnd    = m_loggedStreams.end();
  LoggedStreamsMap_t::iterator iStream = m_loggedStreams.find( sourceName );
  if ( iStream != iEnd ) {
    delete iStream->second;
    iStream->second = 0;
    m_loggedStreams.erase( iStream );
  }

  // before creating a new ofstream, make sure there is no already existing
  // one with the same file name...
  iEnd = m_loggedStreams.end();
  for ( iStream = m_loggedStreams.begin(); iStream != iEnd; ++iStream ) {
    if ( outFileNames.find( outFileName ) != outFileNames.end() ) {
      m_loggedStreams[sourceName] = m_loggedStreams[iStream->first];
      return;
    }
  }

  std::ofstream * out =  new std::ofstream( outFileName.c_str(), openMode );

  if ( !out->good() ) {
    out->close();
    delete out;
    return;
  }

  m_loggedStreams[sourceName] = out;

  return;
}

