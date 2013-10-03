// $Id: Algorithm.h,v 1.20 2008/10/28 10:40:19 marcocle Exp $
// ============================================================================
// ============================================================================
#ifndef GAUDIPYTHON_ALGORITHM_H
#define GAUDIPYTHON_ALGORITHM_H
// ============================================================================
// Include Files
// ============================================================================
// Python
// ============================================================================
#include "Python.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Bootstrap.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================

namespace GaudiPython
{
  /// call the python method
  GAUDI_API StatusCode call_python_method ( PyObject* self , const char* method ) ;
}

namespace GaudiPython
{

  /** @class PyAlgorithm
   *  Python Algorithm base class
   */
  class GAUDI_API PyAlgorithm : public Algorithm
  {
  public:
    /** constructor
     *  @param self python objects
     *  @param name name of algorithm instance
     */
    PyAlgorithm
    ( PyObject*          self ,
      const std::string& name ) ;
  public:
    StatusCode  initialize () ;
    StatusCode  start      () ;
    StatusCode  beginRun   () ;
    StatusCode  endRun     () ;
    StatusCode  execute    () ;
    StatusCode  stop       () ;
    StatusCode  finalize   () ;
    IAlgorithm* myself() { return this; }
  private:
    PyObject* m_self;
  };
  // ==========================================================================
  /** @class PyAlg
   *  general class to embed the existing algorithm/base class
   *  into the python
   *  @author Vanya BELYAEV  Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-03
   */
  template <class ALGORITHM>
  class GAUDI_API PyAlg : public          ALGORITHM
  {
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from Python object and the name
     *  @param self python object
     *  @param name name of algorithm instance
     */
    PyAlg
    ( PyObject*          self ,
      const std::string& name )
      : ALGORITHM ( name , Gaudi::svcLocator() )
      , m_self ( self )
    {
      // the printout of actual type for embedded algorithm has no sense
      this -> setProperty ( "TypePrint" , "false" ) ;
      this -> setProperty ( "StatPrint" , "true"  ) ;
      // The owner of the Algorithm is Python (as creator) therefore
      // it should not be deleted by Gaudi (added an extra addRef()).
      this -> addRef() ;
      this -> addRef() ;
    }
    // ========================================================================
  protected:
    // ========================================================================
    /// virtual destructor
    virtual ~PyAlg() {}                                   // virtual destructor
    /// get the object
    PyObject* _obj() const { return m_self ; }            //     get the object
    // ========================================================================
  public:
    // ========================================================================
    virtual StatusCode initialize  ()
    { return GaudiPython::call_python_method ( m_self , "initialize" ) ; }
    virtual StatusCode start  ()
    { return GaudiPython::call_python_method ( m_self , "start"      ) ; }
    virtual StatusCode beginRun  ()
    { return GaudiPython::call_python_method ( m_self , "beginRun"   ) ; }
    virtual StatusCode endRun  ()
    { return GaudiPython::call_python_method ( m_self , "endRun"     ) ; }
    virtual StatusCode execute     ()
    { return GaudiPython::call_python_method ( m_self , "execute"    ) ; }
    virtual StatusCode stop  ()
    { return GaudiPython::call_python_method ( m_self , "stop"       ) ; }
    virtual StatusCode finalize    ()
    { return GaudiPython::call_python_method ( m_self , "finalize"   ) ; }
    // ========================================================================
    virtual IAlgorithm* ialgorithm () { return this ; }
    virtual IProperty*  iproperty  () { return this ; }
    // ========================================================================
    // preserve the existing methods
    virtual StatusCode initialize_ () { return ALGORITHM::initialize () ; }
    virtual StatusCode finalize_   () { return ALGORITHM::finalize   () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    PyAlg() ;                            // the default constructor is disabled
    /// the copy constructor is disabled
    PyAlg ( const PyAlg& );              //    the copy constructor is disabled
    /// the assignment operator is disabled
    PyAlg& operator=( const PyAlg& ) ;   // the assignment operator is disabled
    // ========================================================================
  private:
    // ========================================================================
    /// "shadow" python class
    PyObject* m_self;                                  // "shadow" python class
    // ========================================================================
  } ;
  // ==========================================================================
} //                                               end of namespace GaudiPython
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIPYTHON_ALGORITHM_H
// ============================================================================
