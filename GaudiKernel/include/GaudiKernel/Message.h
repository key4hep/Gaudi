/***********************************************************************************\
* (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_MESSAGE_H
#define GAUDIKERNEL_MESSAGE_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/Kernel.h" // for GAUDI_API
#include <iostream>
#include <string>

/** @class Message Message.h GaudiKernel/Message.h

    The Message class. This class is used to contain messages which can then
    be formatted and sent to a message service

    @author Iain Last
*/
class GAUDI_API Message final {
public:
  /// Default constructor
  Message() = default;

  /// Constructor.
  Message( const char* src, int type, const char* msg );

  /// Constructor.
  Message( std::string src, int type, std::string msg );

  /// Default destructor.
  ~Message() = default;

  /// Get the message string.
  const std::string& getMessage() const { return m_message; }

  /// Set the message string.
  void setMessage( std::string msg ) { m_message = std::move( msg ); }

  /// Get the message type.
  int getType() const { return m_type; }

  /// Set the message type.
  void setType( int msg_type ) { m_type = msg_type; }

  /// Get the message source.
  const std::string& getSource() const { return m_source; }

  /// Set the message source.
  void setSource( std::string_view src ) { m_source = src; }

  /** @name Event identifiers of message */
  //@{
  EventContext::ContextID_t  getEventSlot() const { return m_ecSlot; }
  EventContext::ContextEvt_t getEventNumber() const { return m_ecEvt; }
  EventIDBase                getEventID() const { return m_ecEvtId; }
  //@}

  /// Get the format string.
  const std::string& getFormat() const { return m_format; }

  /// Get the default format string.
  static const std::string getDefaultFormat() { return DEFAULT_FORMAT; }

  /// Set the format string.
  void setFormat( std::string msg ) const;

  /// Get the time format string.
  const std::string& getTimeFormat() const { return m_time_format; }

  /// Get the default time format string
  static const std::string getDefaultTimeFormat() { return DEFAULT_TIME_FORMAT; }

  /// Set the time format string.
  void setTimeFormat( std::string timeFormat ) const;

  /// Needed to build maps
  friend bool operator<( const Message& lhs, const Message& rhs );

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

  /// Truncate or pad the output string to the field width. If middle is true, cut the central part, otherwise the end
  void sizeField( const std::string& text, bool middle = false ) const;

  /// Set the width of a stream.
  void setWidth( const std::string& formatArg ) const;

  std::string         m_message;                            ///< The message text
  std::string         m_source{ "UNKNOWN" };                ///< The message source
  int                 m_type{ 0 };                          ///< The message type/level
  mutable std::string m_format{ DEFAULT_FORMAT };           ///< The format string
  mutable std::string m_time_format{ DEFAULT_TIME_FORMAT }; ///< Time format string
  mutable std::string m_formatted_msg;                      ///< Formatted message
  mutable char        m_fill{ ' ' };                        ///< Current fill character
  mutable int         m_width{ 0 };                         ///< Current field width
  mutable bool        m_left{ true };                       ///< The message alignment

  /** @name Event identifiers */
  //@{
  EventContext::ContextID_t  m_ecSlot;  ///< Event slot
  EventContext::ContextEvt_t m_ecEvt;   ///< Event number
  EventIDBase                m_ecEvtId; ///< Full event ID
  pthread_t                  m_ecThrd;  ///< Thread ID
  //@}

  /** @name Formatting characters */
  //@{
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

  /// The character used to indicate that the message timestamp should be printed in UTC time.
  static const char UTIME = 'u';

  /// The character used to indicate that the message source should be printed.
  static const char SOURCE = 'S';

  /// The character used to indicate that the message source should be printed, focus on the component
  static const char COMP = 'C';

  /// The character used to indicate that the slot number should be printed.
  static const char SLOT = 's';

  /// The character used to indicate that the event number should be printed.
  static const char EVTNUM = 'e';

  /// The character used to indicate that the thread ID should be printed.
  static const char THREAD = 'X';

  /// The character used to indicate that the full event ID should be printed.
  static const char EVENTID = 'E';

  /// The character used to indicate that the previous character is used to pad fields if the text is not long enough.
  static const char FILL = 'F';

  /// The character used to indicate that the previous decimal characters should be taken as the field width.
  static const char WIDTH = 'W';
  //@}

  /// The default message format.
  static constexpr const char* DEFAULT_FORMAT = "% F%18W%S%7W%R%T %0W%M";

  /// The default time format (accepts strftime formatters plus \%f for milliseconds).
  static constexpr const char* DEFAULT_TIME_FORMAT = "%Y-%m-%d %H:%M:%S,%f";
};

#endif
