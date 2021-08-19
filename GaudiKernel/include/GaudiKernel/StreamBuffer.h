/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_STREAMBUFFER_H
#define GAUDIKERNEL_STREAMBUFFER_H 1

// STL include files
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <typeinfo>
#include <vector>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/swab.h"

// forward declarations
class StreamBuffer;
class DataObject;
class ContainedObject;

/** @class StreamBuffer StreamBuffer.h GaudiKernel/StreamBuffer.h

    The stream buffer is a small object collecting object data.
    The basic idea behind the StreamBuffer is generic object conversion.
    The StreamBuffer acts as a byte stream (hence inheriting from a
    std::string: DP: this is not true anymore and it is not a bad thing in my
    opinion) and stores any information streamed to the buffer.
    Since the information must be represented in a generic way
    on the fly byte swapping is performed. However, not only primitive
    data can be stored in the buffer, but also pointers to DataObjects
    (symbolic links) and pointers to contained objects. Automatically during
    serialization the persistent references to the corresponding objects
    and containers must be stored. These objects are accessible
    from the StreamBuffer object.

    "On the fly" data conversion to non persistent

    @author   M.Frank
*/
class StreamBuffer /* : public std::string  */
{
public:
  /// A small base class to handle generic data streaming
  class DataIO {
  public:
    /// Standard constructor
    DataIO() = default;
    /// Standard destructor
    virtual ~DataIO() = default;
    /// Throw Exception
    void badStreamMode() { throw( "Not acceptable stream mode!" ); }
    /// Serialization method: loads/dumps streambuffer content
    virtual void serialize( StreamBuffer& stream ) {
      if ( stream.isReading() )
        load( stream );
      else if ( stream.isWriting() )
        dump( stream );
      else
        badStreamMode();
    }
    /// Template function to load stream data
    virtual void load( StreamBuffer& ) { badStreamMode(); }
    /// Template function to save stream data
    virtual void dump( StreamBuffer& ) { badStreamMode(); }
  };

  /// Reader for standard input streams
  class Istream : public DataIO {
    /// Reference to input stream
    std::istream* m_stream;

  public:
    /// Constructor
    Istream( std::istream& str ) : m_stream( &str ) {}

    /// Data load method
    void load( StreamBuffer& stream ) override {
      // Generic implementation for istreams:
      int len;
      ( *m_stream ) >> len;
      stream.erase();
      stream.reserve( len );
      m_stream->read( stream.data(), len );
    }
  };
  /// Writer for standard output streams
  class Ostream : public DataIO {
    std::ostream* m_stream;

  public:
    /// Standard constructor: pass reference to stream object
    Ostream( std::ostream& str ) : m_stream( &str ) {}
    /// Standard Destructor
    virtual ~Ostream() = default;

    /// Output dumper
    void dump( StreamBuffer& stream ) override {
      // Generic implementation for ostreams:
      ( *m_stream ) << stream.buffPointer();
      m_stream->write( stream.data(), stream.buffPointer() );
    }
  };

public:
  /// Streamer mode
  enum Mode { UNINITIALIZED, READING, WRITING };
  /// Data Sawp actions
  enum SwapAction { SINGLE_BYTE, SWAP, NOSWAP };
  /// Link state defintions
  enum State { INVALID = -1, VALID };
  /// Definition of the contained link set
  class ContainedLink {
  public:
    ContainedObject* first  = nullptr;
    long             second = INVALID;
    long             third  = INVALID;
    ContainedLink()         = default;
    ContainedLink( ContainedObject* pObj, long hint, long link ) : first( pObj ), second( hint ), third( link ) {}
    ContainedLink& operator=( const ContainedLink& copy ) = default;
  };
  /// Definition of the contained link set
  class IdentifiedLink {
  public:
    DataObject* first                            = nullptr;
    long        second                           = INVALID;
    IdentifiedLink()                             = default;
    IdentifiedLink( const IdentifiedLink& copy ) = default;
    IdentifiedLink( DataObject* pObj, long hint ) : first( pObj ), second( hint ) {}
  };

  typedef std::vector<ContainedLink> ContainedLinks;
  /// Definition of the identifiable link set
  typedef std::vector<IdentifiedLink> IdentifiedLinks;
  /// Definition of the buffer analyzer
  typedef void ( *AnalyzeFunction )( const void* data, int siz, const std::type_info& type );
  /// DataObject is friend
  friend class DataObject;

protected:
  /// Boolean indicating wether the stream is in read or write mode
  Mode m_mode = UNINITIALIZED;

  /// Current buffer pointer
  mutable long m_pointer = 0;

  /// Total buffer length
  mutable long m_length = 0;

  /// Pointer to heap buffer
  mutable char* m_buffer = nullptr;

