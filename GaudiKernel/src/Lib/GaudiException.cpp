#include "GaudiKernel/GaudiException.h"

bool GaudiException::s_proc(false);

GaudiException::GaudiException( const std::string& Message,
                  const std::string& Tag,
                  const StatusCode & Code )
    : m_message    ( Message    )
    , m_tag        ( Tag        )
    , m_code       ( Code       )
    , m_previous   (     0      )
    { s_proc = true; }


GaudiException::GaudiException( const std::string&     Message    ,
                  const std::string&     Tag        ,
                  const StatusCode &     Code       ,
                  const GaudiException&  Exception  )
    : m_message    ( Message            )
    , m_tag        ( Tag                )
    , m_code       ( Code               )
    , m_previous   ( Exception.clone()  )
    {}


GaudiException::GaudiException( const std::string&     Message    ,
                  const std::string&     Tag        ,
                  const StatusCode &     Code       ,
                  const std::exception&  Exception  )
    : m_message    ( Message    )
    , m_tag        ( Tag        )
    , m_code       ( Code       )
    , m_previous   (     0      )
    {
      s_proc = true;
      m_message += ": " + System::typeinfoName(typeid(Exception)) + ", " +
                   Exception.what();
    }


GaudiException::GaudiException( const GaudiException& Exception ) : std::exception(Exception)
  {
    s_proc     = true;
    m_message  =   Exception.message() ;
    m_tag      =   Exception.tag    () ;
    m_code     =   Exception.code   () ;
    m_previous = ( 0 == Exception.previous() ) ?
      0 : Exception.previous()->clone() ;
  }


GaudiException::~GaudiException() throw() {
    m_code.setChecked();
    if( 0 != m_previous ) { delete m_previous ; m_previous = 0 ; }
    s_proc = false;
  }


GaudiException& GaudiException::operator=( const GaudiException& Exception ) {
    if ( &Exception == this ) { return *this; }
    m_message  =   Exception.message() ;
    m_tag      =   Exception.tag    () ;
    m_code     =   Exception.code   () ;
    if( 0 != m_previous ) { delete m_previous; m_previous = 0 ; }
    m_previous = ( 0 == Exception.previous() ) ?
      0 : Exception.previous()->clone() ;
    return *this;
  }


const std::string&   GaudiException::message   () const { return m_message; }


const std::string&    GaudiException::setMessage( const std::string& newMessage ) {
    m_message = newMessage; return message() ;
  }


const std::string&    GaudiException::tag       () const { return m_tag; }


const std::string&    GaudiException::setTag    ( const std::string& newTag     ) {
    m_tag = newTag ; return tag() ;
  }


const StatusCode&     GaudiException::code      () const { return m_code; }


const StatusCode&     GaudiException::setCode   ( const StatusCode& newStatus  ) {
    m_code = newStatus; return code() ;
  }


GaudiException*       GaudiException::previous  () const { return m_previous ; }


std::ostream& GaudiException::printOut  ( std::ostream& os ) const {
    os << tag() << " \t " << message() ;
    switch( code() ) {
      case StatusCode::SUCCESS : os << "\t StatusCode=SUCCESS"    ;  break ;
      case StatusCode::FAILURE : os << "\t StatusCode=FAILURE"    ;  break ;
      default                  : os << "\t StatusCode=" << code() ;  break ;
    }
    return ( 0 != previous() ) ? previous()->printOut( os << std::endl ) : os ;
  }


MsgStream& GaudiException::printOut ( MsgStream& os ) const {
    os << tag() << "\t" << message() ;
    switch( code() ) {
	    case StatusCode::SUCCESS : os << "\t StatusCode=SUCCESS"    ;  break ;
	    case StatusCode::FAILURE : os << "\t StatusCode=FAILURE"    ;  break ;
	    default                  : os << "\t StatusCode=" << code().getCode() ;  break ;
    }
    return ( 0 != previous() ) ? previous()->printOut( os << endmsg ) : os ;
  }


GaudiException* GaudiException::clone() const { return new GaudiException(*this); };


const char* GaudiException::what () const throw() { return message().c_str() ; }
