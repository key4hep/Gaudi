/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDI_MESSAGESVC_H
#define GAUDI_MESSAGESVC_H

// Include files
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/Message.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"
#include <Gaudi/Property.h>

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
class MessageSvc : public extends<Service, IMessageSvc, IInactiveMessageCounter> {
public:
  typedef std::pair<std::string, std::ostream*>   NamedStream;
  typedef std::multimap<int, NamedStream>         StreamMap;
  typedef std::multimap<StatusCode, Message>      MessageMap;
  typedef std::map<std::string, int, std::less<>> ThresholdMap;

  // Default constructor.
  MessageSvc( const std::string& name, ISvcLocator* svcloc );

  // Implementation of IService::reinitialize()
  StatusCode reinitialize() override;
  // Implementation of IService::initialize()
  StatusCode initialize() override;
  // Implementation of IService::finalize()
  StatusCode finalize() override;

  // Implementation of IMessageSvc::reportMessage()
  void reportMessage( const Message& message ) override;

  // Implementation of IMessageSvc::reportMessage()
  void reportMessage( const Message& msg, int outputLevel ) override;

  // Implementation of IMessageSvc::reportMessage()
  void reportMessage( const StatusCode& code, std::string_view source = "" ) override;

  // Implementation of IMessageSvc::reportMessage()
  void reportMessage( std::string source, int type, std::string message ) override;

  // Implementation of IMessageSvc::insertMessage()
  void insertMessage( const StatusCode& code, Message message ) override;

  // Implementation of IMessageSvc::eraseMessage()
  void eraseMessage() override;

  // Implementation of IMessageSvc::eraseMessage()
  void eraseMessage( const StatusCode& code ) override;

  // Implementation of IMessageSvc::eraseMessage()
  void eraseMessage( const StatusCode& code, const Message& message ) override;

  // Implementation of IMessageSvc::insertStream()
  void insertStream( int message_type, std::string name, std::ostream* stream ) override;

  // Implementation of IMessageSvc::eraseStream()
  void eraseStream() override;

  // Implementation of IMessageSvc::eraseStream()
  void eraseStream( int message_type ) override;

  // Implementation of IMessageSvc::eraseStream()
  void eraseStream( int message_type, std::ostream* stream ) override;

  // Implementation of IMessageSvc::eraseStream()
  void eraseStream( std::ostream* stream ) override;

  // Implementation of IMessageSvc::desaultStream()
  std::ostream* defaultStream() const override { return m_defaultStream; }

  // Implementation of IMessageSvc::setDefaultStream()
  void setDefaultStream( std::ostream* stream ) override {
    auto lock       = std::scoped_lock{ m_reportMutex };
    m_defaultStream = stream;
  }

  // Implementation of IMessageSvc::ouputLevel()
  int outputLevel() const override;

  // Implementation of IMessageSvc::ouputLevel()
  int outputLevel( std::string_view source ) const override;

  // Implementation of IMessageSvc::setOuputLevel()
  void setOutputLevel( int new_level ) override;

  // Implementation of IMessageSvc::setOuputLevel()
  void setOutputLevel( std::string_view source, int new_level ) override;

  // Implementation of IMessageSvc::useColor()
  bool useColor() const override { return m_color; }

  // Implementation of IMessageSvc::getLogColor()
  std::string getLogColor( int logLevel ) const override;

  // Implementation of IMessageSvc::messageCount()
  int messageCount( MSG::Level logLevel ) const override;

  // Implementation of IInactiveMessageCounter::incrInactiveCount()
  void incrInactiveCount( MSG::Level level, std::string_view src ) override;

protected:
  /// Internal implementation of reportMessage(const Message&,int) without lock.
  virtual void i_reportMessage( const Message& msg, int outputLevel );

  /// Internal implementation of reportMessage(const StatusCode&,const std::string&) without lock.
  virtual void i_reportMessage( const StatusCode& code, std::string_view source );

private:
  Gaudi::Property<std::string>  m_defaultFormat{ this, "Format", Message::getDefaultFormat(), "" };
  Gaudi::Property<std::string>  m_defaultTimeFormat{ this, "timeFormat", Message::getDefaultTimeFormat(), "" };
  Gaudi::Property<bool>         m_stats{ this, "showStats", false, "" };
  Gaudi::Property<unsigned int> m_statLevel{ this, "statLevel", 0, "" };

