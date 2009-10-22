// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/GaudiMT/MTMessageSvc.h,v 1.4 2007/01/18 14:53:54 hmd Exp $
#ifndef GAUDI_MTMESSAGESVC_H
#define GAUDI_MTMESSAGESVC_H

// Include files
#include <string>
#include <map>

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/Message.h"

// Forward declarations
class ISvcLocator;

//
// ClassName:   MTMessageSvc
//
// Description: The MTMessageSvc service implements the IMessageSvc interface and provides the
//              basic messaging needed by batch oriented applications.
//
// Author:      Iain Last
//
class MTMessageSvc : public extends1<Service, IMessageSvc> {
public:
  typedef std::pair< std::string, std::ostream* > NamedStream;
  typedef std::multimap< int, NamedStream > StreamMap;
  typedef std::multimap< StatusCode, Message > MessageMap;
  typedef std::map< std::string, int > ThresholdMap;

  // Default constructor.
  MTMessageSvc( const std::string& name, ISvcLocator* svcloc );
  // Destructor.
  virtual ~MTMessageSvc() {}

  // Implementation of IService::initialize()
  virtual StatusCode initialize();
  // Implementation of IService::finalize()
  virtual StatusCode finalize();

  // Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage( const Message& message );

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

  /** Show whether colors are used
   */
  virtual bool useColor() const { return false; }

  /** Get the color codes for various log levels
      @param logLevel Logging level
   */
  virtual std::string getLogColor(int /* logLevel */) const {
    return std::string();
  }

  virtual int messageCount( MSG::Level /* level */ ) const { return 0; }

private:
  std::ostream* m_defaultStream;      ///< Pointer to the output stream.
  Message m_defaultMessage;           ///< Default Message
  StreamMap m_streamMap;              ///< Stream map
  MessageMap m_messageMap;            ///< Message map
  ThresholdMap m_thresholdMap;        ///< Output level threshold map
  std::string m_defaultFormat;        ///< Default format for the messages
  std::vector<std::string> m_thresholdProp[MSG::NUM_LEVELS]; ///< Properties controling
};

#endif
