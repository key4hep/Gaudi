#include "StreamLogger.h"
#include <fstream>

StreamLogger::StreamLogger( const std::string& file )
    : m_name{"file:" + file}, m_oost{new std::ofstream( file )}, m_ost{m_oost.get()}
{
  if ( !m_oost ) throw std::invalid_argument( "can't open file" );
}

StreamLogger::StreamLogger( IMessageSvc* svc, MSG::Level lev )
    : m_name{"MsgStream"}, m_msgStr{new MsgStream( svc, "IssueLogger" )}, m_level{lev}
{
}

StreamLogger::StreamLogger( std::ostream& ost )
    : m_name{&ost == &std::cerr ? "STDERR" : &ost == &std::cout ? "STDOUT" : "unknown ostream"}, m_ost{&ost}
{
}
