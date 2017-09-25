#ifndef GAUDISVC_STREAMLOGGER_H
#define GAUDISVC_STREAMLOGGER_H

#include "GaudiKernel/MsgStream.h"

#include <ostream>
#include <string>

class IMessageSvc;

class StreamLogger final
{
public:
  StreamLogger( const std::string& file );
  StreamLogger( std::ostream& ost );
  StreamLogger( IMessageSvc*, MSG::Level );

  const std::string& name() const { return m_name; }

  void WriteToStream( const std::string& str ) { *m_ost << str << std::endl; }
  void WriteToMsgSvc( const std::string& str ) { *m_msgStr << m_level << str << endmsg; }

private:
  std::string m_name;
  std::unique_ptr<std::ostream> m_oost;
  std::ostream* m_ost = nullptr;
  std::unique_ptr<MsgStream> m_msgStr;
  MSG::Level m_level = MSG::NIL;
};

#endif
