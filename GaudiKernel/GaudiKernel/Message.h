// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/Message.h,v 1.5 2008/02/20 19:16:23 hmd Exp $
#ifndef GAUDIKERNEL_MESSAGE_H
#define GAUDIKERNEL_MESSAGE_H

#include <string>
#include <iostream>

/** @class Message Message.h GaudiKernel/Message.h

    The Message class. This class is used to contain messages which can then
    be formatted and sent to a message service

    @author Iain Last
*/
class GAUDI_API Message {
public:
  /// Default constructor
  Message();

  /// Constructor.
  Message ( const char* src, int type, const char* msg );

  /// Constructor.
  Message ( const std::string& src, int type, const std::string& msg );

  /// Default destructor.
  ~Message() {}

  /// Get the message string.
  const std::string& getMessage() const;

  /// Set the message string.
  void setMessage( const std::string& msg );

  /// Get the message type.
  int getType() const;

  /// Set the message type.
  void setType( int msg_type );

  /// Get the message source.
  const std::string& getSource() const;

  /// Set the message source.
  void setSource( const std::string& src );

  /// Get the format string.
  const std::string& getFormat() const;

  /// Get the default format string.
  static const std::string getDefaultFormat();

  /// Set the format string.
  void setFormat( const std::string& msg ) const;

  /// Get the time format string.
  const std::string& getTimeFormat() const;

  /// Get the default time format string
  static const std::string getDefaultTimeFormat() ;

  /// Set the time format string.
  void setTimeFormat( const std::string& timeFormat ) const;

  /// Needed to build maps
  bool operator < ( const Message& test );

  /// Insert the message into a stream.
  friend std::ostream& operator << ( std::ostream& stream, const Message& msg );

  /// Insert the message into a stream.
  friend bool operator == ( const Message& a, const Message& b );

protected:
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
  std::string m_source;

  /// The format string.
  mutable std::string m_format;

  /// Time format string.
  mutable std::string m_time_format;

  /// The type.
  int m_type;

  /// Formatted message.
  mutable std::string m_formatted_msg;

  /// The current fill character.
  mutable char m_fill;

  /// The current field width.
  mutable int m_width;

  /// Justification.
  mutable bool m_left;

  /// The character used to prefix formatting commands.
  static const char FORMAT_PREFIX;

  /// The character used to indicate start of left text justification.
  static const char JUSTIFY_LEFT;

  /// The character used to indicate start of right text justification.
  static const char JUSTIFY_RIGHT;

  /// The character used to indicate that the message should be printed.
  static const char MESSAGE;

  /// The character used to indicate that the message type should be printed.
  static const char TYPE;

  /// The character used to indicate that the message timestamp should be printed.
  static const char TIME;

  /// The character used to indicate that the message
  /// timestamp should be printed in UTC time.
  static const char UTIME;

  /// The character used to indicate that the message source should be printed.
  static const char SOURCE;

  /** The character used to indicate that the previous character is to be used
   * for padding out fields if the text is not long enough.
   */
  static const char FILL;

  /** The character used to indicate that the previous decimal characters
   * should be taken as the field width.
   */
  static const char WIDTH;

  /// The default message format.
  static const char* DEFAULT_FORMAT;

  /// The default time format.
  static const char* DEFAULT_TIME_FORMAT;

};

/// Insert the message into a stream.
GAUDI_API std::ostream& operator << ( std::ostream& stream, const Message& msg );

/// Insert the message into a stream.
GAUDI_API bool operator == ( const Message& a, const Message& b );

#endif
