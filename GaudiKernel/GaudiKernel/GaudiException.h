/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_GAUDIEXCEPTION_H
#define GAUDIKERNEL_GAUDIEXCEPTION_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

#include <exception>
#include <iostream>
#include <string>

/**
 * @class GaudiException GaudiException.h GaudiKernel/GaudiException.h
 *
 * Define general base for Gaudi exception
 *
 * @author Vanya Belyaev
 * @author Sebastien Ponce
 */
class GAUDI_API GaudiException : virtual public std::exception {
  friend class StatusCode;

public:
  /** Constructor (1)
      @param Message error message
      @param Tag "name tag", or exeption type
      @param Code status code
  */
  GaudiException( std::string Message, std::string Tag, StatusCode Code );

  /** Constructor (2)
      @param Message error message
      @param Tag "name tag", or exeption type
      @param Code status code
      @param Exception "previous"  exception
  */
  GaudiException( std::string Message, std::string Tag, StatusCode Code, const GaudiException& Exception );

  /** Constructor (3)
      @param Message error message
      @param Tag "name tag", or exeption type
      @param Code status code
      @param Exception "previous" exception (used to improve the error message)
  */
  GaudiException( std::string Message, std::string Tag, StatusCode Code, const std::exception& Exception );

  /// Copy constructor (deep copying!)
  GaudiException( const GaudiException& Exception );

  /// destructor (perform the deletion of "previous" field!)
  virtual ~GaudiException() throw();

  /// assignment operator
  GaudiException& operator=( const GaudiException& Exception );

  ///  error message to be printed
  virtual const std::string& message() const { return m_message; }

  /// update the error message to be printed
  virtual const std::string& setMessage( const std::string& newMessage ) {
    m_message = newMessage;
    return message();
  }

  ///  name tag for the exception, or exception type
  virtual const std::string& tag() const { return m_tag; }

  /// update name tag
  virtual const std::string& setTag( const std::string& newTag ) {
    m_tag = newTag;
    return tag();
  }

  /// StatusCode for Exception
  virtual const StatusCode& code() const { return m_code; }

  ///  update the status code for the exception
  virtual const StatusCode& setCode( const StatusCode& newStatus ) {
    m_code = newStatus;
    return code();
  }

  /// get the previous exception ( "previous" element in the linked list)
  virtual GaudiException* previous() const { return m_previous.get(); }

  /// return the stack trace at instantiation
  virtual const std::string& backTrace() const { return m_backTrace; }

  /// methods  for overloaded printout to std::ostream& and MsgStream&
  virtual std::ostream& printOut( std::ostream& os = std::cerr ) const;

  /// Output the exception to the Gaudi MsgStream
  virtual MsgStream& printOut( MsgStream& os ) const;

  /// clone operation
  virtual GaudiException* clone() const { return new GaudiException( *this ); }

  /// method from std::exception
  const char* what() const throw() override { return message().c_str(); }

protected:
  std::string                     m_message;   /// error message
  std::string                     m_tag;       /// exception tag
  StatusCode                      m_code;      /// status code for exception
  std::string                     m_backTrace; /// stack trace at instantiation
  std::unique_ptr<GaudiException> m_previous;  /// "previous" element in the linked list
  static bool                     s_proc;
};

/// overloaded printout to std::ostream
std::ostream& operator<<( std::ostream& os, const GaudiException& ge );

/// overloaded printout to std::ostream
std::ostream& operator<<( std::ostream& os, const GaudiException* pge );

/// overloaded printout to MsgStream
MsgStream& operator<<( MsgStream& os, const GaudiException& ge );

/// overloaded printout to MsgStream
MsgStream& operator<<( MsgStream& os, const GaudiException* pge );

#endif // GAUDIKERNEL_GAUDIEXCEPTION_H
