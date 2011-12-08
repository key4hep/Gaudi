// $Id: TupleDetail.h,v 1.2 2004/10/18 08:18:00 mato Exp $
#ifndef GAUDIALG_TUPLEDETAIL_H
#define GAUDIALG_TUPLEDETAIL_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiAlg/Tuples.h"
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"
// ============================================================================

// ============================================================================
/** @file
 *  Collection of few 'technical' methods for instantiation of tuples
 *  @author Ivan BELYAEV
 *  @date   2004-01-27
 */
// ============================================================================

namespace Tuples
{
  /** @namespace detail TupleObj.h GaudiAlg/TupleObj.h
   *  namespace with few technical implementations
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2004-01-23
   */
  namespace detail
  {
    /** @class TupleObjImp
     *
     *  The simplest concrete implementation of class TupleObj
     *  with 3 arbitrary error handlers
     *  Any types (classes, functions, etc.) which supports the
     *  semantics
     *
     *  @code
     *
     *     HANDLER obj = .. ;
     *
     *     StatusCode obj( msg , sc )
     *
     *  @endcode
     *
     *  can be used as error handler, e.g.
     *
     *  @code
     *
     *   void print_error   ( const std::string& msg , StatusCode sc )
     *    { std::cout <<
     *      "Error"   << msg << " code " << sc.getCode() << std::endl ; }
     *   void print_warning ( const std::string& msg , StatusCode sc )
     *    { std::cout <<
     *      "Warning" << msg << " code " << sc.getCode() << std::endl ; }
     *   void print_print   ( const std::string& msg , StatusCode sc )
     *    { std::cout << msg << " code " << sc.getCode() << std::endl ; }
     *
     *   NTuple::Tuple* tuple = ... ;
     *   TupleObj* obj =
     *    createTupleObj( print_errror   ,
     *                    print_warning  ,
     *                    print_print    ,
     *                    " my tuple "   , ntuple ) ;
     *
     *  @endcode
     *
     *  Templated helper functions allow to avoid heavy semantics of
     *  dealing with explicit type
     *
     *  Few concrete error handlers for dealing with classes,
     *  which supports member functions Error and Warning
     *   ( e.g. class GaudiAlgorithm or class GaudiTool )
     *  are provided
     *
     * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     * @date   2004-1-24
     */
    template<class HANDLER1, class HANDLER2>
    class TupleObjImp: public TupleObj
    {
    public:
      /** constructor
       *  @param  handler1 error   handler
       *  @param  handler2 warning handler
       *  @param  name  Name of the NTuple
       *  @param  tuple NTuple itself
       *  @param  clid  NTuple CLID
       *  @param  type  NTuple type
       */
      TupleObjImp ( HANDLER1              handler1                      ,
                    HANDLER2              handler2                      ,
                    const std::string&    name                          ,
                    NTuple::Tuple*        tuple                         ,
                    const CLID&           clid = CLID_ColumnWiseTuple   ,
                    const Tuples::Type    type = Tuples::NTUPLE         )
        : TupleObj ( name , tuple , clid, type )
        , m_handler1 ( handler1 )
        , m_handler2 ( handler2 )
      {}
    public:

      virtual StatusCode Error
      ( const std::string& msg ,
        const StatusCode   sc  = StatusCode::FAILURE ) const
      { m_handler1 ( name() + " " + msg , sc  ) ; return sc ; }

      virtual StatusCode Warning
      ( const std::string& msg ,
        const StatusCode   sc  = StatusCode::FAILURE ) const
      { m_handler2 ( name() + " " + msg , sc  ) ; return sc ; }

    protected:
      /// empty protected  destructor
      virtual ~TupleObjImp(){}
    private:
      HANDLER1 m_handler1 ;
      HANDLER2 m_handler2 ;
    };

