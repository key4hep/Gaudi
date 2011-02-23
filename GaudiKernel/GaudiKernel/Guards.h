// $Id: Guards.h,v 1.2 2008/04/03 14:40:19 marcocle Exp $
// ============================================================================
#ifndef GAUDIKERNEL_GUARDS_H
#define GAUDIKERNEL_GUARDS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <exception>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/IAuditor.h"
// ============================================================================
// forward declaration
// ============================================================================
class GaudiException ;
// ============================================================================
namespace Gaudi
{
  /** @namespace Gaudi::Guards Guards.h GaudiKernel/Guards.h
   *  Collection of very simple classes which allows to
   *  execution of some functions within  the given well-defined
   *  and exception-safe context.
   *  In addition these utilities allow to remove many
   *  ugly lines from many ancient base classes
   *
   *  The namespace contains two simple guards:
   *    - class Gaudi::Guards::ExceptionGuard
   *    - class Gaudi::Guards::AuditorGuard
   *
   *  Please note tha the class Gaudi::Utils::AlgContext is also
   *   some kind of "guard".
   *
   *  All these guards could work together in very collaborative way:
   *
   *  @code
   *
   *  StatusCode Algorithm::sysInitialize ()
   *   {
   *    // create the message stream:
   *    MsgStream log ( msgSvc() , name() ) ;
   *
   *    // lock the scope for Auditor Service
   *    Gaudi::Guards::AuditorGuard auditor
   *           ( this                           ,
   *             auditorSvc()                   ,   ///< Auditor Service
   *             &IAuditorSvc::beforeInitialize ,   ///< "pre"-action
   *             &IAuditorsvc::afterInitialize  ) ; ///< "post"-action
   *
   *    // Lock the scope for Algorithm Context Service:
   *    Gaudi::Utils::AlgContext context ( this , m_contextSvc ) ;
   *
   *    // execute IAlgorithm::initialize within "try{}catch(..)" clause:
   *    Gaudi::Guards::ExceptionGuard guard
   *        ( this                                  , ///< OBJECT
   *          std::mem_fun(&IAlgorithm::initialize) , ///< FUNCTOR
   *          log                                   , ///< STREAM
   *          m_exceptionSvc ) ; ///< (optional) Exception Service
   *
   *    return guard ;  /// return the result of functor evaluation
   *   }
   *  @endcode
   *
   *  C++ guarantees the proper execution of all "post"-actions for all
   *  involved guards  when the scope is destroyed.
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-03-07
   */
  namespace Guards
  {
    /** @class ExceptionGuard Guards.h GaudiKernel/Guards.h
     *  The most simple guard - it execute the certain code
     *  withing  typical "try {} catch" clause, used in
     *  Auditor, Algorithm, AlgTool, etc.. classes
     *
     *  The usage is fairly trivial:
     *
     *  @code
     *
     *  // get the stream:
     *  MsgStream& log = ... ;
     *
     *  // create the guard object and execute this->initialize()
     *  // within the  standard "try"-clause:
     *  Gaudi::Guards::Guard guard
     *          ( this                                ,   ///< OBJECT
     *            std::mem_fun(&IAuditor::initialize) ,   ///< FUNCTOR
     *            log                                 ) ; ///< stream
     *
     *  // extract the status code"
     *  StatusCode sc = guard.code() ;
     *
     *  @endcode
     *
     *  The utility could be reused for member-function, regular functions, etc..
     *  It could be easily combined with STL-idioms
     *  Essentially it required only the semantical validity of the
     *  expression "StatusCode sc = fun(obj)"
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-03-07
     */
    class GAUDI_API ExceptionGuard
    {
    public:
      /* constructor form the object/scope, function and log-stream
       * @param obj the object/scope
       * @param fun function to be used
       * @param log output log-stream
       */
      template <class OBJECT, class FUNCTION>
      ExceptionGuard
      ( OBJECT         obj     ,
        FUNCTION       fun     ,
        MsgStream&     log     ,
        IExceptionSvc* svc = 0 )
        : m_sc  ( StatusCode::FAILURE )
      {
        try
        {
          // execute the functor:
          m_sc = fun ( obj ) ;  ///< execute the functor
          // in the case of error try use Exception Service
          if ( 0 != svc && m_sc.isFailure() ) { m_sc = svc->handleErr ( *obj , m_sc ) ; }
        }
        catch ( const GaudiException& e )
        {
          // Use the local handler and then (if possible) the Exception Service
          handle ( e  , log ) ;
          if ( 0 != svc ) { m_sc = svc -> handle ( *obj , e ) ; }
        }
        catch ( const std::exception& e )
        {
          // Use the local handler and then (if possible) the Exception Service
          handle ( e  , log ) ;
          if ( 0 != svc ) { m_sc = svc -> handle ( *obj , e ) ; }
        }
        catch ( ...  )
        {
          // Use the local handler and then (if possible) the Exception Service
          handle (      log ) ;
          if ( 0 != svc ) { m_sc = svc -> handle ( *obj     ) ; }
        }
      }
      /// destructor
      ~ExceptionGuard() ;
    public:
      /// the result of function evaluation
      const StatusCode&    code () const { return m_sc   ; }
      /// cast operator, useful for the implicit conversions
      operator const StatusCode&() const { return code() ; }
    private:
      // default constructor is disabled
      ExceptionGuard() ; ///< default constructor is disabled
      // copy constructor is disabled
      ExceptionGuard           ( const ExceptionGuard& ) ; ///< no copy
      // assignment operator is disabled
      ExceptionGuard& operator=( const ExceptionGuard& ) ; ///< no assignement
    protected:
      /// local handler of GaudiException
      void handle ( const GaudiException& e , MsgStream& s ) ;
      /// local handler of std::exception
      void handle ( const std::exception& e , MsgStream& s ) ;
      /// local handler of UNKNOWN exceptions
      void handle (                           MsgStream& s ) ;
    private:
      // status code: result of the function evaluation
      StatusCode m_sc    ; ///< status code : result of function evaluation
    } ;
    // ========================================================================
    /** @class AuditorGuard Guards.h GaudiKernel/Guards.h
     *  It is a simple guard,
     *  which "locks" the scope for the Auditor Service
     *  is am exception-safe way
     *
     *  The pattern ensures that "post-action" will be always executed
     *
     *  @code
     *
     *  StatusCode Algorithm::sysInitialize ()
     *   {
     *      AuditorGuard auditor ( this ,
     *        auditSvc()                ,   ///< pointer to Auditor Service
     *        IAuditor::Initialize      ) ; ///< event to audit
     *      ...
     *      StatusCode sc = ... ;
     *      ...
     *      return sc ;                  ///< RETURN
     *   }
     *
     *  @endcode
     *
     *  @code
     *
     *  StatusCode Algorithm::sysExecute ()
     *   {
     *      AuditorGuard auditor ( this       ,
     *        auditSvc()                      ,   ///< pointer to Auditor Service
     *        IAuditor::execute               ) ; ///< event to audit
     *      ...
     *      StatusCode sc = ... ;
     *      ...
     *      auditor.setCode ( sc ) ;
     *      ...
     *      return sc ;                  ///< RETURN
     *   }
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-03-07
     */
    class GAUDI_API AuditorGuard
    {
    public:
      /// constructor
      AuditorGuard ( INamedInterface*                      obj      ,
                     IAuditor*                    svc      ,
                     IAuditor::StandardEventType  evt      ) ;
      /// constructor
      AuditorGuard ( INamedInterface*                      obj      ,
                     IAuditor*                    svc      ,
                     IAuditor::CustomEventTypeRef evt      ) ;

