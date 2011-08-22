// $Id: GaudiException.h,v 1.6 2007/12/20 18:28:33 marcocle Exp $
#ifndef GAUDIKERNEL_GAUDIEXCEPTION_H
#define GAUDIKERNEL_GAUDIEXCEPTION_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"

#include <string>
#include <iostream>
#include <exception>


/**
 * @class GaudiException GaudiException.h GaudiKernel/GaudiException.h
 *
 * Define general base for Gaudi exception
 *
 * @author Vanya Belyaev
 * @author Sebastien Ponce
*/
class GAUDI_API GaudiException: virtual public std::exception {
  // friends
  friend inline std::ostream& operator<< ( std::ostream& os ,
                                           const GaudiException&   ge ) ;
  friend inline std::ostream& operator<< ( std::ostream& os ,
                                           const GaudiException*  pge ) ;
  friend inline MsgStream&    operator<< ( MsgStream&    os ,
                                           const GaudiException&   ge ) ;
  friend inline MsgStream&    operator<< ( MsgStream&    os ,
                                           const GaudiException*  pge ) ;
  friend class StatusCode;

public:
  /** Constructor (1)
      @param Message error message
      @param Tag "name tag", or exeption type
      @param Code status code
  */
  GaudiException( const std::string& Message,
                  const std::string& Tag,
                  const StatusCode & Code )
    : m_message    ( Message    )
    , m_tag        ( Tag        )
    , m_code       ( Code       )
    , m_previous   (     0      )
    { s_proc = true; }

  /** Constructor (2)
      @param Message error message
      @param Tag "name tag", or exeption type
      @param Code status code
      @param Exception "previous"  exception
  */
  GaudiException( const std::string&     Message    ,
                  const std::string&     Tag        ,
                  const StatusCode &     Code       ,
                  const GaudiException&  Exception  )
    : m_message    ( Message            )
    , m_tag        ( Tag                )
    , m_code       ( Code               )
    , m_previous   ( Exception.clone()  )
    {}

  /** Constructor (3)
      @param Message error message
      @param Tag "name tag", or exeption type
      @param Code status code
      @param Exception "previous" exception (used to improve the error message)
  */
  GaudiException( const std::string&     Message    ,
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

  /// Copy constructor (deep copying!)
  GaudiException( const GaudiException& Exception ) : std::exception(Exception)
  {
    s_proc     = true;
    m_message  =   Exception.message() ;
    m_tag      =   Exception.tag    () ;
    m_code     =   Exception.code   () ;
    m_previous = ( 0 == Exception.previous() ) ?
      0 : Exception.previous()->clone() ;
  }

  /// destructor (perform the deletion of "previous" field!)
  virtual ~GaudiException() throw() {
    m_code.setChecked();
    if( 0 != m_previous ) { delete m_previous ; m_previous = 0 ; }
    s_proc = false;
  }

  /// assignment operator
  GaudiException& operator=( const GaudiException& Exception ) {
    if ( &Exception == this ) { return *this; }
    m_message  =   Exception.message() ;
    m_tag      =   Exception.tag    () ;
    m_code     =   Exception.code   () ;
    if( 0 != m_previous ) { delete m_previous; m_previous = 0 ; }
    m_previous = ( 0 == Exception.previous() ) ?
      0 : Exception.previous()->clone() ;
    return *this;
  }

  ///  error message to be printed
  virtual const std::string&    message   () const { return m_message; }

  /// update the error message to be printed
  virtual const std::string&    setMessage( const std::string& newMessage ) {
    m_message = newMessage; return message() ;
  }

  ///  name tag for the exception, or exception type
  virtual const std::string&    tag       () const { return m_tag; }

  /// update name tag
  virtual const std::string&    setTag    ( const std::string& newTag     ) {
    m_tag = newTag ; return tag() ;
  }

  /// StatusCode for Exception
  virtual const StatusCode&     code      () const { return m_code; }

  ///  update the status code for the exception
  virtual const StatusCode&     setCode   ( const StatusCode& newStatus  ) {
    m_code = newStatus; return code() ;
  }

  /// get the previous exception ( "previous" element in the linked list)
  virtual GaudiException*       previous  () const { return m_previous ; }

  /// methods  for overloaded printout to std::ostream& and MsgStream&
  virtual std::ostream& printOut  ( std::ostream& os = std::cerr ) const {
    os << tag() << " \t " << message() ;
    switch( code() ) {
      case StatusCode::SUCCESS : os << "\t StatusCode=SUCCESS"    ;  break ;
      case StatusCode::FAILURE : os << "\t StatusCode=FAILURE"    ;  break ;
      default                  : os << "\t StatusCode=" << code() ;  break ;
    }
    return ( 0 != previous() ) ? previous()->printOut( os << std::endl ) : os ;
  }

  /// Output the exception to the Gaudi MsgStream
  virtual MsgStream& printOut ( MsgStream& os ) const {
    os << tag() << "\t" << message() ;
    switch( code() ) {
	    case StatusCode::SUCCESS : os << "\t StatusCode=SUCCESS"    ;  break ;
	    case StatusCode::FAILURE : os << "\t StatusCode=FAILURE"    ;  break ;
	    default                  : os << "\t StatusCode=" << code().getCode() ;  break ;
    }
    return ( 0 != previous() ) ? previous()->printOut( os << endmsg ) : os ;
  }

  /// clone operation
  virtual GaudiException* clone() const { return new GaudiException(*this); }

  /// method from std::exception
  virtual const char* what () const throw() { return message().c_str() ; }
protected:
  mutable std::string     m_message ;  /// error message
  mutable std::string     m_tag     ;  /// exception tag
  mutable StatusCode      m_code    ;  /// status code for exception
  mutable GaudiException* m_previous;  /// "previous" element in the linked list
  static bool             s_proc;
};

/// overloaded printout to std::ostream
std::ostream& operator<< ( std::ostream& os , const GaudiException&   ge ) {
  return ge.printOut( os );
}
std::ostream& operator<< ( std::ostream& os , const GaudiException*  pge )
{ return (0 == pge) ?
    ( os << " GaudiException* points to NULL!" ) : ( os << *pge ); }

/// overloaded printout to MsgStream
MsgStream&    operator<< ( MsgStream& os    , const GaudiException&   ge ) {
  return ge.printOut( os );
}
/// overloaded printout to MsgStream
MsgStream&    operator<< ( MsgStream& os    , const GaudiException*  pge ) {
  return (0 == pge) ?
    ( os << " GaudiException* points to NULL!" ) : ( os << *pge );
}

#endif  // GAUDIKERNEL_GAUDIEXCEPTION_H