    /** Templated helper functions allow to avoid heavy semantics of
     *  dealing with explicit type of class TupleObjImp
     *
     *  @code
     *
     *   void print_error   ( const std::string& msg , StatusCode sc )
     *    { std::cout <<
     *      "Error"   << msg << " code " << sc.getCode() << std::endl ; }
     *   void print_warning ( const std::string& msg , StatusCode sc )
     *    { std::cout <<
     *      "Warning" << msg << " code " << sc.getCode() << std::endl ; }
     *
     *   NTuple::Tuple* tuple = ... ;
     *   TupleObj* obj =
     *    createTupleObj( print_errror   ,
     *                    print_warning  ,
     *                    " my tuple "   , ntuple ) ;
     *
     *  @endcode
     *
     *  Few concrete error handlers for dealing with classes,
     *  which supports member functions Error,Warning and Print
     *   ( e.g. class GaudiAlgorithm or class GaudiTool )
     *  are provided
     *
     * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     * @date   2004-1-24
     */
    template<class HANDLER1, class HANDLER2>
    inline TupleObj* createTupleObj
    ( HANDLER1 handler1  ,
      HANDLER2 handler2  ,
      const std::string& name                          ,
      NTuple::Tuple*     tuple                         ,
      const CLID&        clid = CLID_ColumnWiseTuple   ,
      const Tuples::Type type = Tuples::NTUPLE         )
    {
      return new TupleObjImp<HANDLER1,HANDLER2>
        ( handler1 , handler2 ,
          name     , tuple    , clid     , type  ) ;
    }

    /** @class ErrorHandler
     *
     *  Concrete error handlers for dealing with classes,
     *  which supports member functions Error,Warning and Print
     *   ( e.g. class GaudiAlgorithm or class GaudiTool )
     *  are provided
     *
     * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     * @date   2004-1-24
     */
    template <class OBJECT, class FUNCTION>
    class ErrorHandler
    {
    public:
      /// constructor
      ErrorHandler( const OBJECT* obj ,
                    FUNCTION      fun )
        : m_obj ( obj ) , m_fun ( fun ) {}
    public:
      /// the only one 'useful' method
      StatusCode operator() ( const std::string& msg     ,
                              const StatusCode   sc      ,
                              const size_t       mp = 10 ) const
      {
        return (m_obj->*m_fun)( msg , sc , mp ) ;
      }
    private:
      // default constructor is private
      ErrorHandler();
    private:
      const OBJECT* m_obj ;
      FUNCTION      m_fun ;
    };

    /** Templated helper functions allow to avoid heavy semantics of
     *  dealing with explicit type of class ErrorHandler
     *
     * @code
     *
     * const GaudiAlgorithm* algo =  ... ;
     *
     * // error handler using GaudiAlgorithm::Error member function
     * make_handler( algo , &GaudiAlgorithm::Error ) ;
     *
     *  const GaudiTool* tool = .... ;
     * // error handler using GaudiTool::Print member function
     * make_handler( tool , &GaudiTool::Print ) ;
     *
     * @endcode
     *
     * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     * @date   2004-1-24
     */
    template <class OBJECT, class FUNCTION>
    inline ErrorHandler<OBJECT,FUNCTION>
    make_handler ( const OBJECT* object   ,
                   FUNCTION      function )
    { return ErrorHandler<OBJECT,FUNCTION>( object , function ); }

  }

  /** Templated helper functions allow to avoid heavy semantics of
   *  dealing with explicit type of class TupleObjImp
   *
   *  @code
   *
   *   const GaudiAlgorithm* algo  = ... ;
   *   NTuple::Tuple*        tuple = ... ;
   *   TupleObj* obj = createTupleObj( algo , " my tuple 1 "   , ntuple ) ;
   *
   *   const GaudiTool* tool   = ... ;
   *   NTuple::Tuple*   tuple2 = ... ;
   *   TupleObj* obj2 = createTupleObj( tool , " my tuple 2 " , ntuple2 ) ;
   *
   *  @endcode
   *
   * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   * @date   2004-1-24
   */
  template <class OWNER>
  inline TupleObj* createTupleObj
  ( const OWNER*       owner                         ,
    const std::string& name                          ,
    NTuple::Tuple*     tuple                         ,
    const CLID&        clid = CLID_ColumnWiseTuple   ,
    const Tuples::Type type = Tuples::NTUPLE         )
  {
    return detail::createTupleObj
      ( detail::make_handler ( owner , &OWNER::Error   ) ,
        detail::make_handler ( owner , &OWNER::Warning ) ,
        name , tuple , clid , type ) ;
  }

} // end of namespace Tuples



// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_TUPLEDETAIL_H
// ============================================================================