      /// constructor
      AuditorGuard ( INamedInterface*                      obj      ,
                     IAuditor*                    svc      ,
                     IAuditor::StandardEventType  evt      ,
                     const StatusCode            &sc       ) ;
      /// constructor
      AuditorGuard ( INamedInterface*                      obj      ,
                     IAuditor*                    svc      ,
                     IAuditor::CustomEventTypeRef evt      ,
                     const StatusCode            &sc       ) ;
      /// constructor
      AuditorGuard ( const std::string           &name     ,
                     IAuditor*                    svc      ,
                     IAuditor::StandardEventType  evt      ) ;
      /// constructor
      AuditorGuard ( const std::string           &name     ,
                     IAuditor*                    svc      ,
                     IAuditor::CustomEventTypeRef evt      ) ;

      /// constructor
      AuditorGuard ( const std::string           &name     ,
                     IAuditor*                    svc      ,
                     IAuditor::StandardEventType  evt      ,
                     const StatusCode            &sc       ) ;
      /// constructor
      AuditorGuard ( const std::string           &name     ,
                     IAuditor*                    svc      ,
                     IAuditor::CustomEventTypeRef evt      ,
                     const StatusCode            &sc       ) ;

      /// dectructor
      ~AuditorGuard() ;
    public:
      // get the status code
      const StatusCode &code () const { return *m_sc ; }
    private:
      // the default constructor is disabled
      AuditorGuard () ; ///< the default constructor is disabled
      // the copy constructor is disabled
      AuditorGuard           ( const AuditorGuard& right ) ; ///<  no copy
      // assignement operator is disabled
      AuditorGuard& operator=( const AuditorGuard& right ) ; ///<  no assignement
    private :
      /// the guarded object
      INamedInterface*                     m_obj   ;
      /// the guarded object name (if there is no INamedInterface)
      std::string                 m_objName;
      /// auditor service
      IAuditor*                   m_svc   ;
      /// Event type (standard events)
      IAuditor::StandardEventType m_evt   ;
      /// Event type (custom events)
      IAuditor::CustomEventType   m_cevt  ;
      /// Pointer to a status code instance, to be passed to the "after" function if needed
      /// The instance must have a scope larger than the one of the guard.
      /// No check is performed.
      const StatusCode           *m_sc    ;
      /// Flag to remember which event type was used.
      bool                        m_customEvtType;

