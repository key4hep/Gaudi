// $Id: $
// ===========================================================================
#ifndef GAUDI_LOCKEDCHRONO_H
#define GAUDI_LOCKEDCHRONO_H 1
// ===========================================================================
// Include files
// ===========================================================================
// GaudiKernel
// ===========================================================================
#include "GaudiKernel/ChronoEntity.h"
// ===========================================================================
namespace Gaudi
{
  // =========================================================================
  namespace Utils 
  {
    // =======================================================================
    /** @class LockedChrono
     *
     *  Helper object, useful for measurement of CPU-performance
     *  of highly-recursive structures, e.g. Data-On-Demand service, 
     *  complex algorithms, etc...
     *  
     *  @code 
     * 
     *  class MyClass ...
     *  { 
     *
     *  public:
     * 
     *  void doSomethingRecursive() const 
     *    {
     *      ...
     *      LockedChrono timer ( m_chrono , m_lock ) ;
     *      ...
     *      // some direct or indirect recursive call:
     *      if ( ... ) { doSomethingRecursive () ; } 
     *      ...
     *    }
     * 
     *  private:
     *
     *     /// the lock 
     *     mutable bool         m_lock   ;    // lock  
     *     /// the actual timer 
     *     mutable ChronoEntity m_chrono ;    // actual timer 
     *
     *  };
     *  @endcode      
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-08-10
     */
    class LockedChrono
    {
    public: 
      // ======================================================================
      /// constructor from the actual timer: start the timer
      LockedChrono  ( ChronoEntity& c , bool& lock ) 
        : m_timer  ( c     ) 
        , m_locker ( lock  )
        , m_locked ( false )
      {
        if  ( !lock ) 
        {
          m_timer.start() ;
          m_locker = true ;
          m_locked = true ;
        }  
      }
      /// destructor:
      ~LockedChrono () 
      { 
        if  ( m_locked ) 
        { 
          m_timer.stop  () ;
          m_locker = false ;
        }  
      }
      // ======================================================================
    private:
      // ======================================================================
      /// no default constructor
      LockedChrono () ; // no default constructor
      // ======================================================================
    private:
      // ======================================================================
      /// the actual timer
      ChronoEntity& m_timer  ;                              // the actual timer
      /// the actual locker 
      bool&         m_locker ;                             // the actual locker 
      /// locked ? 
      bool          m_locked ;                             //          locked ? 
      // ======================================================================
    } ;
    // ========================================================================
  } //                                               end namespace Gaudi::Utils 
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDI_LOCKEDCHRONO_H
// ============================================================================