  /// Flag indicating swapping
  bool m_swapEnabled = true;

  /// Container with links to contained objects
  mutable ContainedLinks m_containedLinks;

  /// Container with links to contained objects
  mutable IdentifiedLinks m_identifiedLinks;

  /// Hook function for analysis of data to the stream
  AnalyzeFunction m_analyzer = nullptr;

  /// Check for byte swapping
  SwapAction swapBuffer( int siz ) const;

  /** Helper to distinguish between identified pointers and contained pointers.
      This entry resolves identified pointers (= Pointers to DataObject instances.)
  */
  template <class TYPE>
  StreamBuffer& getObjectPointer( const DataObject* /*pObject*/, TYPE*& refpObject ) {
    IdentifiedLink& link = m_identifiedLinks.back();
    DataObject*     pObj = link.first;
    m_identifiedLinks.pop_back();
    refpObject = dynamic_cast<TYPE*>( pObj );
    return *this;
  }
  /** Helper to distinguish between identified pointers and contained pointers.
      This entry resolves contained pointers (= Pointers to ContainedObject instances.)
  */
  template <class TYPE>
  StreamBuffer& getObjectPointer( const ContainedObject* /*pObject*/, TYPE*& refpObject ) {
    ContainedLink&   link = m_containedLinks.back();
    ContainedObject* pObj = link.first;
    m_containedLinks.pop_back();
    refpObject = dynamic_cast<TYPE*>( pObj );
    return *this;
  }

public:
  /// Standard constructor
  StreamBuffer( bool do_swap = true ) : m_swapEnabled( do_swap ) {}
  /// Standard destructor
  virtual ~StreamBuffer() { ::free( m_buffer ); }
  /// Read access to data buffer
  const char* data() const { return m_buffer; }
  /// write access to data buffer
  char* data() { return m_buffer; }
  /// Reset the buffer
  void erase() { m_pointer = 0; }
  /// Remove the data buffer and pass it to client. It's the client responsability to free the memory
  char* adopt() const {
    char* ptr = m_buffer;
    m_containedLinks.erase( m_containedLinks.begin(), m_containedLinks.end() );
    m_identifiedLinks.erase( m_identifiedLinks.begin(), m_identifiedLinks.end() );
    m_buffer  = NULL; // char *
    m_pointer = 0;    // long
    m_length  = 0;    // long
    return ptr;
  }
  /// Reserve buffer space; Default: 16 k buffer size
  void reserve( long len ) {
    if ( len > m_length ) {
      m_length = ( len < 16384 ) ? 16384 : len;
      m_buffer = (char*)::realloc( m_buffer, m_length );
    }
  }
  /// Extend the buffer
  void extend( long len ) {
    if ( len + m_pointer > m_length ) {
      // We have to be a bit generous here in order not to run too often
      // into ::realloc().
      long new_len = ( m_length < 16384 ) ? 16384 : 2 * m_length;
      if ( m_length < len ) new_len += len;
      reserve( new_len );
    }
  }
  /// Total buffer size
  long size() const { return m_length; }
  /// Access to contained links
  ContainedLinks& containedLinks() { return m_containedLinks; }
  /// CONST Access to contained links
  const ContainedLinks& containedLinks() const { return m_containedLinks; }

  /// Access to identified links
  IdentifiedLinks& identifiedLinks() { return m_identifiedLinks; }
  /// CONST Access to identified links
  const IdentifiedLinks& identifiedLinks() const { return m_identifiedLinks; }

  /// Set mode of the stream and allocate buffer
  void setMode( Mode m ) {
    m_mode    = m;
    m_pointer = 0;
    m_containedLinks.erase( m_containedLinks.begin(), m_containedLinks.end() );
    m_identifiedLinks.erase( m_identifiedLinks.begin(), m_identifiedLinks.end() );
  }

  /// Get stream buffer state
  bool isReading() const { return m_mode == READING; }

  /// Get stream buffer state
  bool isWriting() const { return m_mode == WRITING; }
  /// Retrieve current buffer pointer
  long buffPointer() const { return m_pointer; }
  /// Retrieve current buffer pointer
  void setBuffPointer( long ptr ) { m_pointer = ptr; }
  /// Enable user analysis function
  void setAnalyzer( AnalyzeFunction fun = nullptr ) { m_analyzer = fun; }
  /// Swap buffers: int, long, short, float and double
  void swapToBuffer( const void* source, int siz );

  /// Swap buffers: int, long, short, float and double
  void swapFromBuffer( void* target, int siz );

  /// Write string to output stream
  StreamBuffer& writeBytes( const char* str, long len ) {
    extend( m_pointer + len + 4 );
    *this << len;
    std::copy_n( str, len, data() + buffPointer() );
    m_pointer += len;
    return *this;
  }