      inline void i_before() {
        if ( 0 != m_svc ) { // if the service is not available, we cannot do anything
          m_svc->addRef(); // increase the reference counting
          if (0 != m_obj) {
            if (m_customEvtType) {
              m_svc->before(m_cevt,m_obj);
            } else {
              m_svc->before(m_evt,m_obj);
            }
          } else { // use object name
            if (m_customEvtType) {
              m_svc->before(m_cevt,m_objName);
            } else {
              m_svc->before(m_evt,m_objName);
            }
          }
        }
      }

      inline void i_after() {
        if ( 0 != m_svc ) { // if the service is not available, we cannot do anything
          if (0 != m_obj) {
            if (m_customEvtType) {
              if (0 != m_sc) {
                m_svc->after(m_cevt,m_obj,*m_sc);
              } else {
                m_svc->after(m_cevt,m_obj);
              }
            } else {
              if (0 != m_sc) {
                m_svc->after(m_evt,m_obj,*m_sc);
              } else {
                m_svc->after(m_evt,m_obj);
              }
            }
          } else { // use object name
            if (m_customEvtType) {
              if (0 != m_sc) {
                m_svc->after(m_cevt,m_objName,*m_sc);
              } else {
                m_svc->after(m_cevt,m_objName);
              }
            } else {
              if (0 != m_sc) {
                m_svc->after(m_evt,m_objName,*m_sc);
              } else {
                m_svc->after(m_evt,m_objName);
              }
            }
          }
          m_svc->release(); // we do not need the service anymore
          m_svc = 0 ;
        }
      }
    } ;
  } // end of namespace Gaudi::Guards
} //end of namespace Gaudi


// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_GUARDS_H
// ============================================================================
