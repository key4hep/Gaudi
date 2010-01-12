// $Id: IErrorTool.h,v 1.2 2004/10/18 08:17:59 mato Exp $
// ============================================================================
#ifndef GAUDIALG_IERRORTOOL_H
#define GAUDIALG_IERRORTOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IMessageSvc.h"
// ============================================================================
class GaudiException ;

/** @class IErrorTool IErrorTool.h GaudiTools/IErrorTool.h
 *
 *
 *  @author Ivan BELYAEV
 *  @date   2004-06-28
 */
class GAUDI_API IErrorTool: virtual public IAlgTool
{
public:
  /// InterfaceID
  DeclareInterfaceID(IErrorTool,2,0);

public:

  /** Print the error  message, return status code
   *  and perform the statistics of error messages
   *
   *  @code
   *
   *  IErrorTool* tool = .. ;
   *   if( a < 0 ) { return tool->Error(" 'a' is negative!") ;}
   *   if( b < 0 ) { return tool->Error(" 'b' is illegal!" , StatusCode(25) );
   *   if( c < 0 )
   *      { return tool->Error(" 'c' is negative" , StatusCode(35) , 50 );
   *
   *  @endcode
   *
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *  @param msg    error message
   *  @param st     status code
   *  @param mx     maximal number of printouts
   *  @return       status code
   */
  virtual StatusCode Error
  ( const std::string& msg ,
    const StatusCode   st  = StatusCode::FAILURE ,
    const size_t       mx  = 10                  ) const = 0 ;

  /** Print the warning  message, return status code
   *  and perform the statistics of warning  messages
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *  @param msg    warning message
   *  @param st     statsu code
   *  @param mx     maximal number of printouts
   *  @return       status code
   */
  virtual StatusCode Warning
  ( const std::string& msg ,
    const StatusCode   st  = StatusCode::FAILURE ,
    const size_t       mx  = 10                  ) const = 0 ;

  /** Print the message and return status code
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *  @param msg    warning message
   *  @param st     status code
   *  @param lev    print level
   *  @return       status code
   */
  virtual StatusCode Print
  ( const std::string& msg ,
    const StatusCode   st  = StatusCode::SUCCESS ,
    const MSG::Level   lev = MSG::INFO           ) const = 0 ;

  /** Assertion - throw exception, if condition is not fulfilled
   *  @see CaloException
   *  @see GaudiException
   *  @exception CaloException for invalid condition
   *  @param ok           condition which should be "true"
   *  @param message      message to be associated with the exception
   *  @param sc           status code to be returned (artificial)
   *  @return             status code
   */
  virtual void Assert
  ( const bool         ok                            ,
    const std::string& message = ""                  ,
    const StatusCode   sc      = StatusCode::FAILURE ) const = 0 ;

  /** Create and (re)-throw the exception
   *  @see GaudiException
   *  @exception CaudiException always!
   *  @param msg    exception message
   *  @param exc    (previous) exception of type GaudiException
   *  @param sc     status code
   *  @return       status code (fictive)
   */
  virtual void Exception
  ( const std::string    & msg                        ,
    const GaudiException & exc                        ,
    const StatusCode       sc  = StatusCode::FAILURE  ) const = 0 ;

  /** Create and (re)-throw the exception
   *  @see GaudiException
   *  @exception GaudiException always!
   *  @param msg    exception message
   *  @param exc    (previous) exception of type std::exception
   *  @param sc     status code
   *  @return       status code (fictive)
   */
  virtual void Exception
  ( const std::string    & msg                        ,
    const std::exception & exc                        ,
    const StatusCode       sc  = StatusCode::FAILURE  ) const = 0 ;

  /** Create and throw the exception
   *  @see GaudiException
   *  @exception GaudiException always!
   *  @param msg    exception message
   *  @param sc     status code
   *  @return       status code (fictive)
   */
  virtual void Exception
  ( const std::string& msg = "no message"        ,
    const StatusCode   sc  = StatusCode::FAILURE ) const = 0 ;

protected:

  // protected destructor
  virtual ~IErrorTool() ;

};

// ============================================================================
#endif // GAUDIALG_IERRORTOOL_H
// ============================================================================