  void getIdentifiedLink( DataObject*& pObject, long& hint ) {
    IdentifiedLink& l = m_identifiedLinks.back();
    pObject           = l.first;
    hint              = l.second;
    m_identifiedLinks.pop_back();
  }
  void addIdentifiedLink( const DataObject* pObject, long hint ) {
    m_identifiedLinks.push_back( IdentifiedLink( (DataObject*)pObject, hint ) );
  }

  void getContainedLink( ContainedObject*& pObject, long& hint, long& link ) {
    ContainedLink& l = m_containedLinks.back();
    pObject          = l.first;
    hint             = l.second;
    link             = l.third;
    m_containedLinks.pop_back();
  }
  void addContainedLink( const ContainedObject* pObject, long hint, long link ) {
    m_containedLinks.push_back( ContainedLink( (ContainedObject*)pObject, hint, link ) );
  }

#ifdef USE_STREAM_ANALYSER
#  define STREAM_ANALYSE( data, len )                                                                                  \
    if ( 0 != m_analyzer ) m_analyzer( &data, len, typeid( data ) )
#else
#  define STREAM_ANALYSE( data, len )
#endif

// Implement streamer macros for primivive data types.
#define IMPLEMENT_STREAMER( TYPE )                                                                                     \
  /*  Output Streamer                                   */                                                             \
  StreamBuffer& operator<<( TYPE data ) {                                                                              \
    swapToBuffer( &data, sizeof( data ) );                                                                             \
    STREAM_ANALYSE( data, sizeof( data ) );                                                                            \
    return *this;                                                                                                      \
  }                                                                                                                    \
  /*  Input Streamer                                    */                                                             \
  StreamBuffer& operator>>( TYPE& data ) {                                                                             \
    swapFromBuffer( &data, sizeof( data ) );                                                                           \
    return *this;                                                                                                      \
  }
// RootCint does not understand this macro....
// But we can easily live without it!
#undef IMPLEMENT_STREAMER

