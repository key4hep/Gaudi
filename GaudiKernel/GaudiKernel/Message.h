#ifndef GAUDIKERNEL_MESSAGE_H
#define GAUDIKERNEL_MESSAGE_H

#include "GaudiKernel/Kernel.h" // for GAUDI_API
#include <iostream>
#include <string>

/** @class Message Message.h GaudiKernel/Message.h

    The Message class. This class is used to contain messages which can then
    be formatted and sent to a message service

    @author Iain Last
*/
class GAUDI_API Message final
{
public:
  /// Default constructor
  Message();

  /// Constructor.
  Message( const char* src, int type, const char* msg );

  /// Constructor.
  Message( std::string src, int type, std::string msg );

  /// Default destructor.
  ~Message() = default;

  /// Get the message string.
  const std::string& getMessage() const;

  /// Set the message string.
  void setMessage( std::string msg );

  /// Get the message type.
  int getType() const;

  /// Set the message type.
  void setType( int msg_type );

  /// Get the message source.
  const std::string& getSource() const;

  /// Set the message source.
  void setSource( std::string src );

  /// Get the format string.
  const std::string& getFormat() const;

  /// Get the default format string.
  static const std::string getDefaultFormat();

  /// Set the format string.
  void setFormat( std::string msg ) const;

  /// Get the time format string.
  const std::string& getTimeFormat() const;

  /// Get the default time format string
  static const std::string getDefaultTimeFormat();

  /// Set the time format string.
  void setTimeFormat( std::string timeFormat ) const;

  /// Needed to build maps
  bool operator<( const Message& test );

  /// Insert the message into a stream.
  friend std::ostream& operator<<( std::ostream& stream, const Message& msg );

  /// Insert the message into a stream.
  friend bool operator==( const Message& a, const Message& b );

private:
  /// Called when an invalid format string is encountered.
  void invalidFormat() const;

  /// Format the message.
  void makeFormattedMsg( const std::string& format ) const;

  /// Decode format.
  void decodeFormat( const std::string& format ) const;

  /// Truncate or pad the output string to the field width as necessary.
  void sizeField( const std::string& text ) const;

  /// Set the width of a stream.
  void setWidth( const std::string& formatArg ) const;

  /// The message.
  std::string m_message;

  /// The source.
  std::string m_source = "UNKNOWN";

  /// The format string.
  mutable std::string m_format = DEFAULT_FORMAT;

  /// Time format string.
  mutable std::string m_time_format = DEFAULT_TIME_FORMAT;

  /// The type.
  int m_type = 0;

  /// Formatted message.
  mutable std::string m_formatted_msg;

  /// The current fill character.
  mutable char m_fill = ' ';

  /// The current field width.
  mutable int m_width = 0;

  /// Justification.
  mutable bool m_left = true;

  /// Formatting string characters.

  /// The character used to prefix formatting commands.
  static const char FORMAT_PREFIX = '%';

  /// The character used to indicate start of left text justification.
  static const char JUSTIFY_LEFT = 'L';

  /// The character used to indicate start of right text justification.
  static const char JUSTIFY_RIGHT = 'R';

  /// The character used to indicate that the message should be printed.
  static const char MESSAGE = 'M';

  /// The character used to indicate that the message type should be printed.
  static const char TYPE = 'T';

  /// The character used to indicate that the message timestamp should be printed.
  static const char TIME = 't';

  /// The character used to indicate that the message
  /// timestamp should be printed in UTC time.
  static const char UTIME = 'u';

  /// The character used to indicate that the message source should be printed.
  static const char SOURCE = 'S';

  /** The character used to indicate that the previous character is to be used
   * for padding out fields if the text is not long enough.
   */
  static const char FILL = 'F';

  /** The character used to indicate that the previous decimal characters
   * should be taken as the field width.
   */
  static const char WIDTH = 'W';

  /// The default message format.
  // static const char* Message::DEFAULT_FORMAT = "%
  // F%67W%L#############################################################################\n-----------------------------------------------------------------------------\nMessage
  // follows...\nSource  : %S\nType    : %T\nMessage : %M\nEnd of
  // message.\n-----------------------------------------------------------------------------\n";
  static constexpr const char* DEFAULT_FORMAT = "% F%18W%S%7W%R%T %0W%M";

  /// The default time format.
  // Time format accepts anything that strftime does plus %f for milliseconds
  static constexpr const char* DEFAULT_TIME_FORMAT = "%Y-%m-%d %H:%M:%S,%f";

  /// For slot and event number from EventContext
  static const char SLOT   = 's';
  static const char EVTNUM = 'e';
  static const char THREAD = 'X';

  size_t m_ecSlot;
  long int m_ecEvt;
  pthread_t m_ecThrd;
};

/// Insert the message into a stream.
GAUDI_API std::ostream& operator<<( std::ostream& stream, const Message& msg );

/// Insert the message into a stream.
GAUDI_API bool operator==( const Message& a, const Message& b );

#endif
