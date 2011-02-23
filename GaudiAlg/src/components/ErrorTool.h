// $Id: ErrorTool.h,v 1.3 2006/11/30 10:16:12 mato Exp $
// ============================================================================
#ifndef GAUDIALG_ERRORTOOL_H
#define GAUDIALG_ERRORTOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/IErrorTool.h"
// ============================================================================

// ============================================================================
/** @class ErrorTool ErrorTool.h components/ErrorTool.h
 *
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-06-28
 */
// ============================================================================

class ErrorTool :         public  GaudiTool ,
                          virtual public IErrorTool
{

public:

  /** Standard constructor
   *  @see GaudiTool
   *  @see   AlgTool
   *  @param type   tool type (?)
   *  @param name   tool name
   *  @param parent pointer to parent component
   */
  ErrorTool ( const std::string& type,
              const std::string& name,
              const IInterface* parent);

  /// virtual and protected destrcutor
  virtual ~ErrorTool( );

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
    const size_t       mx  = 10                  ) const
  { return GaudiTool::Error ( msg , st, mx ) ; }

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
    const size_t       mx  = 10                  ) const
  { return GaudiTool::Warning( msg , st , mx ) ; }

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
    const MSG::Level   lev = MSG::INFO           ) const
  { return GaudiTool::Print( msg , st , lev ) ; }

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
    const StatusCode   sc      = StatusCode::FAILURE ) const
  { GaudiTool::Assert ( ok , message , sc ) ; }

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
    const StatusCode       sc  = StatusCode::FAILURE  ) const
  { Exception ( msg , exc , sc ) ; }

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
    const StatusCode       sc  = StatusCode::FAILURE  ) const
  { Exception ( msg , exc , sc ) ; }

  /** Create and throw the exception
   *  @see GaudiException
   *  @exception GaudiException always!
   *  @param msg    exception message
   *  @param sc     status code
   *  @return       status code (fictive)
   */
  virtual void Exception
  ( const std::string& msg = "no message"        ,
    const StatusCode   sc  = StatusCode::FAILURE ) const
  { Exception ( msg , sc ) ; }

};

#endif // GAUDIALG_ERRORTOOL_H
