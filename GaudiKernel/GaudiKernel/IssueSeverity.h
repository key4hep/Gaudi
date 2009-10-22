#ifndef GAUDIKERNEL_ISSUESEVERITY_H
#define GAUDIKERNEL_ISSUESEVERITY_H 1

class StatusCode;

#include <string>
#include <map>
#include <iostream>

#include "GaudiKernel/Kernel.h"

#define ISSUE(x,y) IssueSeverity(x,__LINE__,__FILE__,y)
#define STATUSCODE(z,x,y) StatusCode(z,ISSUE(x,y))

class IIssueLogger;

// namespace IS {
//   enum Level {
//     NIL = 0,
//     VERBOSE,
//     DEBUG,
//     DEBUG1,
//     DEBUG2,
//     DEBUG3,
//     INFO,
//     WARNING,
//     RECOVERABLE,
//     ERROR,
//     FATAL,
//     ALWAYS,
//     NUM_LEVELS
//   };
// }


#ifdef _WIN32
#ifdef ERROR
#undef ERROR
#endif
#endif

class GAUDI_API IssueSeverity {

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

  IssueSeverity();
  IssueSeverity( const IssueSeverity::Level &level, int line,
		 const std::string& file,
		 const std::string& msg="");
  IssueSeverity( const IssueSeverity::Level &level, const std::string& msg="");

  IssueSeverity( const IssueSeverity& es );
  IssueSeverity( IssueSeverity* es );

  IssueSeverity& operator=(const IssueSeverity& rhs);

  ~IssueSeverity();

  void setLevel(const IssueSeverity::Level& l) {
    m_level = l;
  }
  void setMsg(const std::string& m) {
    m_msg = m;
  }

  IssueSeverity::Level getLevel() const { return m_level; }
  std::string  getMsg() const { return m_msg; }
  std::string  getOrigin() const;

  void report();

  operator StatusCode() const;

  friend inline std::ostream& operator<< ( std::ostream&,
					   const IssueSeverity& ) ;

private:

  int m_line;
  std::string m_file;

  static bool m_init;
  static IIssueLogger* m_ers;

  IssueSeverity::Level m_level;
  std::string m_msg;
  bool m_reported;

  static void init();

};

inline IssueSeverity::IssueSeverity(): m_line(0), m_file(""),
				       m_level(IssueSeverity::NIL),
 				       m_msg(""), m_reported(true) {}

inline IssueSeverity::IssueSeverity(const IssueSeverity::Level &level, int line,
				    const std::string& file,
				    const std::string& msg):
  m_line(line), m_file(file), m_level(level), m_msg(msg), m_reported(false) {

  init();
  report();

}

inline IssueSeverity::IssueSeverity(const IssueSeverity::Level &level,
				    const std::string& msg):
  m_line(0), m_file("??"), m_level(level), m_msg(msg), m_reported(false) {

  init();
  report();

}

inline IssueSeverity::IssueSeverity( const IssueSeverity& rhs ) {
  m_line = rhs.m_line;
  m_file = rhs.m_file;
  m_level = rhs.m_level;
  m_msg   = rhs.m_msg;
  m_reported = true;
}

inline IssueSeverity::IssueSeverity( IssueSeverity* rhs ) {
  m_line = rhs->m_line;
  m_file = rhs->m_file;
  m_level = rhs->m_level;
  m_msg   = rhs->m_msg;
  m_reported = true;
}

inline IssueSeverity& IssueSeverity::operator=(const IssueSeverity& rhs) {
  m_line = rhs.m_line;
  m_file = rhs.m_file;
  m_level = rhs.m_level;
  m_msg   = rhs.m_level;
  m_reported = true;
  return *this;
}

std::ostream& operator<< ( std::ostream& os , const IssueSeverity& rhs ) {
  os << "ISSUE:  level " << rhs.getLevel() << " from: " << rhs.getOrigin()
     << " msg: " << rhs.getMsg();
  return os;
}


#endif
