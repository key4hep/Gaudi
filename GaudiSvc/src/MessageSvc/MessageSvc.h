#ifndef GAUDI_MESSAGESVC_H
#define GAUDI_MESSAGESVC_H

// Include files
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iosfwd>

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/Message.h"
#include "GaudiKernel/Property.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/array.hpp>

// Forward declarations
class ISvcLocator;

//
// ClassName:   MessageSvc
//
// Description: The MessageSvc service implements the IMessageSvc interface and provides the
//              basic messaging needed by batch oriented applications.
//
// Author:      Iain Last
//
class MessageSvc : public extends2<Service, IMessageSvc, IInactiveMessageCounter> {
public:
  typedef std::pair< std::string, std::ostream* > NamedStream;
  typedef std::multimap< int, NamedStream > StreamMap;
  typedef std::multimap< StatusCode, Message > MessageMap;
  typedef std::map< std::string, int > ThresholdMap;

  // Default constructor.
  MessageSvc( const std::string& name, ISvcLocator* svcloc );
  // Destructor.
  virtual ~MessageSvc();

  // Implementation of IService::reinitialize()
  virtual StatusCode reinitialize();
  // Implementation of IService::initialize()
  virtual StatusCode initialize();
  // Implementation of IService::finalize()
  virtual StatusCode finalize();

  // Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage( const Message& message );

  // Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage( const Message& msg, int outputLevel );

  // Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage( const StatusCode& code, const std::string& source = "");

  // Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage( const char* source, int type, const char* message);

  // Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage( const std::string& source, int type, const std::string& message);

  // Implementation of IMessageSvc::insertMessage()
  virtual void insertMessage( const StatusCode& code, const Message& message );

  // Implementation of IMessageSvc::eraseMessage()
  virtual void eraseMessage();

  // Implementation of IMessageSvc::eraseMessage()
  virtual void eraseMessage( const StatusCode& code ) ;

  // Implementation of IMessageSvc::eraseMessage()
  virtual void eraseMessage( const StatusCode& code, const Message& message );

  // Implementation of IMessageSvc::insertStream()
  virtual void insertStream( int message_type, const std::string& name, std::ostream* stream );

  // Implementation of IMessageSvc::eraseStream()
  virtual void eraseStream();

  // Implementation of IMessageSvc::eraseStream()
  virtual void eraseStream( int message_type );

  // Implementation of IMessageSvc::eraseStream()
  virtual void eraseStream( int message_type, std::ostream* stream );

  // Implementation of IMessageSvc::eraseStream()
  virtual void eraseStream( std::ostream* stream );

  // Implementation of IMessageSvc::desaultStream()
  virtual std::ostream* defaultStream() const {
    return m_defaultStream;
  }

  // Implementation of IMessageSvc::setDefaultStream()
  virtual void setDefaultStream( std::ostream* stream ) {
    boost::recursive_mutex::scoped_lock lock(m_reportMutex);
    m_defaultStream = stream;
  }

  // Implementation of IMessageSvc::ouputLevel()
  virtual int outputLevel()   const;

  // Implementation of IMessageSvc::ouputLevel()
  virtual int outputLevel(const std::string& source)   const;

  // Implementation of IMessageSvc::setOuputLevel()
  virtual void setOutputLevel(int new_level);

  // Implementation of IMessageSvc::setOuputLevel()
  virtual void setOutputLevel(const std::string& source, int new_level);

  // Implementation of IMessageSvc::useColor()
  virtual bool useColor() const { return m_color; }

  // Implementation of IMessageSvc::getLogColor()
  virtual std::string getLogColor(int logLevel) const;

  // Implementation of IMessageSvc::messageCount()
  virtual int messageCount( MSG::Level logLevel ) const;

  // Implementation of IInactiveMessageCounter::incrInactiveCount()
  virtual void incrInactiveCount( MSG::Level level,
				  const std::string& src );


private:
  std::ostream* m_defaultStream;      ///< Pointer to the output stream.
  Message m_defaultMessage;           ///< Default Message
  StreamMap m_streamMap;              ///< Stream map
  MessageMap m_messageMap;            ///< Message map
  ThresholdMap m_thresholdMap;        ///< Output level threshold map
  std::string m_defaultFormat;        ///< Default format for the messages
  std::string m_defaultTimeFormat;    ///< Default format for timestamps in the messages
  StringArrayProperty m_thresholdProp[MSG::NUM_LEVELS]; ///< Properties controlling
  BooleanProperty m_color;
  BooleanProperty m_stats;
  UnsignedIntegerProperty m_statLevel;
  StringArrayProperty m_logColors[MSG::NUM_LEVELS];
  IntegerProperty m_msgLimit[MSG::NUM_LEVELS];

  std::string m_logColorCodes[MSG::NUM_LEVELS];

  /// Private helper class to keep the count of messages of a type (MSG::LEVEL).
  struct MsgAry {
    /// Simple typedef for readability.
    typedef boost::array<int,MSG::NUM_LEVELS> ArrayType;
    /// Internal array of counters.
    ArrayType msg;
    /// Default constructor.
    MsgAry() {
      // This is a special hack to have a fast initialization of the array
      // because we cannot use initializer lists in the constructor (should be
      // possible in C++0X).
      static const ArrayType zero = {{0}};
      msg = zero;
    }
  };

  std::map<std::string,MsgAry> m_sourceMap, m_inactiveMap;
  BooleanProperty m_suppress, m_inactCount;

  std::string colTrans(std::string, int);
  typedef std::map<std::string, MSG::Color> ColorMap;
  ColorMap m_colMap;

  int m_msgCount[MSG::NUM_LEVELS];

  std::map<std::string, std::string> m_loggedStreamsName;
  typedef std::map<std::string, std::ostream*> LoggedStreamsMap_t;
  LoggedStreamsMap_t m_loggedStreams;

  void initColors(Property& prop);
  void setupColors(Property& prop);
  void setupLimits(Property& prop);
  void setupThreshold(Property& prop);
  void setupInactCount(Property& prop);

  void setupLogStreams();

  void tee( const std::string& sourceName, const std::string& logFileName,
	    const std::set<std::string>& declaredOutFileNames );

  /// Mutex to synchronize multiple threads printing.
  mutable boost::recursive_mutex m_reportMutex;

  /// Mutex to synchronize multiple access to m_messageMap.
  mutable boost::recursive_mutex m_messageMapMutex;

  /// Mutex to synchronize multiple access to m_thresholdMap
  /// (@see MsgStream::doOutput).
  mutable boost::recursive_mutex m_thresholdMapMutex;
};

#endif
