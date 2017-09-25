#ifndef GAUDIKERNEL_ISSUESEVERITY_H
#define GAUDIKERNEL_ISSUESEVERITY_H 1

class StatusCode;

#include <iostream>
#include <map>
#include <string>

#include "GaudiKernel/Kernel.h"

#define ISSUE( x, y ) IssueSeverity( x, __LINE__, __FILE__, y )
#define STATUSCODE( z, x, y ) StatusCode( z, ISSUE( x, y ) )

class IIssueLogger;

#ifdef _WIN32
#ifdef ERROR
#undef ERROR
#endif
#endif

class GAUDI_API IssueSeverity final
{

public:
  enum Level {
    NIL = 0,
    VERBOSE,
    DEBUG,
    DEBUG1,
    DEBUG2,
    DEBUG3,
    INFO,
    WARNING,
    RECOVERABLE,
    ERROR,
    FATAL,
    ALWAYS,
    NUM_LEVELS
  };

  IssueSeverity() {}
  IssueSeverity( IssueSeverity::Level level, int line, std::string file, std::string msg = "" );
  IssueSeverity( IssueSeverity::Level level, std::string msg = "" );

  IssueSeverity( const IssueSeverity& ) = delete;
  IssueSeverity& operator=( const IssueSeverity& ) = delete;
  IssueSeverity( IssueSeverity&& ) noexcept;
  IssueSeverity& operator=( IssueSeverity&& ) noexcept;

  ~IssueSeverity();

  void setLevel( const IssueSeverity::Level& l ) { m_level = l; }
  void setMsg( std::string m ) { m_msg = std::move( m ); }

  IssueSeverity::Level getLevel() const { return m_level; }
  std::string getMsg() const { return m_msg; }
  std::string getOrigin() const;

  void report();

  friend inline std::ostream& operator<<( std::ostream&, const IssueSeverity& );

private:
  std::string m_file;
  std::string m_msg;

  int m_line                   = 0;
  IssueSeverity::Level m_level = IssueSeverity::NIL;
  bool m_reported              = true;

  static bool m_init;
  static IIssueLogger* m_ers;
};

inline IssueSeverity::IssueSeverity( IssueSeverity::Level level, int line, std::string file, std::string msg )
    : m_file( std::move( file ) ), m_msg( std::move( msg ) ), m_line( line ), m_level( level ), m_reported( false )
{
  report();
}

inline IssueSeverity::IssueSeverity( IssueSeverity::Level level, std::string msg )
    : IssueSeverity( std::move( level ), 0, "??", std::move( msg ) )
{
}

inline IssueSeverity::IssueSeverity( IssueSeverity&& rhs ) noexcept
{
  m_file     = std::move( rhs.m_file );
  m_msg      = std::move( rhs.m_msg );
  m_line     = rhs.m_line;
  m_level    = rhs.m_level;
  m_reported = std::exchange( rhs.m_reported, true );
}

inline IssueSeverity& IssueSeverity::operator=( IssueSeverity&& rhs ) noexcept
{
  m_file     = std::move( rhs.m_file );
  m_msg      = std::move( rhs.m_msg );
  m_line     = rhs.m_line;
  m_level    = rhs.m_level;
  m_reported = std::exchange( rhs.m_reported, true );
  return *this;
}

std::ostream& operator<<( std::ostream& os, const IssueSeverity& rhs )
{
  os << "ISSUE:  level " << rhs.getLevel() << " from: " << rhs.getOrigin() << " msg: " << rhs.getMsg();
  return os;
}

#endif
