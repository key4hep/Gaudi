#ifndef GAUDIKERNEL_IMESSAGESVC_H
#define GAUDIKERNEL_IMESSAGESVC_H

// Include files
#include "GaudiKernel/IInterface.h"
#include <iostream>
#include <string>

// Forward declarations
class StatusCode;
class Message;

/// Print levels enumeration
namespace MSG {
  enum Level {
    NIL = 0,
    VERBOSE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    ALWAYS,
    NUM_LEVELS
  };
  enum Color {
    BLACK = 0,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
    NUM_COLORS
  };
}

#ifdef _WIN32
// Avoid (hopefully) conflicts between Windows' headers and MSG.
#  ifndef NOMSG
#    define NOMSG
#    ifndef NOGDI
#      define NOGDI
#    endif
#  endif
#endif

/** @class IMessageSvc IMessageSvc.h GaudiKernel/IMessageSvc.h

    The IMessage is the interface implemented by the message service.
    This interface is used by any algorithm or services wanting to report
    messages to the end-user.

    @author Iain Last
*/
class GAUDI_API IMessageSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IMessageSvc,2,0);

  /** Report a message by sending a Message object to the message service
      @param message  Reference to a message object
      @param outputLevel Output level of the message source for this message
  */
  virtual void reportMessage( const Message& msg, int outputLevel ) = 0;

  /** Report a message by sending a Message object to the message service
      @param message  Reference to a message object
  */
  virtual void reportMessage( const Message& message ) = 0;

  /** Report an error to the message service. The service will use the error code
      number for formating a human readable message
      @param code Error code number
      @param source Message source. Typically the alg/svc name
  */
  virtual void reportMessage( const StatusCode& code,
                              const std::string& source = "" ) = 0;

  /** Report a message by specifying the source, severity level and text.
      @param source Message source. Typically the alg/svc name
      @param type Severity level
      @param message Text message
  */
  virtual void reportMessage( const std::string& source,
                              int type,
                              const std::string& message ) = 0;

  /** Report a message by specifying the source, severity level and text. The text is
      passed as C like character string to avoid extra copying.
      @param source Message source. Typically the alg/svc name
      @param type Severity level
      @param message Text message
  */
  virtual void reportMessage( const char* source,
                              int type,
                              const char* message = "" ) = 0;

  /** Insert a message to be sent for a given status code into the error code repository.
      @param code Status error code
      @param message Message associated
  */
  virtual void insertMessage( const StatusCode& code,
                              const Message& message ) = 0;

  /// Erase all messages associated to all status codes.
  virtual void eraseMessage() = 0;

  /// Erase message associated to a given status code.
  virtual void eraseMessage( const StatusCode& code ) = 0;

  /** Erase a given message associated to a given status code.
      @param code Status error code
      @param message Message associated
  */
  virtual void eraseMessage( const StatusCode& code,
                             const Message& message  ) = 0;

  /** Add a new stream for a message type (severity level).
      @param type Severity level
      @param name Stream name
      @param stream Pointer to a C++ stream
  */
  virtual void insertStream( int type,
                             const std::string& name,
                             std::ostream* stream ) = 0;

  /// Delete all the streams.
  virtual void eraseStream() = 0;

  /// Delete all the streams for a given message type (severity level).
  virtual void eraseStream( int message_type ) = 0;

  /** Delete a single stream for a given message type (severity level)
      @param type Severity level
      @param stream Pointer to a C++ stream
  */
  virtual void eraseStream( int type,
                            std::ostream* stream ) = 0;

  /** Delete all occurrences of a stream.
      @param stream Pointer to a C++ stream
  */
  virtual void eraseStream( std::ostream* stream ) = 0;

  /// Get the default stream.
  virtual std::ostream* defaultStream() const = 0;

  /** Set the default stream.
      @param stream Pointer to a C++ stream
  */
  virtual void setDefaultStream( std::ostream* stream ) = 0;

  /// Retrieve the current output level threshold
  virtual int outputLevel()   const = 0;

  /** Retrieve the current output level threshold for a given message source
      @param source Message source. Typically the alg/svc name
  */
  virtual int outputLevel( const std::string& source ) const = 0;

  /// Set new global output level threshold
  virtual void setOutputLevel( int new_level )  = 0;

  /** Set new output level threshold for a given message source
      @param source  Message source
      @param new_level Severity level
  */
  virtual void setOutputLevel( const std::string& source,
                               int new_level)  = 0;


  /** Show whether colors are used
   */
  virtual bool useColor() const = 0;

  /** Get the color codes for various log levels
      @param logLevel Logging level
   */
  virtual std::string getLogColor(int logLevel) const = 0;

  /** Get the number of messages issued at a particular level
   */
  virtual int messageCount( MSG::Level level ) const = 0;

};

class GAUDI_API IInactiveMessageCounter: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IInactiveMessageCounter,1,0);

  /** Increment deactivated message count.
   *  Used by MsgStream to record the sources of messages that are prepared, but
   *  not printed (because if insufficient level).
   */
  virtual void incrInactiveCount( MSG::Level level,
                                  const std::string& src ) = 0;
};

#endif // GAUDIKERNEL_IMESSAGESVC_H