  ///  Output Streamer
  StreamBuffer& operator<<( long long data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( long long& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( int data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( int& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( unsigned int data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( unsigned int& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( long data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( long& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( unsigned long data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( unsigned long& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( short data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( short& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( unsigned short data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( unsigned short& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( char data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( char& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( unsigned char data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( unsigned char& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( float data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( float& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  ///  Output Streamer
  StreamBuffer& operator<<( double data ) {
    swapToBuffer( &data, sizeof( data ) );
    STREAM_ANALYSE( data, sizeof( data ) );
    return *this;
  }
  ///  Input Streamer
  StreamBuffer& operator>>( double& data ) {
    swapFromBuffer( &data, sizeof( data ) );
    return *this;
  }
  /// Streamer to read strings in (char*) format
  StreamBuffer& operator>>( char* data ) {
    long i, len;
    *this >> len;
    for ( i = 0, data[0] = 0; i < len; i++ ) { data[i] = m_buffer[m_pointer++]; }
    return *this;
  }
  /// Streamer to write strings in (char*) format
  StreamBuffer& operator<<( const char* data ) {
    const char* ptr = 0 == data ? "" : data;
    size_t      len = strlen( ptr ) + 1;
    if ( 0 == m_analyzer )
      writeBytes( ptr, len );
    else {
      STREAM_ANALYSE( data, len );
    }
    return *this;
  }
  /// Streamer to read strings in (std::string) format
  StreamBuffer& operator>>( std::string& data ) {
    long i, len;
    *this >> len;
    for ( i = 0, data = ""; i < len; i++ ) { data.append( 1, m_buffer[m_pointer++] ); }
    return *this;
  }
  /// Streamer to write strings in (std::string) format
  StreamBuffer& operator<<( const std::string& data ) {
    if ( 0 == m_analyzer ) {
      const char* ptr = data.c_str();
      long        len = data.length();
      writeBytes( ptr, len );
    } else {
      STREAM_ANALYSE( data, sizeof( data ) );
    }
    return *this;
  }
  /** Streamer to read links to contained or identified objects.
      The specified internal function call distinguishes between
      contained and identified objects.
      @param    refpObject Reference to pointer to object to be loaded.
      @return              Reference to StreamBuffer object
  */
  template <class TYPE>
  StreamBuffer& operator>>( TYPE*& refpObject ) {
    return getObjectPointer( refpObject, refpObject );
  }

  /** Streamer to write links to contained objects.
      Links to contained objects are not stored immediately,
      but collected instead and analyzed later.
      @param       pObject Pointer to object to be loaded.
      @return              Reference to StreamBuffer object
  */
  StreamBuffer& operator<<( const ContainedObject* pObject ) {
    STREAM_ANALYSE( pObject, sizeof( pObject ) );
    addContainedLink( pObject, INVALID, INVALID );
    return *this;
  }

  /** Streamer to write links to identified objects.
      Links to identified objects are not stored immediately,
      but collected instead and analyzed later.
      @param       pObject Pointer to object to be loaded.
      @return              Reference to StreamBuffer object
  */
  StreamBuffer& operator<<( const DataObject* pObject ) {
    STREAM_ANALYSE( pObject, sizeof( pObject ) );
    addIdentifiedLink( pObject, INVALID );
    return *this;
  }

  /** Serialize the buffer using an IO object.
      The streambuffer object will make use of a DataIO object, which
      can be specialized for streaming to any representation like
      e.g. disk files, Root files, Objectivity etc.
      @param       ioObject Reference to data IO object.
  */
  void serialize( DataIO& ioObject ) {
    ioObject.serialize( *this );
    m_pointer = 0;
  }
};

#undef STREAM_ANALYSE

/// Check for byte swapping
inline StreamBuffer::SwapAction StreamBuffer::swapBuffer( int siz ) const {
  switch ( siz ) {
  case 1:
    return SINGLE_BYTE;
  default:
#if defined( __alpha ) && !defined( __VMS )
    //    return m_swapEnabled ? SWAP : NOSWAP;
    return NOSWAP;
#elif defined( __sun ) && defined( __SVR4 ) && defined( __i386 )
    //    return m_swapEnabled ? SWAP : NOSWAP;
    return NOSWAP;
#elif defined( __APPLE__ )
    //    return m_swapEnabled ? SWAP : NOSWAP;
    return SWAP;
#elif defined( __linux ) && !defined( __powerpc )
    //    return m_swapEnabled ? SWAP : NOSWAP;
    return NOSWAP;
#elif defined( BORLAND ) || defined( _WIN32 ) || defined( WIN32 )
    //    return m_swapEnabled ? SWAP : NOSWAP;
    return NOSWAP;
#else
    return m_swapEnabled ? SWAP : NOSWAP;
//    return NOSWAP;
#endif
  }
}

/// Swap bytes from a source buffer to the stream buffer with arbitray size
inline void StreamBuffer::swapToBuffer( const void* source, int siz ) {
  char buff[8], *tar, *src = (char*)source;
  extend( m_pointer + siz );
  tar = m_buffer + m_pointer;
  switch ( swapBuffer( siz ) ) {
  case SINGLE_BYTE:
    *tar = *src;
    break;
  case SWAP:
#ifdef __APPLE__
    for ( int i = 0, j = siz - 1; i < siz; i++, j-- ) tar[j] = src[i];
#else
    ::_swab( src, buff, siz );
#endif
    src = buff;
    [[fallthrough]];
  case NOSWAP:
    std::copy_n( src, siz, tar );
    break;
  default:
    break;
  }
  m_pointer += siz;
}

/// Swap bytes from the stream buffer to target buffer with arbitray size
inline void StreamBuffer::swapFromBuffer( void* target, int siz ) {
  char* tar = (char*)target;
  char* src = m_buffer + m_pointer;
  switch ( swapBuffer( siz ) ) {
  case SINGLE_BYTE:
    *tar = *src;
    break;
  case SWAP:
#ifdef __APPLE__
    for ( int i = 0, j = siz - 1; i < siz; i++, j-- ) tar[j] = src[i];
#else
    ::_swab( src, tar, siz );
#endif
    break;
  case NOSWAP:
    std::copy_n( src, siz, tar );
    break;
  default:
    break;
  }
  m_pointer += siz;
}

// Output serialize a vector of items
template <class T>
inline StreamBuffer& operator<<( StreamBuffer& s, const std::vector<T>& v ) {
  s << v.size();
  for ( const auto& i : v ) s << i;
  return s;
}

// Input serialize a vector of items
template <class T>
inline StreamBuffer& operator>>( StreamBuffer& s, std::vector<T>& v ) {
  long i, len;
  s >> len;
  v.clear();
  for ( i = 0; i < len; i++ ) {
    T temp;
    s >> temp;
    v.push_back( temp );
  }
  return s;
}

// Output serialize a list of items
template <class T>
inline StreamBuffer& operator<<( StreamBuffer& s, const std::list<T>& l ) {
  s << l.size();
  for ( const auto& i : l ) s << i;
  return s;
}

// Input serialize a list of items
template <class T>
inline StreamBuffer& operator>>( StreamBuffer& s, std::list<T>& l ) {
  long len;
  s >> len;
  l.clear();
  for ( long i = 0; i < len; i++ ) {
    T temp;
    s >> temp;
    l.push_back( temp );
  }
  return s;
}
#endif // GAUDIKERNEL_STREAMBUFFER_H
