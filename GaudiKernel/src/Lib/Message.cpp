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
#include "GaudiKernel/Message.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/Time.h"
#include "GaudiKernel/Timing.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

using namespace MSG;

namespace {
  // get the current time from the system and format it according to the format
  inline std::string formattedTime( const std::string& fmt, bool universal = false ) {
    return Gaudi::Time::current().format( !universal, fmt );
  }
} // namespace

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: Constructor.
// Purpose:
// ---------------------------------------------------------------------------
//
Message::Message( const char* src, int type, const char* msg )
    : Message( std::string( src ), type, std::string( msg ) ) {}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: Constructor.
// Purpose:
// ---------------------------------------------------------------------------
//
Message::Message( std::string src, int type, std::string msg )
    : m_message( std::move( msg ) ), m_source( std::move( src ) ), m_type( type ) {

  const EventContext& ctx = Gaudi::Hive::currentContext();
  m_ecSlot                = ctx.slot();
  m_ecEvt                 = ctx.evt();
  m_ecEvtId               = ctx.eventID();
  m_ecThrd                = pthread_self();
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: operator <<
// Purpose:Insert the message into a stream.
// ---------------------------------------------------------------------------
//
std::ostream& operator<<( std::ostream& stream, const Message& msg ) {
  msg.makeFormattedMsg( msg.m_format );
  stream << msg.m_formatted_msg;
  return stream;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: operator <
// Purpose: comparison operator needed for maps
// ---------------------------------------------------------------------------
//
bool operator<( const Message& lhs, const Message& rhs ) {
  return lhs.m_type < rhs.m_type || lhs.m_source < rhs.m_source || lhs.m_message < rhs.m_message;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: operator ==
// Purpose: comparison op.
// ---------------------------------------------------------------------------
//
bool operator==( const Message& a, const Message& b ) {
  return a.m_source == b.m_source && a.m_type == b.m_type && a.m_message == b.m_message;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Set the format string -
//          use isFormatted() to check for valid format.
// ---------------------------------------------------------------------------
//
void Message::setFormat( std::string format ) const {
  if ( LIKELY( !format.empty() ) ) {
    m_format = std::move( format );
  } else {
    m_format = DEFAULT_FORMAT;
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Set the time format string -
//          use isFormatted() to check for valid format.
// ---------------------------------------------------------------------------
//
void Message::setTimeFormat( std::string timeFormat ) const {
  m_time_format = ( timeFormat.empty() ? DEFAULT_TIME_FORMAT : std::move( timeFormat ) );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: makeFormattedMsg
// Purpose: This formats the message according to the format string.
// ---------------------------------------------------------------------------
//
void Message::makeFormattedMsg( const std::string& format ) const {
  m_formatted_msg.clear();
  auto i = format.begin();
  while ( i != format.end() ) {

    // Output format string until format statement found.
    while ( i != format.end() && *i != FORMAT_PREFIX ) m_formatted_msg += *i++;

    // Test for end of format string.
    if ( i == format.end() ) break;
    i++;

    // Find type of formatting.
    std::string this_format;
    while ( i != format.end() && *i != FORMAT_PREFIX && *i != MESSAGE && *i != TYPE && *i != SOURCE && *i != COMP &&
            *i != FILL && *i != WIDTH && *i != TIME && *i != UTIME && *i != SLOT && *i != EVTNUM && *i != THREAD &&
            *i != EVENTID && *i != JUSTIFY_LEFT && *i != JUSTIFY_RIGHT ) {
      this_format += *i++;
    }

    // Reached end of string with improper format.
    if ( i == format.end() ) {
      invalidFormat();
      break;
    }

    this_format += *i++;
    decodeFormat( this_format );
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: decodeFormat
// Purpose: This the work horse that checks for a valid format string.
// ---------------------------------------------------------------------------
//
void Message::decodeFormat( const std::string& format ) const {
  if ( !format.empty() ) {
    const char        FORMAT_TYPE  = format[format.length() - 1];
    const std::string FORMAT_PARAM = format.substr( 0, format.length() - 1 );

    // Now test the format.
    std::string level;
    switch ( FORMAT_TYPE ) {
    case FILL:
      if ( FORMAT_PARAM.length() == 1 ) {
        m_fill = FORMAT_PARAM[0];
      } else
        invalidFormat();
      break;

    case TIME: {
      sizeField( formattedTime( m_time_format ) );
    } break;

    case UTIME: {
      sizeField( formattedTime( m_time_format, true ) );
    } break;

    case THREAD: {
      std::ostringstream ost;
      //	ost << "0x" << std::hex << pthread_self();
      ost << "0x" << std::hex << m_ecThrd;
      const std::string& thrStr( ost.str() );
      sizeField( thrStr );
    } break;

    case SLOT: {
      std::ostringstream ost;
      if ( m_ecSlot != EventContext::INVALID_CONTEXT_ID ) { ost << m_ecSlot; }
      sizeField( ost.str() );
    } break;

    case EVTNUM: {
      std::ostringstream ost;
      if ( m_ecEvt != EventContext::INVALID_CONTEXT_EVT ) { ost << m_ecEvt; }
      sizeField( ost.str() );
    } break;

    case EVENTID: {
      std::ostringstream ost;
      if ( m_ecEvtId.isValid() ) { ost << m_ecEvtId; }
      sizeField( ost.str() );
    } break;

    case MESSAGE:
      sizeField( m_message );
      break;

    case SOURCE:
      sizeField( m_source );
      break;

    case COMP:
      sizeField( m_source, true );
      break;

    case TYPE:
      switch ( m_type ) {
#define SET( x )                                                                                                       \
  case x:                                                                                                              \
    level = #x;                                                                                                        \
    break
        SET( NIL );
        SET( VERBOSE );
        SET( DEBUG );
        SET( INFO );
        SET( WARNING );
        SET( ERROR );
        SET( FATAL );
      case ALWAYS:
        level = "SUCCESS";
        break;
      default:
        level = "UNKNOWN";
        break;
#undef SET
      }
      sizeField( level );
      break;

    case FORMAT_PREFIX:
      m_formatted_msg += FORMAT_PREFIX;
      break;
    case JUSTIFY_RIGHT:
      m_left = false;
      break;
    case JUSTIFY_LEFT:
      m_left = true;
      break;
    case WIDTH:
      setWidth( FORMAT_PARAM );
      break;
    default:
      invalidFormat();
      break;
    }
  } else
    invalidFormat();
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: invalidFormat.
// Purpose: called when invalid format found.
// ---------------------------------------------------------------------------
//

void Message::invalidFormat() const { makeFormattedMsg( DEFAULT_FORMAT ); }

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: setWidth
// Purpose: Sets the minimum width of a stream field.
// ---------------------------------------------------------------------------
//
namespace {
  // Check that a container only contains digits.
  constexpr struct all_digit_t {
    template <typename C>
    bool operator()( const C& c ) const {
      return std::all_of( std::begin( c ), std::end( c ),
                          []( typename C::const_reference i ) { return isdigit( i ); } );
    }
  } all_digits{};
} // namespace

void Message::setWidth( const std::string& formatArg ) const {
  // Convert string to int, if string contains digits only...
  if ( all_digits( formatArg ) )
    m_width = std::stoi( formatArg );
  else
    invalidFormat();
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: sizeField
// Purpose: Truncates or pads the text to m_width as necessary
// ---------------------------------------------------------------------------
//

void Message::sizeField( const std::string& text, bool middle ) const {
  std::string newText;
  if ( m_width == 0 || m_width == static_cast<int>( text.length() ) ) {
    newText = text;
  } else {
    const size_t width = static_cast<size_t>( m_width );
    // Truncate the text if it is too long.
    if ( width < text.length() ) {
      if ( middle && width > 4 ) { // truncate text in the middle
        size_t iTrunc;
        // If the text is a component-name-chain, try showing the last component
        const size_t iDot = text.rfind( '.' );
        if ( iDot != std::string::npos && width > text.length() - iDot + 3 ) {
          iTrunc = iDot + 1;
        } else {
          // No dot or text after dot too long
          iTrunc = text.length() - width / 2;
        }
        const size_t taillength  = text.length() - iTrunc;
        const size_t frontlength = width - taillength - 3;
        newText.reserve( width );
        newText.append( text, 0, frontlength );
        newText.append( 3, '.' );
        newText.append( text, iTrunc );
      }      // else if middle
      else { // truncate text at the end
        newText = text.substr( 0, m_width );
        for ( int i = 0, j = newText.length() - 1; i < 3 && j >= 0; ++i, --j ) newText[j] = '.';
      }
    }
    // Pad the text.
    else {
      newText = std::string( m_width, m_fill );
      if ( m_left )
        newText.replace( newText.begin(), newText.begin() + text.length(), text.begin(), text.end() );
      else
        newText.replace( newText.end() - text.length(), newText.end(), text.begin(), text.end() );
    }
  }

  m_formatted_msg += newText;
}
