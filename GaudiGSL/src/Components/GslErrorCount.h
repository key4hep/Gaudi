// $Id: GslErrorCount.h,v 1.2 2006/11/30 10:40:53 mato Exp $
// ============================================================================
#ifndef GAUDIGSL_GSLERRORCOUNT_H
#define GAUDIGSL_GSLERRORCOUNT_H 1
// Include files
// from STL
#include <string>
#include <map>
// from GaudiKernel
#include "GaudiKernel/AlgTool.h"
// from GaudiGSL
#include "GaudiGSL/IGslErrorHandler.h"
#include "GaudiGSL/GslError.h"
// forward declaration

/** @class GslErrorCount GslErrorCount.h
 *
 *  Concrete GSL error handler
 *  It is just counts number of GSL errors
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   30/04/2002
 */

class GslErrorCount: public extends1<AlgTool, IGslErrorHandler> {
public:

  /** handle the GSL error
   *  @see IGslErrorHandler
   *  @param error  error to be handled
   *  @see GslError
   *  @return status code
   */
  virtual StatusCode handle
  ( const GslError& error ) const ;

  /** standard finalization of Tool
   *  @see  AlgTool
   *  @see IAlgTool
   *  @return status code
   */
  virtual StatusCode finalize   () ;

  /** Standard constructor
   *  @param type   tool type (?)
   *  @param name   tool name
   *  @param parent pointer to parent
   */
  GslErrorCount
  ( const std::string& type   ,
    const std::string& name   ,
    const IInterface*  parent );

  /// destructor (protected and virtual)
  virtual ~GslErrorCount( );

private:

  /// container of error counters
  typedef std::map<GslError,unsigned int>  Counters ;
  mutable Counters m_counters ;


};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GSLERRORCOUNT_H
// ============================================================================