  std::array<Gaudi::Property<std::vector<std::string>>, MSG::NUM_LEVELS> m_thresholdProp{ { { /*ignored*/ },
                                                                                            { this, "setVerbose" },
                                                                                            { this, "setDebug" },
                                                                                            { this, "setInfo" },
                                                                                            { this, "setWarning" },
                                                                                            { this, "setError" },
                                                                                            { this, "setFatal" },
                                                                                            { this, "setAlways" } } };

  Gaudi::Property<bool> m_color{ this, "useColors", false, "" };

  std::array<Gaudi::Property<std::vector<std::string>>, MSG::NUM_LEVELS> m_logColors{ { { /*ignored*/ },
                                                                                        { this, "verboseColorCode" },
                                                                                        { this, "debugColorCode" },
                                                                                        { this, "infoColorCode" },
                                                                                        { this, "warningColorCode" },
                                                                                        { this, "errorColorCode" },
                                                                                        { this, "fatalColorCode" },
                                                                                        { this, "alwaysColorCode" } } };

  std::array<Gaudi::Property<int>, MSG::NUM_LEVELS> m_msgLimit{ { { this, "defaultLimit", 500 },
                                                                  { this, "verboseLimit", 500 },
                                                                  { this, "debugLimit", 500 },
                                                                  { this, "infoLimit", 500 },
                                                                  { this, "warningLimit", 500 },
                                                                  { this, "errorLimit", 500 },
                                                                  { this, "fatalLimit", 500 },
                                                                  { this, "alwaysLimit", 0 } } };

  Gaudi::Property<bool> m_suppress{ this, "enableSuppression", false, "" };
  Gaudi::Property<bool> m_inactCount{ this, "countInactive", false, &MessageSvc::setupInactCount, "" };

  Gaudi::Property<std::vector<std::string>> m_tracedInactiveSources{
      this,
      "tracedInactiveSources",
      {},
      "for each message source specified,  print a stack trace for the unprotected and unseen messages" };

  Gaudi::Property<std::map<std::string, std::string, std::less<>>> m_loggedStreamsName{
      this, "loggedStreams", {}, "MessageStream sources we want to dump into a logfile" };

  std::ostream* m_defaultStream = &std::cout; ///< Pointer to the output stream.
  Message       m_defaultMessage;             ///< Default Message
  StreamMap     m_streamMap;                  ///< Stream map
  MessageMap    m_messageMap;                 ///< Message map
  ThresholdMap  m_thresholdMap;               ///< Output level threshold map

  std::string m_logColorCodes[MSG::NUM_LEVELS];

  /// Private helper class to keep the count of messages of a type (MSG::LEVEL).
  struct MsgAry final {
    /// Internal array of counters.
    std::array<int, MSG::NUM_LEVELS> msg = { { 0 } };
    /// Default constructor.
    MsgAry() = default;
  };

  std::map<std::string, MsgAry, std::less<>> m_sourceMap, m_inactiveMap;

  std::array<int, MSG::NUM_LEVELS> m_msgCount;

  std::map<std::string, std::shared_ptr<std::ostream>, std::less<>> m_loggedStreams;

  void setupColors( Gaudi::Details::PropertyBase& prop );
  void setupLimits( Gaudi::Details::PropertyBase& prop );
  void setupThreshold( Gaudi::Details::PropertyBase& prop );
  void setupInactCount( Gaudi::Details::PropertyBase& prop );

  void setupLogStreams();

  /// Mutex to synchronize multiple threads printing.
  mutable std::recursive_mutex m_reportMutex;

  /// Mutex to synchronize multiple access to m_messageMap.
  mutable std::recursive_mutex m_messageMapMutex;

  /// Mutex to synchronize multiple access to m_thresholdMap
  /// (@see MsgStream::doOutput).
  mutable std::recursive_mutex m_thresholdMapMutex;
};

#endif
